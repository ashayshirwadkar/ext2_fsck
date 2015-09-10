OBJ_FILES := $(SRC_FILES:%.c=%.o)

ext2_fsck: ext2_fsck.c
	gcc -o ext2_fsck ext2_fsck.c -I.
clean:
	rm ext2_fsck
