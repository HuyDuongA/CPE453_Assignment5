#include "minget.h"

char *prog;
int partitions = -1;
int subpartitions = -1;
int v_flag = 0;

/* ============ Functions for parsing command line arguments ============ */

/* Parses out the command line arguments given */
void parse_args(int argc, char *argv[]) {
	int i, pval, sval, flag;
	char* imgfile = NULL;
	char* mpath = NULL;
	char* hpath = NULL;
	
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
		update_paths(&imgfile, &mpath, &hpath, argv[i]);
    }
	
	check_parts();
	print_opts(imgfile, mpath, hpath);
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
void update_paths(char **imgfile, char **mpath, char **hpath, char *arg){
	if (*imgfile == NULL) {
		*imgfile = arg;
	}
	
	else if (*mpath == NULL) {
		*mpath = arg;
	}
	
	else if (*hpath == NULL) {
		*hpath = arg;
	}
	
	else {
		usage_message();
	}
}

/* Updates part/subpart based on the part flag */
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

void print_opts(char *imgfile, char *mpath, char *hpath) {
	printf("\nOptions:\n");
	printf("  opt->part      %d\n", partitions);
	printf("  opt->subpart   %d\n", subpartitions);
	printf("  opt->imagefile %s\n", imgfile);
	printf("  opt->srcpath   %s\n", mpath);
	printf("  opt->dstpath   %s\n", hpath);
	printf("\n  verbosity-> %d\n", v_flag);
}
	