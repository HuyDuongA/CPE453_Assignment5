#include "minls.h"

static char *prog;
static int partitions = -1;
static int subpartitions = -1;
static int v_flag = 0;
static struct superblock sup_block;
/*static struct inode inode_info;*/
static uint32_t fs_start = 0;
static uint32_t inode_start = 0;

/* ============ Functions for no partition nor subpartition === */
void parse_file_sys(FILE *fp){
    uint32_t imap_offset = 0;
    uint32_t zmap_offset = 0;
    uint32_t inode_offset = 0;
	struct inode root_inode;

    /* parse superblock */ 
    get_sup_block(fp);

    /* parse inode info */
    get_offsets(&imap_offset, &zmap_offset, &inode_offset);
    /*get_inode_info(fp, inode_offset);*/
	get_inode(fp, 1, &root_inode);

	if (v_flag > 0) {
		print_stored_fields();
		print_inode(&root_inode);
	}
	
	print_dir(fp, &root_inode);
}

void get_sup_block(FILE *fp){
    if(fseek(fp, fs_start, SEEK_SET) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fseek(fp, S_BLOCK_OFFSET, SEEK_CUR) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fread(&sup_block, sizeof(sup_block), 1, fp) == 0){
        perror("fread reads nothing");
        exit(-1);
    }
}

void get_offsets(uint32_t *imap_offset, uint32_t *zmap_offset, 
    uint32_t *inode_offset) {
   uint16_t blocksize = sup_block.blocksize;

   *imap_offset = fs_start + 2 * blocksize; 
   *zmap_offset = *imap_offset + sup_block.i_blocks * blocksize;
   *inode_offset = *zmap_offset + sup_block.z_blocks * blocksize;
   inode_start = *inode_offset;
}

/*void get_inode_info(FILE *fp, uint32_t inode_offset){
    if(fseek(fp, inode_offset, SEEK_SET) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fread(&inode_info, sizeof(inode_info), 1, fp) == 0){
        perror("fread reads nothing");
        exit(-1);
    }
}*/

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
	
	/*memcpy(i, &curr_inode, sizeof(struct inode));*/
	*i = curr_inode;
}

void print_stored_fields(){
    uint16_t block_size = sup_block.blocksize;
    int16_t log_zone_size = sup_block.log_zone_size;
    uint32_t zone_size = block_size * (1 << log_zone_size);

    fprintf(stderr, "\nSuperblock Contents:\n");
    fprintf(stderr, "Stored Fields:\n");
    fprintf(stderr, "  ninodes%13d\n", sup_block.ninodes);
    fprintf(stderr, "  i_blocks%12d\n", sup_block.i_blocks);
    fprintf(stderr, "  z_blocks%12d\n", sup_block.z_blocks);
    fprintf(stderr, "  firstdata%11d\n", sup_block.firstdata);
    fprintf(stderr, "  log_zone_size%7d (zone size: %d)\n", 
        log_zone_size, zone_size);
    fprintf(stderr, "  max_file%12u\n", sup_block.max_file);
    fprintf(stderr, "  magic         0x""%x\n", sup_block.magic);
    fprintf(stderr, "  zones%15d\n", sup_block.zones);
    fprintf(stderr, "  blocksize%11d\n", sup_block.blocksize);
    fprintf(stderr, "  subversion%10d\n", sup_block.subversion);
}

void print_inode(struct inode *inode_info){
    char perm[11] = {0};
    /*time_t a_time = inode_info.atime;
    time_t m_time = inode_info.mtime;
    time_t c_time = inode_info.ctime;*/
	time_t a_time = inode_info->atime;
    time_t m_time = inode_info->mtime;
    time_t c_time = inode_info->ctime;
    int i = 0;

    /*TODO print mode*/
	convert_mode_to_string(perm, inode_info);
    /*fprintf(stderr, "\nFile inode:\n");
    fprintf(stderr, "  unsigned short mode         0x"
                    "%x\t(%s)\n", inode_info.mode, perm);
    fprintf(stderr, "  unsigned short links%14d\n", inode_info.links);
    fprintf(stderr, "  unsigned short uid%16d\n", inode_info.uid);
    fprintf(stderr, "  unsigned short uid%16d\n", inode_info.gid);
    fprintf(stderr, "  uint32_t size%15d\n", inode_info.size);

    fprintf(stderr, "  uint32_t atime%14d"" --- %s", 
        inode_info.atime, ctime(&a_time));
    fprintf(stderr, "  uint32_t mtime%14d"" --- %s", 
        inode_info.mtime, ctime(&m_time));
    fprintf(stderr, "  uint32_t ctime%14d"" --- %s\n", 
        inode_info.ctime, ctime(&c_time));
    fprintf(stderr, "  Direct zones:\n");
    
    for(i = 0; i < DIRECT_ZONES; i++){
        fprintf(stderr, "               zone[%d]  =%11d\n", 
            i, inode_info.zone[i]);
    }
    fprintf(stderr, "   uint32_t indirect    =%11d\n", inode_info.indirect);
    fprintf(stderr, "   uint32_t double      =%11d\n", 
        inode_info.double_indirect);*/
		
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
        fprintf(stderr, "Unrecognized type_mask in convert_mode_to_string\n");
        exit(-1);
    }
}

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
void print_dir(FILE *fp, struct inode *inode_ent){
	struct dirent curr_dirent;
	struct inode curr_inode;
	int i;
	
	uint16_t block_size = sup_block.blocksize;
    int16_t log_zone_size = sup_block.log_zone_size;
    uint32_t zone_size = block_size * (1 << log_zone_size);
	uint32_t zone_num = (inode_ent->zone[0]) * zone_size;
	uint32_t num_dirents = inode_ent->size / sizeof(struct dirent);
	
	printf("\nZone Num %d with location %d \nNum of dirents = %d\n", inode_ent->zone[0], zone_num, num_dirents);
	
	if(fseek(fp, fs_start, SEEK_SET) < 0){
			perror("fseek");
			exit(-1);
	}
	
	if(fseek(fp, zone_num, SEEK_CUR) < 0){
			perror("fseek");
			exit(-1);
	}
	
	for (i = 0; i < num_dirents ; i++) {
		if(fread(&curr_dirent, sizeof(struct dirent), 1, fp) == 0){
			perror("fread reads nothing");
			exit(-1);
		}
		
		printf("inode: %u name: %s ", curr_dirent.inode, curr_dirent.name);
		
		get_inode(fp, curr_dirent.inode, &curr_inode);
		
		printf("size: %u\n", curr_inode.size);
	}
}

