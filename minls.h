#ifndef MINLS_H
#define MINLS_H

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* Functions for parsing command line arguments */
void parse_args(int argc, char *argv[]);
int parse_int(char *arg);
void update_paths(char **imgfile, char **mpath, char *arg);
void update_parts(char flag, int val);
void update_verbosity();
void check_parts();
void range_part_err(char flag, int val);
void mult_part_err(char flag);
void usage_message();
void print_opts(char *imgfile, char *mpath);

#endif
