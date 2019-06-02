#ifndef MINLS_H
#define MINLS_H

#define DIRECT_ZONES 7

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* Struct definition for a partition table entry */
struct pt_entry {
	uint8_t bootind 			/* Boot magic number (0x80 if bootable) */
	uint8_t start_head 			/* Start of partition in CHS */
	uint8_t[2] start_sec_cyl 	/* See note on sec_cyl addressing */
	uint8_t type 				/* Type of Partition (0x81 is MINIX) */
	uint8_t end_head 			/* End of partition in CHS */
	uint8_t[2] end_sec_cyl 		/* See note on sec_cyl addressing */
	uint32_t lFirst 			/* First sector (LBA addressing) */
	uint32_t size 				/* size of partition (in sectors */
}

/* Struct definition for the superblock */
struct superblock {
	uint32_t ninodes 		/* number of inodes in this fs */
	uint16_t pad1; 			/* padding to line stuff up properly */
	int16_t i_blocks; 		/* # of blocks used by inode bit map */
	int16_t z_blocks; 		/* # of blocks used by zone bit map */
	uint16_t firstdata; 	/* number of first data zone */
	int16_t log_zone_size; 	/* log2 of blocks per zone */
	uint16_t pad2; 			/* padding to line stuff up properly */
	uint32_t max_file; 		/* maximum file size */
	uint32_t zones; 		/* number of zones on disk */
	int16_t magic; 			/* magic number */
	uint16_t pad3; 			/* padding to line stuff up properly */
	uint16_t blocksize; 	/* block size in bytes */
	uint8_t subversion; 	/* filesystem sub-version */
}

/* Struct definition for a inode entry */
struct inode {
	uint16_t mode;
	uint16_t links; 	/* number of links to file */
	uint16_t uid;
	uint16_t gid;
	uint32_t size;
	int32_t atime; 		/* Last accessed time */
	int32_t mtime; 		/* Last modified time */
	int32_t ctime; 		/* Created time */
	uint32_t zone[DIRECT_ZONES];
	uint32_t indirect;
	uint32_t double_indirect;
	uint32_t unused;
}

/* Struct definition for a directory entry */
struct dirent {
	uint32_t inode; 	/* inode number */
	u_char name[60]; 	/* filename (nul-terminated if space available) */
}

/* Functions for parsing command line arguments */
void parse_args(int argc, char *argv[]);
int parse_int(char *arg);
void update_paths(char **imgfile, char **mpath, char *arg);
void update_parts(char flag, int val);
void update_verbosity();
void check_parts();
void range_part_err(char flag, int val);
void mult_part_err(char flag);
void usage_message();
void print_opts(char *imgfile, char *mpath);

#endif
