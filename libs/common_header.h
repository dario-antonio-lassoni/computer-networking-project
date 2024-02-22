#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "logger.h"

#define BOOKING_CODE_LEN 10
#define CMD_LOGIN_LEN 16

struct cmd_struct {
	char* cmd;
	void* args[6];
};

#endif
