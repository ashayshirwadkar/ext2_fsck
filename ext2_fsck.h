#ifndef __EXT2_FSCK_H
#define __EXT2_FSCK_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <./linux/ext2_fs.h> 
#include <sys/types.h>
#include <sys/stat.h>


#define BASE_OFFSET 1024


static unsigned int block_size = 0;

inline unsigned int block_offset(unsigned int block);
static void read_dir(int fd, const struct ext2_inode *inode, 
					 const struct ext2_group_desc *group);
static void read_inode(int fd, int inode_no, 
						const struct ext2_group_desc *group,
						struct ext2_inode *inode);

#endif