#ifndef MINLS_H
#define MINLS_H

#define DIRECT_ZONES    7
#define PT_START        0x1BE
#define PT_TYPE         0x81
#define PT_VALID_1      0x55
#define PT_VALID_2      0xAA
#define MINIX_MAGIC_NUM 0x4D5A
#define INODE_B_SIZE 	64
#define DIRENT_B_SIZE 	64
#define SECTOR_SIZE 	512
#define S_BLOCK_OFFSET 	1024
#define PT_VALID_CHECK_1 510
#define INODE_B_SIZE 	64
#define DIRENT_B_SIZE 	64
#define SECTOR_SIZE 	512
#define S_BLOCK_OFFSET 	1024
#define INODE_B_SIZE    64
#define DIRENT_B_SIZE   64
#define SECTOR_SIZE     512
#define S_BLOCK_OFFSET  1024
#define ROOT_INODE_IDX	1

#define VERSION         3
#define MAX_FN_SIZE     60

#define TYPE_MASK       0170000
#define REG_FILE        0100000
#define DIRECTORY       0040000

#define OW_READ_PERM    0000400
#define OW_WRITE_PERM   0000200
#define OW_EXEC_PERM    0000100
#define OW_READ_INDEX   1
#define OW_WRITE_INDEX  2
#define OW_EXEC_INDEX   3

#define G_READ_PERM     0000040
#define G_WRITE_PERM    0000020
#define G_EXEC_PERM     0000010
#define G_READ_INDEX    4
#define G_WRITE_INDEX   5
#define G_EXEC_INDEX    6

#define O_READ_PERM     0000004
#define O_WRITE_PERM    0000002
#define O_EXEC_PERM     0000001
#define O_READ_INDEX    7
#define O_WRITE_INDEX   8
#define O_EXEC_INDEX    9

#define PTR_PER_ZONE    1024
#define _BSD_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

/* Struct definition for a partition table entry */
typedef struct __attribute__ ((__packed__)) pt_entry {
    uint8_t bootind; 			/* Boot magic number*/
    uint8_t start_head; 		/* Start of partition in CHS */
    uint8_t start_sec_cyl[2]; 	/* See note on sec_cyl addressing */
    uint8_t type; 				/* Type of Partition*/
    uint8_t end_head; 			/* End of partition in CHS */
    uint8_t end_sec_cyl[2]; 	/* See note on sec_cyl addressing */
    uint32_t lFirst; 			/* First sector*/
    uint32_t size; 				/* size of partition*/
} pt_entry;

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

/* Struct definition for computed fields in superblock contents */
struct __attribute__ ((__packed__)) comp_fields {
    uint32_t version;
    uint8_t firstImap;
    uint8_t firstZmap;
    uint8_t firstIblock;
    uint32_t zonesize;
    uint32_t ptrs_per_zone;
    uint32_t ino_per_block;
    uint32_t wrongended;
    uint32_t fileent_size;
    uint32_t max_filename;
    uint32_t ent_per_zone;
};

void minls(char* imgfile, char* mpath);
void get_start(FILE *fp);
void parse_pt_entry(FILE *fp, pt_entry *p, int idx);
void check_valid_part(pt_entry *p);
void check_valid_pt(FILE *fp);
void print_pt_table(FILE *fp, char p_flag);
void print_pt_entry(pt_entry *p);

/* Functions for parsing command line arguments */
void parse_args(int argc, char *argv[]);
int parse_int(char *arg);
void update_paths(char **imgfile, char **mpath, char *arg);
void update_parts(char flag, int val);
void update_verbosity();
void check_parts();
void check_imgfile(char* imgfile);
void range_part_err(char flag, int val);
void mult_part_err(char flag);
void usage_message();
void print_opts(char *imgfile, char *mpath);

void parse_file_sys(FILE *fp, char* mpath);
void set_cursor_s_field(FILE *fp);
void get_sup_block(FILE *fp);
void check_magic_number();
void get_computed_field();
void get_offsets(uint32_t *imap_offset, uint32_t *zmap_offset, 
    uint32_t *inode_offset);

void print_sup_block(uint32_t imap_offset, uint32_t zmap_offset, 
    uint32_t inode_offset);
void print_stored_fields();
void print_inode(struct inode *inode_info);
void convert_mode_to_string(char *perm_string, struct inode *inode_info);
void print_computed_fields();

void get_owner_perm(char *perm_string, uint16_t mode);
void get_group_perm(char *perm_string, uint16_t mode);
void get_other_perm(char *perm_string, uint16_t mode);
void print_dir(FILE *fp, struct inode *inode_ent);
void print_file(struct dirent *curr_dir, struct inode *curr_inode);

void get_inode(FILE *fp, uint32_t inode_num, struct inode *i);

#endif
