#include "minls.h"

static char *prog;
static int partitions = -1;
static int subpartitions = -1;
static int v_flag = 0;
static uint32_t fs_start = 0;
static uint32_t inode_start = 0;
static struct comp_fields comp_f;

/* ===== Functions for running minls ===== */

/* Takes the file arguments, and run minls */
void minls(char* imgfile, char* mpath) {
    FILE *fp;

	/* Opens image file */
    if((fp = fopen(imgfile, "r")) == NULL){
        perror("fopen");
        exit(-1);
    }
	
	/* Find start of filesystem from partitions */
	/* Otherwise,  unpartitioned disk image begins at start */
	if (partitions >= 0 || subpartitions >= 0) {
		get_start(fp);
	}
	
	parse_file_sys(fp, mpath);
}

/* Driver for parsing various needed info in filesystem */
void parse_file_sys(FILE *fp, char* mpath) {
    struct inode inode;
	struct superblock s_block;
    char *o_path;

	/* Store the original path name */
    if(mpath != NULL){
        set_o_path(mpath, &o_path);
    }
    else{
        o_path = NULL;
    }

    /* Parse superblock */ 
    get_sup_block(fp, &s_block);
    check_magic_number(&s_block);
	
	/* Compute offsets need to access inode table */
    get_offsets(&s_block);

    /* Generate computed fields */
    get_computed_field(&s_block);

    /* Parse root inode info */
	get_inode(fp, ROOT_INODE_IDX, &inode);
	
	/* Traverse through the path to get to correct file inode */
	traverse_path(fp, mpath, &inode, o_path);
	
	if (v_flag > 0) {
		print_inode(&inode);
	}
	
	if (check_if_dir(&inode)) {
		if(o_path != NULL){
			printf("%s:\n", o_path);
		}
		else{
			printf("/:\n");
		}
		print_dir(fp, &inode);
	}
	
	else {
		print_file(-1, o_path, &inode);
	}
}

/* Prints the contents of the directory file / inode */
void print_dir(FILE *fp, struct inode *inode) {
	struct dirent curr_dir;
	struct inode curr_inode;
    int i, index = 0;
    uint32_t zone_off;
	uint32_t num_dirents = inode->size / sizeof(struct dirent);
		
	for (i = 0; i < num_dirents; i++) {
		if (i % comp_f.ent_per_zone == 0) {
			/* Get location of file's data */
			zone_off = (inode->zone[index]) * comp_f.zonesize;
			if(fseek(fp, fs_start + zone_off, SEEK_SET) < 0) {
				perror("fseek");
				exit(-1);
			}
			index++;
		}
			
		if(fread(&curr_dir, sizeof(struct dirent), 1, fp) == 0){
			perror("fread reads nothing");
			exit(-1);
		}
			
		get_inode(fp, curr_dir.inode, &curr_inode);
		print_file(curr_dir.inode, (char *)curr_dir.name, &curr_inode);
	}
}

/* Print each individual file info */
void print_file(uint32_t inode, char* name, struct inode *curr_inode) {
	char perm[11] = {0};
	
	if(inode != 0){
		convert_mode_to_string(perm, curr_inode);
		printf("%s%10u", perm, curr_inode->size);
	    printf(" %s\n", name);
    }
}

/* ===== Functions for accessing filesystem information ===== */

/* Store original path name */
void set_o_path(char *m_path, char **o_path){
    int len = strlen(m_path);

    *o_path = calloc(1, len);
    if(*o_path == NULL){
        perror("calloc");
        exit(-1);
    }
    memcpy(*o_path, m_path, len);
}

/* Traverses through path, updates * inode to inode of deepest file */
void traverse_path(FILE *fp, char* mpath, struct inode *inode, char *o_path) {
	const char s[2] = {'/'};
	char *file_name = NULL;
	struct inode *curr_inode = inode;
	
	if (mpath != NULL) {
		file_name = strtok(mpath, s);
	}
	
	while (file_name != NULL) {
			
		/* Go to that directory file's start of data */
		if (!get_next_path_inode(fp, file_name, curr_inode, o_path)) {
			bad_file_err(o_path);
		}
		file_name = strtok(NULL, s);
	}
}

