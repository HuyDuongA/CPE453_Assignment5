#include "minls.h"

static char *prog;
static int partitions = -1;
static int subpartitions = -1;
static int v_flag = 0;
static struct superblock sup_block;

/* ============ Functions for no partition nor subpartition === */
void parse_file_sys(FILE *fp){
    /* parse supper block */ 
    if(fseek(fp, S_BLOCK_OFFSET, SEEK_CUR) < 0){
        perror("fseek");
        exit(-1);
    }
    if(fread(&sup_block, sizeof(sup_block), 1, fp) == 0){
        perror("fread");
        exit(-1);
    }
    print_sup_block();
}

void print_sup_block(){
    uint16_t block_size = sup_block.blocksize;
    int16_t log_zone_size = sup_block.log_zone_size;
    uint32_t zone_size = block_size * (1 << log_zone_size);

    printf("Superblock Contents:\n");
    printf("Stored Fields:\n");
    printf("  ninodes %d\n", sup_block.ninodes);
    printf("  i_blocks %d\n", sup_block.i_blocks);
    printf("  z_blocks %d\n", sup_block.z_blocks);
    printf("  firstdata %d\n", sup_block.firstdata);
    printf("  log_zone_size %d (zone size: %d)\n", 
        log_zone_size, zone_size);
    printf("  max_file %u\n", sup_block.max_file);
    printf("  magic 0x%x\n", sup_block.magic);
    printf("  zones %d\n", sup_block.zones);
    printf("  blocksize %d\n", sup_block.blocksize);
    printf("  subversion %d\n", sup_block.subversion);
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

