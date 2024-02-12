#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "logger.h"

struct cmd_struct {
	char* cmd;
	void* args[6];
};

#endif
