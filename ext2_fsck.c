#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <./linux/ext2_fs.h> 


#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)


static unsigned int block_size = 0;


int main()
{
	char name[15]="/dev/sda2";
	int input_fd,op,i=10;
	unsigned char boot[1024],bit[4098];
	unsigned char *bitmap;
	struct ext2_super_block es;
	struct ext2_group_desc grp_desc;

	input_fd = open( name,O_RDONLY);
	if(input_fd == -1) {
		perror("open");
	}


	/* Reads the boot section */
	read(input_fd, boot, 1024);


	lseek(input_fd,BASE_OFFSET,SEEK_SET);
	read(input_fd, &es, sizeof(struct ext2_super_block));
	
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


	block_size = 1024 << es.s_log_block_size;
	
	/* read group descriptor */

	lseek(input_fd, BASE_OFFSET + block_size, SEEK_SET);
	read(input_fd, &grp_desc, sizeof(struct ext2_group_desc));

       	printf("\nBlock Group Descriptor:\n Blocks of blocks bitmap: %d \n ",grp_desc.bg_block_bitmap);
	printf("Inode bitmap blocks: %d \n ",grp_desc.bg_inode_bitmap);	
	printf("Inode Table block: %d \n ",grp_desc.bg_inode_table);
	printf("Free Blocks Count: %d \n ",grp_desc.bg_free_blocks_count);
	printf("Free Inode Count: %d \n ",grp_desc.bg_free_inodes_count);
	printf("Inode Table block: %d \n ",grp_desc.bg_inode_bitmap);
	

	/*read block bitmap from disk*/
	bitmap = malloc(block_size);
	lseek(input_fd, BLOCK_OFFSET(grp_desc.bg_block_bitmap), SEEK_SET);
	read(input_fd, bitmap, block_size); 
	
	printf("%d %d",(unsigned int)bitmap,block_size);

	close(input_fd);
	return 0;
}