/* ============ Functions for accessing image info ============ */
void minls(char* imgfile, char* mpath) {
	FILE *fp;
	
	if((fp = fopen(imgfile, "r")) == NULL){
        perror("fopen");
        exit(-1);
    }
	
	if (partitions < 0 && subpartitions < 0) {
		/* call superblock func here, pass in fp */
		parse_file_sys(fp);
	}
	
	else {
		/* move fp, then call superblock func, passing in new fp */
		get_start(fp);
		parse_file_sys(fp);
	}
}

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
	/*fseek(fp, start, SEEK_SET);*/
}

void parse_pt_entry(FILE *fp, pt_entry *p, int idx){
	fpos_t pos;
	
	fgetpos(fp, &pos);
	fseek(fp, PT_START + (idx * sizeof(pt_entry)), SEEK_CUR);
	fread(p, sizeof(pt_entry), 1, fp);
	fsetpos(fp, &pos);
	
	check_valid_part(p);
}

void check_valid_part(pt_entry *p) {
	/*print_pt_entry(p);*/
	if (p->type != PT_TYPE) {
		/* Change to stderr?? */
		printf("Invalid pt table entry\n");
		exit(-1);
	}
}

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


/* ============ Functions for parsing command line arguments ============ */

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
				/*printf("in p, optarg is %s\n", optarg);*/
				pval = parse_int(optarg);
				update_parts(flag, pval);
				break;
			case 's':
				/*printf("in s, optarg is %s\n", optarg);*/
				sval = parse_int(optarg);
				update_parts(flag, sval);
				break;
			case 'h':
				/*printf("in h\n");*/
				usage_message();
				break;
			case 'v':
				/*printf("in v\n");*/
				update_verbosity();
				break;
			case '?':
				/*printf("in ? (WHY!)\n");*/
				break;
		}
	}
		  
	/*printf("optind %d\n", optind);*/
	for(i = optind; i < argc; i++){    
		/*printf("argv[%d] = %s\n", optind, argv[optind]);*/
		update_paths(&imgfile, &mpath, argv[i]);
    }
	
	check_parts();
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
			printf("%s: badly formed integer.\n", arg);
			usage_message();
		}
	}
		
	return val;
}

/* Updates path values from arg based on what is not defined yet */
void update_paths(char **imgfile, char **mpath, char *arg){
	if (*imgfile == NULL) {
		/*printf("adding %s to imgfile\n", arg);*/
		*imgfile = arg;
	}
	
	else if (*mpath == NULL) {
		/*printf("adding %s to mpath\n", arg);*/
		*mpath = arg;
	}
	
	else {
		/*printf("in update_paths(), adding %s\n", arg);*/
		usage_message();
	}
}

/* Updates part/subpart based on flag arg */
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

/* Error message for partitions out of range */
void range_part_err(char flag, int val) {
	if (flag == 'p')
		printf("Partition %d out of range.  Must be 0..3.\n", val);
	else
		printf("Subpartition %d out of range.  Must be 0..3.\n", val);
	
	usage_message();
}

/* Error message for multiple partitions specified */
void mult_part_err(char flag) {
	if (flag == 'p') 
		printf("%s: more than one partition specified.\n", prog);
	else
		printf("%s: more than one subpartition specified.\n", prog);
	
	usage_message();
}

/* Prints out the usage help message to stdout and exits*/
void usage_message() {
	printf("usage: %s  ", prog);
	printf("[ -v ] [ -p num [ -s num ] ] imagefile minixpath [ hostpath ]\n");
	printf("Options:\n");
	printf("        -p       part    ");
	printf("--- select partition for filesystem (default: none)\n");
	printf("        -s       sub     ");
	printf("--- select subpartition for filesystem (default: none)\n");
	printf("        -h       help    ");
	printf("--- print usage information and exit\n");
	printf("        -v       verbose ");
	printf("--- increase verbosity level\n");
	exit(0);
}

void print_opts(char *imgfile, char *mpath) {
	printf("\nOptions:\n");
	printf("  opt->part      %d\n", partitions);
	printf("  opt->subpart   %d\n", subpartitions);
	printf("  opt->imagefile %s\n", imgfile);
	printf("  opt->srcpath   %s\n", mpath);
	printf("  opt->dstpath   (null)\n");
	/*printf("\n  verbosity-> %d\n", v_flag);*/
}

