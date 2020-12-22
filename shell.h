/* Do not modify this file */

#ifndef SHELL_H
#define SHELL_H

#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdbool.h>
#include<errno.h>
#include<signal.h>

#include "logging.h"

#define MAXLINE 100 /* the max number of chars in one command line */
#define MAXARGS 50 /* the max number of arguments in one command line */

#endif /*SHELL_H*/