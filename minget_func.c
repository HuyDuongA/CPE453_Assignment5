#include "minget.h"

char *prog;
int partitions = -1;
int subpartitions = -1;
int v_tag = 0;

/* ============ Functions for parsing command line arguments ============ */

/* Parses out the command line arguments given */
void parse_args(int argc, char *argv[]) {
	int check_next = 0, i = 1, val;
	char p_tag;
	char* imgfile = NULL;
	char* mpath = NULL;
	char* hpath = NULL;
	
	prog = argv[0];
	if (argc == 1) {
		usage_message();
	}
	
	for (i = 1; i < argc; i++)
	{
		if (check_next) {
			val = parse_int(argv[i]);
			update_parts(p_tag, val);
			check_next = 0;
		}
			
		else if (argv[i][0] == '-')
		{
			check_next = check_tags(argv[i] + 1, &p_tag);
		}

		else
		{
			update_paths(&imgfile, &mpath, &hpath, argv[i]);
		}
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

/* Updates verbosity level */
void update_verbosity() {
	if (v_tag < 2)
		v_tag++;
}

/* Updates part/subpart based on flag (tag) arg */
void update_parts(char tag, int val) {
	if (val > 3 || val < 0)
		range_part_err(tag, val);
	
	if (tag == 'p') {
		if (partitions == -1)
			partitions = val;
		else
			mult_part_err(tag);
	}
	
	else {
		if (subpartitions == -1)
			subpartitions = val;
		else
			mult_part_err(tag);
	}
}

/* Checks the value of the flag passed in command line */
int check_tags(char *arg, char *p_tag) {
	int val;
	int i = 0;
	
	while (i < strlen(arg)) {
		if (arg[i] == 'p' || arg[i] == 's') {
			if (arg[i + 1] == '\0') {
				*p_tag = arg[i];
				return 1;
			}
			
			val = parse_int(arg + i + 1);
			update_parts(arg[i], val);
			i++;
		}
		
		else if(arg[i] == 'v') {
			update_verbosity();
		}
		
		else if (arg[i] == 'h') {
			usage_message();
		}
		
		else {
			invalid_opt_err(arg[i]);
		}
		
		i++;
	}
	
	return 0;
}

/* Checks if part/subpart values are valid */
void check_parts() {
	if (partitions < 0 && subpartitions >= 0) {
		printf("Cannot have a subpartition without a partition.\n");
		usage_message();
	}
}

/* Error message for partitions out of range */
void range_part_err(char tag, int val) {
	if (tag == 'p')
		printf("Partition %d out of range.  Must be 0..3.\n", val);
	else
		printf("Subpartition %d out of range.  Must be 0..3.\n", val);
	
	usage_message();
}

/* Error message for invalid flag values */
void invalid_opt_err(char opt) {
	printf("%s: invalid option -- '%c'\n", prog, opt);
}

/* Error message for multiple partitions specified */
void mult_part_err(char tag) {
	if (tag == 'p') 
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
	printf("\n  verbosity-> %d\n", v_tag);
}
	