/* Updates inode * to point to inode indicated by file_name */
int get_next_path_inode(FILE *fp, char *file_name, struct inode *inode,
    char *o_path) 
{
	struct dirent curr_dir;
    int i, found = 0, index = 0;
	uint32_t zone_off, zone_num;
	uint32_t num_dirents = inode->size / sizeof(struct dirent);
	
	for (i = 0; i < num_dirents; i++) {
		if (i % comp_f.ent_per_zone == 0) {
			/* Get location of file's data */
			/*zone_off = (inode->zone[index]) * comp_f.zonesize;*/
			zone_num = get_zone_num(fp, inode, index);
			zone_off = zone_num * comp_f.zonesize;
			
			if(fseek(fp, fs_start + zone_off, SEEK_SET) < 0) {
				perror("fseek");
				exit(-1);
			}
			index++;
		}
		
		if(fread(&curr_dir, sizeof(struct dirent), 1, fp) == 0){
			perror("fread reads nothing");
			exit(-1);
		}
		
		if (strcmp((char *)curr_dir.name, file_name) == 0) {
            if (!check_if_dir(inode)){
                bad_dir_err(o_path);
            }
			if (curr_dir.inode == 0) {
				bad_file_err(o_path);
			}
			get_inode(fp, curr_dir.inode, inode);
			found = TRUE;
			break;
		}
	}
	
	return found;	
}

/* Traverses through inode to return zone indicated by the index */
uint32_t get_zone_num(FILE *fp, struct inode *inode, int index) {
	uint32_t zone_num;
	fpos_t pos;
	
	fgetpos(fp, &pos);
	
	if (index > DIRECT_ZONES - 1) {
		index -= DIRECT_ZONES;
		
		if (index > comp_f.ptrs_per_zone - 1) {
			/* Do double indirect stuff*/
			index -= comp_f.ptrs_per_zone;
		}
		
		/* Move to zone pointed to by indirect field */
		fseek(fp, fs_start + (inode->indirect * comp_f.zonesize), 
			SEEK_SET);
		/* Move to specific index location in indirect zone */
		fseek(fp, index * sizeof(zone_num), SEEK_CUR);
		fread(&zone_num, sizeof(zone_num), 1, fp);
	}
	
	else {
		zone_num = inode->zone[index];
	}
	
	fsetpos(fp, &pos);
	return zone_num;
}

/* Get superblock and prints if -v */
void get_sup_block(FILE *fp, struct superblock *sup_block) {
    if(fseek(fp, fs_start, SEEK_SET) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fseek(fp, S_BLOCK_OFFSET, SEEK_CUR) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fread(sup_block, sizeof(struct superblock), 1, fp) == 0){
        perror("fread reads nothing");
        exit(-1);
    }
	
	if (v_flag > 0) {
		print_stored_fields(sup_block);
	}
}

/* Computes inode_start based on offsets */
void get_offsets(struct superblock *sup_block) {
	uint32_t imap_offset = 0;
    uint32_t zmap_offset = 0;
    uint32_t inode_offset = 0;
	uint16_t blocksize = sup_block->blocksize;
	
	imap_offset = fs_start + 2 * blocksize;
	zmap_offset = imap_offset + sup_block->i_blocks * blocksize;
	inode_offset = zmap_offset + sup_block->z_blocks * blocksize;
	inode_start = inode_offset;
}
   
/* Computes computed fields and prints if -v */
void get_computed_field(struct superblock *sup_block) {
    comp_f.version = VERSION;
    comp_f.firstImap = 2;
    comp_f.firstZmap = comp_f.firstImap + sup_block->i_blocks;
    comp_f.firstIblock = comp_f.firstZmap + sup_block->z_blocks;
    comp_f.zonesize = sup_block->blocksize << sup_block->log_zone_size;
    comp_f.ptrs_per_zone = comp_f.zonesize / sizeof(comp_f.zonesize);
    comp_f.ino_per_block = sup_block->blocksize/sizeof(struct inode);
    comp_f.wrongended = 0;
    comp_f.fileent_size = DIRENT_B_SIZE;
    comp_f.max_filename = MAX_FN_SIZE;
    comp_f.ent_per_zone = comp_f.zonesize/sizeof(struct dirent);
	
	if (v_flag > 0) {
		print_computed_fields();
	}
}

