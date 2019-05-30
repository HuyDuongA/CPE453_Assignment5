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
void parseArgs(int argc, char *argv[]);
int check_tags(char *arg);
int parse_int(char *arg);
void update_parts(char tag, int val);
void invalid_opt_err (char opt);
void mult_part_err(char tag);
void update_paths(char **imgfile, char **mpath, char **hpath, char *arg);

#endif
