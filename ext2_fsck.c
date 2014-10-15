#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <./linux/ext2_fs.h> 
#include <sys/types.h>
#include <sys/stat.h>


#define BASE_OFFSET 1024
//#define BLOCK_OFFSET(block) (1024 + (block-1)*4096)


static unsigned int block_size = 0;


inline unsigned int block_offset(unsigned int block)
{
	unsigned int op;
	printf("%u",block);
	op = (BASE_OFFSET + ((block - 1) * block_size)); 
	printf("\n%u",op);
	return op;
}


static void read_dir(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group)
{
	void *block;

	if (S_ISDIR(inode->i_mode)) {
		struct ext2_dir_entry_2 *entry;
		unsigned int size = 0;

		if ((block = malloc(block_size)) == NULL) { /* allocate memory for the data block */
			fprintf(stderr, "Memory error\n");
			close(fd);
			exit(1);
		}

		lseek(fd, inode->i_block[0]*4096, SEEK_SET);
		read(fd, block, block_size);                /* read block from disk*/

		entry = (struct ext2_dir_entry_2 *) block;  /* first entry in the directory */
                /* Notice that the list may be terminated with a NULL
                   entry (entry->inode == NULL)*/
		while((size < inode->i_size) && entry->inode) {
			char file_name[EXT2_NAME_LEN+1];
			memcpy(file_name, entry->name, entry->name_len);
			file_name[entry->name_len] = 0;     /* append null character to the file name */
			printf("%10u %s\n", entry->inode, file_name);
			entry = (void*) entry + entry->rec_len;
			size += entry->rec_len;
		}

		free(block);
	}
} /* read_dir() */

static void read_inode(int fd,int inode_no,const struct ext2_group_desc *group,struct ext2_inode *inode)
{
	unsigned int op = group->bg_inode_table*4096+((inode_no-1)*sizeof(struct ext2_inode));
	printf("offset %u\n",op);
	lseek(fd, op, SEEK_SET);
	read(fd, inode, sizeof(struct ext2_inode));
} /* read_inode() */


int main()
{
	char name[15]="/dev/sda2";
	int input_fd,i;
	long op;
	unsigned char boot[1024],bit[4098];
	unsigned char *bitmap;

	struct ext2_super_block es;
	struct ext2_group_desc grp_desc;
	struct ext2_inode inode;

	input_fd = open( name,O_RDONLY);
	if(input_fd == -1) {
		perror("open");
	}


	/* Reads the boot section */
	read(input_fd, boot, 1024);
	op  = lseek(input_fd, 0, SEEK_CUR);
	printf("Boot block %ld\n",op);

	//	lseek(input_fd,BASE_OFFSET,SEEK_SET);
	read(input_fd, &es, sizeof(struct ext2_super_block));
	op  = lseek(input_fd, 0, SEEK_CUR);
	printf("Superblock %ld\n",op);
	
	/* Read Superblock */
       	printf("Super Block :\n Size of int  %d byts\n ", sizeof(int));
	printf("Size of char %d byts\n ", sizeof(char));
	printf("Number of of inodes: %d \n ", es.s_inodes_count);
	printf("Blocks: %d \n ", es.s_blocks_count); 
	printf("Number of free blocks: %d \n ", es.s_free_blocks_count); 
	printf("Number of free inodes: %d \n ", es.s_free_inodes_count);
	printf("Blocks per group: %d \n ", es.s_blocks_per_group);
	printf("Fragments per group: %d \n ", es.s_frags_per_group);
	printf("NM: %x\n", es.s_magic);
	printf("First Inode: %dn", es.s_first_ino);


	block_size = 1024 << es.s_log_block_size;
       	//op  = lseek(input_fd, 0, SEEK_CUR);
	printf("Block size %ld\n",(long)block_size);
	/* Read group descriptor */

	lseek(input_fd, BASE_OFFSET + block_size, SEEK_SET);
	op  = lseek(input_fd, -32*sizeof(struct ext2_group_desc), SEEK_CUR);
	printf("Before grp desc %ld\n",op);
	read(input_fd, &grp_desc, sizeof(struct ext2_group_desc));
	op  = lseek(input_fd, 0, SEEK_CUR);
	printf("after grp desc %ld\n",op);



       	printf("\nBlock Group Descriptor:\n Blocks of blocks bitmap: %d \n ",grp_desc.bg_block_bitmap);
	printf("Inode bitmap blocks: %d \n ",grp_desc.bg_inode_bitmap);	
	printf("Inode Table block: %d \n ",grp_desc.bg_inode_table);
	printf("Free Blocks Count: %d \n ",grp_desc.bg_free_blocks_count);
	printf("Free Inode Count: %d \n ",grp_desc.bg_free_inodes_count);


	/*read block bitmap from disk*/
	bitmap = malloc(block_size);
       	lseek(input_fd, grp_desc. bg_block_bitmap * block_size, SEEK_SET);

	op  = lseek(input_fd, 0, SEEK_CUR);
	printf("before bitmap %ld\n",op);

	read(input_fd, bitmap, block_size); 

       	op  = lseek(input_fd, 0, SEEK_CUR);
	printf("after bitmap %ld\n",op);
	
	//printf("%d %d",(unsigned int)bitmap,block_size);
	read(input_fd, bitmap, block_size); 
	
	
	/*read inode from disk */
	read_inode(input_fd,3,&grp_desc,&inode);
	
	printf("Reading root inode\n"
	       "File mode: %hu\n"
	       "Owner UID: %hu\n"
	       "Size     : %u bytes\n"
	       "Blocks   : %u\n"
	       ,
	       inode.i_mode,
	       inode.i_uid,
	       inode.i_size,
	       inode.i_blocks);

	for(i=0; i<EXT2_N_BLOCKS; i++)
			if (i < EXT2_NDIR_BLOCKS)         /* direct blocks */
				printf("Block %2u : %u\n", i, inode.i_block[i]);
			else if (i == EXT2_IND_BLOCK)     /* single indirect block */
				printf("Single   : %u\n", inode.i_block[i]);
			else if (i == EXT2_DIND_BLOCK)    /* double indirect block */
				printf("Double   : %u\n", inode.i_block[i]);
			else if (i == EXT2_TIND_BLOCK)    /* triple indirect block */
			printf("Triple   : %u\n", inode.i_block[i]);
	
	printf("\nReading Directories \n");
	read_dir(input_fd, &inode, &grp_desc);

	close(input_fd);
	return 0;
}