/* Updates inode * to point to inode indicated by inode_num */
void get_inode(FILE *fp, uint32_t inode_num, struct inode *i){
	struct inode curr_inode;
	uint32_t inode_index = (inode_num - 1) * sizeof(struct inode);
	fpos_t pos;
	
	fgetpos(fp, &pos);
	
	if(fseek(fp, inode_start + inode_index, SEEK_SET) < 0){
		perror("fseek");
        exit(-1);
	}   

    if(fread(&curr_inode, sizeof(struct inode), 1, fp) == 0){
		perror("fread reads nothing");
        exit(-1);
    }
	
	fsetpos(fp, &pos);
	
	*i = curr_inode;
}

/* Converts mode of inode to a permission string */
void convert_mode_to_string(char *perm_string, struct inode *inode_info){
    uint16_t mode = inode_info->mode;
    uint16_t type_mask = mode & TYPE_MASK;
	
    if(type_mask == REG_FILE){
        perm_string[0] = '-';
        get_owner_perm(perm_string, mode);
        get_group_perm(perm_string, mode);
        get_other_perm(perm_string, mode);

    }
    else if(type_mask == DIRECTORY){
        perm_string[0] = 'd';
        get_owner_perm(perm_string, mode);
        get_group_perm(perm_string, mode);
        get_other_perm(perm_string, mode);
    }
    else{
		fprintf(stderr, "Unrecognized type_mask in "
            "convert_mode_to_string\n");
        exit(-1);
    }
}

/* Writes owner permission string */
void get_owner_perm(char *perm_string, uint16_t mode){
    if(mode & OW_READ_PERM){
        perm_string[OW_READ_INDEX] = 'r';
    }
    else{
        perm_string[OW_READ_INDEX] = '-';
    }
    if(mode & OW_WRITE_PERM){
        perm_string[OW_WRITE_INDEX] = 'w';
    }
    else{
        perm_string[OW_WRITE_INDEX] = '-';
    }
    if(mode & OW_EXEC_PERM){
        perm_string[OW_EXEC_INDEX] = 'x';
    }
    else{
        perm_string[OW_EXEC_INDEX] = '-';
    }
}

/* Writes group permission string */
void get_group_perm(char *perm_string, uint16_t mode){
    if(mode & G_READ_PERM){
        perm_string[G_READ_INDEX] = 'r';
    }
    else{
        perm_string[G_READ_INDEX] = '-';
    }
    if(mode & G_WRITE_PERM){
        perm_string[G_WRITE_INDEX] = 'w';
    }
    else{
        perm_string[G_WRITE_INDEX] = '-';
    }
    if(mode & G_EXEC_PERM){
        perm_string[G_EXEC_INDEX] = 'x';
    }
    else{
        perm_string[G_EXEC_INDEX] = '-';
    }
}

/* Writes other permission string */
void get_other_perm(char *perm_string, uint16_t mode){
    if(mode & O_READ_PERM){
        perm_string[O_READ_INDEX] = 'r';
    }
    else{
        perm_string[O_READ_INDEX] = '-';
    }
    if(mode & O_WRITE_PERM){
        perm_string[O_WRITE_INDEX] = 'w';
    }
    else{
        perm_string[O_WRITE_INDEX] = '-';
    }
    if(mode & O_EXEC_PERM){
        perm_string[O_EXEC_INDEX] = 'x';
    }
    else{
        perm_string[O_EXEC_INDEX] = '-';
    }
}

/* ===== Functions for getting start of filesystem ===== */

/* Get the start of the filesystem by traversing through partitions */
void get_start(FILE *fp) {
    int start;
    struct pt_entry part_info;
    struct pt_entry subpart_info;

    if (partitions >= 0) {
        check_valid_pt(fp);
        parse_pt_entry(fp, &part_info, partitions);
        if (v_flag > 0) {
            print_pt_table(fp, 'p');
        }
        start = part_info.lFirst * SECTOR_SIZE;

        if (subpartitions >= 0) {
            fseek(fp, part_info.lFirst * SECTOR_SIZE, SEEK_SET);
            check_valid_pt(fp);
            parse_pt_entry(fp, &subpart_info, subpartitions);
            if (v_flag > 0) {
                print_pt_table(fp, 's');
            }
            start = subpart_info.lFirst * SECTOR_SIZE;
        }
    }

    fs_start = start;
}

