#include "minget.h"

char *prog;
int partitions;
int subpartitions;

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
void parseArgs(int argc, char *argv[]) {
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
}

/* Updates argument values based on what is not defined yet */
int check_tags(char *arg) {
	int val, stat;
	
	if (arg[0] == 'p' || arg[0] == 's') {
		if (arg[1]) == '\0') {
			return 1;
		}
		
		val = parse_int(arg + 1);
		update_parts(arg[0], val);
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

void invalid_opt_err (char opt) {
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

	