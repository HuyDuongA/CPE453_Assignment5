#include "minls.h"

static char *prog;
static int partitions = -1;
static int subpartitions = -1;
static int v_flag = 0;
static struct superblock sup_block;
static struct inode inode_info;
static uint32_t fs_start = 0;

/* ============ Functions for no partition nor subpartition === */
void parse_file_sys(FILE *fp){
    uint32_t imap_offset = 0;
    uint32_t zmap_offset = 0;
    uint32_t inode_offset = 0;

    /* parse superblock */ 
    get_sup_block(fp);

    /* parse inode info */
    get_offsets(&imap_offset, &zmap_offset, &inode_offset);
    get_inode_info(fp, inode_offset);

    print_sup_block(imap_offset, zmap_offset, inode_offset);
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
    uint32_t *inode_offset)
{
   uint16_t blocksize = sup_block.blocksize;

   *imap_offset = fs_start + 2 * blocksize; 
   *zmap_offset = *imap_offset + sup_block.i_blocks * blocksize;
   *inode_offset = *zmap_offset + sup_block.z_blocks * blocksize;
}

void get_inode_info(FILE *fp, uint32_t inode_offset){
    if(fseek(fp, fs_start, SEEK_SET) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fseek(fp, inode_offset, SEEK_CUR) < 0){
        perror("fseek");
        exit(-1);
    }

    if(fread(&inode_info, sizeof(inode_info), 1, fp) == 0){
        perror("fread reads nothing");
        exit(-1);
    }
    
}

void print_sup_block(uint32_t imap_offset, uint32_t zmap_offset,
    uint32_t inode_offset)
{
    print_stored_fields();
    print_inode();
    print_dir();
}

void print_stored_fields(){
    uint16_t block_size = sup_block.blocksize;
    int16_t log_zone_size = sup_block.log_zone_size;
    uint32_t zone_size = block_size * (1 << log_zone_size);

    fprintf(stderr, "Superblock Contents:\n");
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

void print_inode(){
    char perm[11] = {0};
    time_t a_time = inode_info.atime;
    time_t m_time = inode_info.mtime;
    time_t c_time = inode_info.ctime;
    int i = 0;

    convert_mode_to_string(perm);
    fprintf(stderr, "\nFile inode:\n");
    /*TODO print mode*/
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
        inode_info.double_indirect);
}

void convert_mode_to_string(char *perm_string){
    uint16_t mode = inode_info.mode;
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
void print_dir(){
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
	
	/*else if (partitions >= 0 && subpartitions < 0) {
		// move fp, then call superblock func, passing in new fp 
	}
	
	else {
		// move fp, then call superblock func, passing in new fp 
	}*/
	
	/*if (v_flag > 0) {
		print_superblock();
	}*/
}

/*FILE * get_start(FILE *fp) {
	pt_entry part_info;
	
	fseek(fp, PT_START + partitions * sizeof(pt_entry), SEEK_SET);
	fread(&pt_entry, sizeof(pt_entry), 1, fp);
}*/


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
	
	while ((flag = getopt (argc, argv, "s:p:hv")) != -1) {
		switch (flag){
			case 'h':
				usage_message();
				break;
			case 'v':
				update_verbosity();
				break;
			case 'p':
				pval = parse_int(optarg);
				update_parts(flag, pval);
				break;
			case 's':
				sval = parse_int(optarg);
				update_parts(flag, sval);
				break;
		}
	}
		  
	for(i = optind; i < argc; i++){      
		update_paths(&imgfile, &mpath, argv[i]);
    }
	
	check_parts();
	check_imgfile(imgfile);
	print_opts(imgfile, mpath);
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
		*imgfile = arg;
	}
	
	else if (*mpath == NULL) {
		*mpath = arg;
	}
	
	else {
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
	printf("\n  verbosity-> %d\n", v_flag);
}