/* Get the start of the filesystem by traversing through partitions */
void parse_pt_entry(FILE *fp, pt_entry *p, int idx){
    fpos_t pos;

    fgetpos(fp, &pos);
    fseek(fp, PT_START + (idx * sizeof(pt_entry)), SEEK_CUR);
    fread(p, sizeof(pt_entry), 1, fp);
    fsetpos(fp, &pos);

    check_valid_part(p);
}

/* ===== Functions for parsing command line arguments ===== */

/* Parses out the command line arguments given */
void parse_args(int argc, char *argv[]) {
    int i, pval, sval, flag;
    char* imgfile = NULL;
    char* mpath = NULL;

    prog = argv[0];
    if (argc == 1) {
        usage_message();
    }

    while ((flag = getopt (argc, argv, "p:s:vh")) != -1) {
        switch (flag){
            case 'p':
                pval = parse_int(optarg);
                update_parts(flag, pval);
                break;
            case 's':
                sval = parse_int(optarg);
                update_parts(flag, sval);
                break;
            case 'h':
                usage_message();
                break;
            case 'v':
                update_verbosity();
                break;
            case '?':
                break;
        }
    }

    for(i = optind; i < argc; i++){    
        update_paths(&imgfile, &mpath, argv[i]);
    }

	/* Check for valid partitions */
    check_parts();
	
	/* Check for valid imgfiles */
    check_imgfile(imgfile);

    if (v_flag > 1) {
        print_opts(imgfile, mpath);
    }

    minls(imgfile, mpath);
}

/* Parses string argument into an int, returns int if successful */
int parse_int(char *arg) {
    int val;

    if (strcmp(arg, "0") == 0) {
        val = 0;
    }

    else {
        val = atoi(arg);

        if (val == 0) {
            fprintf(stderr, "%s: badly formed integer.\n", arg);
            usage_message();
        }
    }

    return val;
}

/* Updates path values from arg based on what is not defined yet */
void update_paths(char **imgfile, char **mpath, char *arg) {
    if (*imgfile == NULL) {
        *imgfile = arg;
    }

    else if (*mpath == NULL) {
        *mpath = arg;
    }

    else {
        usage_message();
    }
}

/* Updates part or subpart based on flag arg */
void update_parts(char flag, int val) {
    if (val > 3 || val < 0)
        range_part_err(flag, val);

    if (flag == 'p') {
        if (partitions == -1)
            partitions = val;
        else
            mult_part_err(flag);
    }

    else {
        if (subpartitions == -1)
            subpartitions = val;
        else
            mult_part_err(flag);
    }
}

/* Updates verbosity level */
void update_verbosity() {
    if (v_flag < 2)
        v_flag++;
}

/* ===== Functions for checking validity of info ===== */

/* Checks if image is a valid MINIX filesystem */
void check_magic_number(struct superblock *sup_block){
    if(sup_block->magic != MINIX_MAGIC_NUM){
        fprintf(stderr, "Bad magic number. (0x%.4x)\n"
                "This doesn't look like a MINIX filesystem.\n", 
                sup_block->magic);
        exit(-1);
    }
}

/* Checks if file is a valid directory */
int check_if_dir(struct inode *inode) {
	uint16_t mode = inode->mode;
    uint16_t type_mask = mode & TYPE_MASK;
	int dir_check = TRUE;
	
    if(type_mask == REG_FILE){
		dir_check = FALSE;
	}
    else if(type_mask == DIRECTORY){
        dir_check = TRUE; 
    }
    else{
        fprintf(stderr, "dir_check has invalid value of %d\n", type_mask);
        exit(-1);
    }
	
	return dir_check;
}

/* Checks if pt_entry's type is valid */
void check_valid_part(pt_entry *p) {
    if (p->type != PT_TYPE) {
        /* Change to stderr?? */
        printf("Invalid pt table entry\n");
        exit(-1);
    }
}

