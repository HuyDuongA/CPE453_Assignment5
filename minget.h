#ifndef MINGET_H
#define MINGET_H

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* Prints out the usage help message to stdout */
void usage_message();
void parse_args(int argc, char *argv[]);
int parse_int(char *arg);
int check_tags(char *arg, char *p_tag);
void update_parts(char tag, int val);
void update_paths(char **imgfile, char **mpath, char **hpath, char *arg);
void update_verbosity();
void invalid_opt_err (char opt);
void mult_part_err(char tag);
void print_opts(char *imgfile, char *mpath, char *hpath);

#endif
