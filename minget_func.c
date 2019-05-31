#include "minget.h"

char *prog;
int partitions = NULL;
int subpartitions = NULL;
int v_tag = 0;

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

/* Parses out the command line arguments given */
void parse_args(int argc, char *argv[]) {
	int check_next;
	char* imgfile == NULL;
	char* mpath == NULL;
	char* hpath == NULL;
	int i = 1;
	
	prog = argv[0];
	if (argc == 1) {
		usage_message();
	}
	
	while (i < argc)
	{
		if (argv[i][0] == '-')
		{
			check_next = check_tags(argv[i][1]);
			if (check_next) {
				i++;
				parse_int(argv[i]);
			}
		}

		else
		{
			update_paths(&imgfile, &mpath, &hpath, argv[i], argv[0]);
		}
		
		i++;
	}
	
	print_opts(imgfile, mpath, hpath);
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

/* Updates argument values based on what is not defined yet */
int check_tags(char *arg) {
	int val, stat;
	
	for (int i = 0; i < strlen(arg); i++) {
		if (arg[i] == 'p' || arg[i] == 's') {
			if (arg[i + 1]) == '\0') {
				return 1;
			}
			
			val = parse_int(arg + i + 1);
			update_parts(arg[i], val);
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
	}
}

int parse_int(char *arg) {
	int val, stat;
	
	stat = sscanf(arg, "%d", &val);
	if (stat == EOF) {
		printf("%s: badly formed integer.\n", arg);
		usage_message();
	}
		
	return val;
}

void update_verbosity() {
	if (v_tag < 2)
		v_tag++;
}

void update_parts(char tag, int val) {
	if (tag == 'p') {
		if (partitions == NULL)
			partitions = val;
		else
			mult_part_err(tag);
	}
	
	else {
		if (subpartitions == NULL)
			subpartitions = val;
		else
			mult_part_err(tag);
	}
}

void invalid_opt_err(char opt) {
	printf("%s: invalid option -- '%c'\n", prog, opt);
}

void mult_part_err(char tag) {
	if (tag == 'p') 
		printf("%s: more than one partition specified.\n", prog);
	else
		printf("%s: more than one subpartition specified.\n", prog);
	
	usage_message();
}

/* Updates argument values based on what is not defined yet */
void update_paths(char **imgfile, char **mpath, char **hpath, char *arg){
	if (*imgfile == NULL) {
		*imgfile = arg;
	}
	
	else if (*minixpath == NULL) {
		*minixpath == arg;
	}
	
	else if (*hostpath == NULL) {
		*hostpath == arg;
	}
	
	else {
		usage_message();
	}
}

	