/* Checks if partition table bits are valid */
void check_valid_pt(FILE *fp) {
    uint8_t pt_check1, pt_check2;
    fpos_t pos;

    fgetpos(fp, &pos);
    fseek(fp, PT_VALID_CHECK_1, SEEK_CUR);
    fread(&pt_check1, sizeof(pt_check1), 1, fp);
    fread(&pt_check2, sizeof(pt_check2), 1, fp);

    if (pt_check1 != PT_VALID_1 || pt_check2 != PT_VALID_2) {
        /* Change to stderr?? */
        printf("Invalid pt table\n");
        exit(-1);
    }

    fsetpos(fp, &pos);
}

/* Checks if part/subpart values are valid */
void check_parts() {
    if (partitions < 0 && subpartitions >= 0) {
        printf("Cannot have a subpartition without a partition.\n");
        usage_message();
    }
}

/* Checks if an image file is specified */
void check_imgfile(char* imgfile) {
    if (imgfile == NULL) {
        usage_message();
    }
}

/* ===== Functions for printing out information ===== */

/* Prints out the usage help message to stdout and exits*/
void usage_message() {
    fprintf(stdout, "usage: %s  ", prog);
    fprintf(stdout, "[ -v ] [ -p num [ -s num ] ] imagefile " 
        "[ path ]\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t-p\t part    --- select partition for filesystem " 
        "(default: none)\n");
    fprintf(stdout, "\t-s\t sub     --- select subpartition for filesystem " 
        "(default: none)\n");
    fprintf(stdout, "\t-h\t help    --- print usage information and exit\n");
    fprintf(stdout, "\t-v\t verbose --- increase verbosity level\n");
    exit(-1);
}

/* Print options output for verbosity level 2 */
void print_opts(char *imgfile, char *mpath) {
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  opt->part      %d\n", partitions);
    fprintf(stderr, "  opt->subpart   %d\n", subpartitions);
    fprintf(stderr, "  opt->imagefile %s\n", imgfile);
    fprintf(stderr, "  opt->srcpath   %s\n", mpath);
    fprintf(stderr, "  opt->dstpath   (null)\n");
}

/* Prints out the partition or subpart table */
void print_pt_table(FILE *fp, char p_flag) {
    struct pt_entry p_info;
    fpos_t pos;
    int i;

    if (p_flag == 'p')
        printf("\nPartition table:\n");
    else
        printf("\nSubpartition table:\n");

    printf("       ----Start----      ------End-----\n");
    printf("  Boot head  sec  cyl ");
    printf("Type head  sec  cyl");
    printf("      First       Size\n");

    fgetpos(fp, &pos);
    fseek(fp, PT_START, SEEK_CUR);
    for (i = 0; i < 4; i++) {
        fread(&p_info, sizeof(pt_entry), 1, fp);
        print_pt_entry(&p_info);
    }

    fsetpos(fp, &pos);
}

/* Prints out the individual pt_entries */
void print_pt_entry(pt_entry *p) {
	uint16_t start_sec = (p->start_sec_cyl[0] & 0xc0) << 2;
	uint16_t end_sec = (p->end_sec_cyl[0] & 0xc0) << 2;
	
	printf("  0x%02x ", p->bootind);
	printf("%4d", p->start_head);
	printf("%5u", p->start_sec_cyl[0] & 0x3F);
	printf("%5u ", start_sec | p->start_sec_cyl[1]);
	
	printf("0x%02x ", p->type);
	printf("%4d", p->end_head);
	printf("%5u", p->end_sec_cyl[0] & 0x3F);
	printf("%5u ", end_sec | p->end_sec_cyl[1]);
	
	printf("%10d", p->lFirst);
	printf("%11d\n", p->size);
}

/* Prints out the stored fields portion of the superblock */
void print_stored_fields(struct superblock* sup_block) {
    uint16_t block_size = sup_block->blocksize;
    int16_t log_zone_size = sup_block->log_zone_size;
    uint32_t zone_size = block_size * (1 << log_zone_size);

    fprintf(stderr, "\nSuperblock Contents:\n");
    fprintf(stderr, "Stored Fields:\n");
    fprintf(stderr, "  ninodes%13d\n", sup_block->ninodes);
    fprintf(stderr, "  i_blocks%12d\n", sup_block->i_blocks);
    fprintf(stderr, "  z_blocks%12d\n", sup_block->z_blocks);
    fprintf(stderr, "  firstdata%11d\n", sup_block->firstdata);
    fprintf(stderr, "  log_zone_size%7d (zone size: %d)\n", 
            log_zone_size, zone_size);
    fprintf(stderr, "  max_file%12u\n", sup_block->max_file);
    fprintf(stderr, "  magic         0x""%x\n", sup_block->magic);
    fprintf(stderr, "  zones%15d\n", sup_block->zones);
    fprintf(stderr, "  blocksize%11d\n", sup_block->blocksize);
    fprintf(stderr, "  subversion%10d\n", sup_block->subversion);
}

