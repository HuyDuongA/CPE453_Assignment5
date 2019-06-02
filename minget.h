#ifndef MINLS_H
#define MINLS_H

#define DIRECT_ZONES 7
#define PT_START 0x1BE
#define PT_TYPE 0x81
#define PT_VALID_1 0x55
#define PT_VALID_2 0xAA
#define MINIX_MAGIC_NUM 0x4D5A
#define INODE_B_SIZE 64
#define DIRENT_B_SIZE 64

#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/* Struct definition for a partition table entry */
struct __attribute__ ((__packed__)) pt_entry {
	uint8_t bootind; 			/* Boot magic number (0x80 if bootable) */
	uint8_t start_head; 		/* Start of partition in CHS */
	uint8_t start_sec_cyl[2]; 	/* See note on sec_cyl addressing */
	uint8_t type; 				/* Type of Partition (0x81 is MINIX) */
	uint8_t end_head; 			/* End of partition in CHS */
	uint8_t end_sec_cyl[2]; 	/* See note on sec_cyl addressing */
	uint32_t lFirst; 			/* First sector (LBA addressing) */
	uint32_t size; 				/* size of partition (in sectors */
};

/* Struct definition for the superblock */
struct __attribute__ ((__packed__)) superblock {
	uint32_t ninodes; 		/* number of inodes in this fs */
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
};

/* Struct definition for a inode entry */
struct __attribute__ ((__packed__)) inode {
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
};

/* Struct definition for a directory entry */
struct __attribute__ ((__packed__)) dirent {
	uint32_t inode; 	/* inode number */
	uint8_t name[60]; 	/* filename (nul-terminated if space available) */
};

/* Functions for parsing command line arguments */
void parse_args(int argc, char *argv[]);
int parse_int(char *arg);
void update_paths(char **imgfile, char **mpath, char **hpath, char *arg);
void update_parts(char flag, int val);
void update_verbosity();
void check_parts();
void check_imgfile(char* imgfile);
void range_part_err(char flag, int val);
void mult_part_err(char flag);
void usage_message();
void print_opts(char *imgfile, char *mpath, char *hpath);

#endif
