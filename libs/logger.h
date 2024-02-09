#ifndef LOGGER_H
#define LOGGER_H

#include "common_header.h"

#define INFO_COLOR "\e[0;32m" // Verde
#define RESET_COLOR "\e[0m" // Bianco
#define WARN_COLOR "\e[0;33m" // Giallo
#define ERROR_COLOR "\e[0;31m" // Rosso

void LOG_INFO(char* msg);
void LOG_WARN(char* msg);
void LOG_ERROR(char* msg);
void LOG_PERROR(char* msg);
void set_LOG_INFO();
void set_LOG_WARN();
void set_LOG_ERROR();
void reset_LOG();

#endif