/* Prints out the computed fields */
void print_computed_fields(){
    fprintf(stderr, "Computed Fields:\n");
    fprintf(stderr, "  version%13d\n", comp_f.version);
    fprintf(stderr, "  firstImap%11d\n", comp_f.firstImap); 
    fprintf(stderr, "  firstZmap%11d\n", comp_f.firstZmap); 
    fprintf(stderr, "  firstIblock%9d\n", comp_f.firstIblock); 
    fprintf(stderr, "  zonesize%12d\n", comp_f.zonesize); 
    fprintf(stderr, "  ptrs_per_zone%7d\n", comp_f.ptrs_per_zone); 
    fprintf(stderr, "  ino_per_block%7d\n", comp_f.ino_per_block); 
    fprintf(stderr, "  wrongended%10d\n", comp_f.wrongended); 
    fprintf(stderr, "  fileent_size%8d\n", comp_f.fileent_size); 
    fprintf(stderr, "  max_filename%8d\n", comp_f.max_filename); 
    fprintf(stderr, "  ent_per_zone%8d\n", comp_f.ent_per_zone); 
}

/* Prints out the inode's information */
void print_inode(struct inode *inode_info){
    char perm[11] = {0};
	time_t a_time = inode_info->atime;
    time_t m_time = inode_info->mtime;
    time_t c_time = inode_info->ctime;
    int i = 0;

	convert_mode_to_string(perm, inode_info);	
	fprintf(stderr, "\nFile inode:\n");
    
    fprintf(stderr, "  unsigned short mode         0x"
                    "%x\t(%s)\n", inode_info->mode, perm);
    fprintf(stderr, "  unsigned short links%14d\n", inode_info->links);
    fprintf(stderr, "  unsigned short uid%16d\n", inode_info->uid);
    fprintf(stderr, "  unsigned short uid%16d\n", inode_info->gid);
    fprintf(stderr, "  uint32_t size%15d\n", inode_info->size);

    fprintf(stderr, "  uint32_t atime%14d"" --- %s", 
        inode_info->atime, ctime(&a_time));
    fprintf(stderr, "  uint32_t mtime%14d"" --- %s", 
        inode_info->mtime, ctime(&m_time));
    fprintf(stderr, "  uint32_t ctime%14d"" --- %s\n", 
        inode_info->ctime, ctime(&c_time));
    fprintf(stderr, "  Direct zones:\n");

    for(i = 0; i < DIRECT_ZONES; i++){
        fprintf(stderr, "               zone[%d]  =%11d\n", 
            i, inode_info->zone[i]);
    }
    fprintf(stderr, "   uint32_t indirect    =%11d\n", inode_info->indirect);
    fprintf(stderr, "   uint32_t double      =%11d\n", 
        inode_info->double_indirect);
}

/* ===== Functions for printing out error messages ===== */

/* Error message for invalid directory */
void bad_dir_err(char *o_path){
    fprintf(stderr, "lookupdir: Not a directory\n");
	fprintf(stderr, "%s: File not found\n", o_path);
	exit(-1);
}

/* Error message for file not found */
void bad_file_err(char *o_path){
	fprintf(stderr, "%s: File not found\n", o_path);
	exit(-1);
}

/* Error message for partitions out of range */
void range_part_err(char flag, int val) {
    if (flag == 'p')
        fprintf(stderr, 
			"Partition %d out of range.  Must be 0..3.\n", val);
    else
        fprintf(stderr, 
			"Subpartition %d out of range.  Must be 0..3.\n", val);

    usage_message();
}

/* Error message for multiple partitions specified */
void mult_part_err(char flag) {
    if (flag == 'p') 
        fprintf(stderr, "%s: more than one partition specified.\n", prog);
    else
        fprintf(stderr, "%s: more than one subpartition specified.\n", prog);

    usage_message();
}
