/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "logger.h"
#include <time.h>

time_t rawtime;
struct tm* timeinfo;

/* Funzioni che permettono di stampare a schermo una stringa di log di livello INFO, WARN o ERROR */

void LOG_INFO(char* msg) {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", INFO_COLOR);
	printf(" INFO  ");
	printf("%s", RESET_COLOR);
	printf("--- %s\n",  msg); 
	fflush(stdout);
}

void LOG_WARN(char* msg) {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", WARN_COLOR);
	printf(" WARN  ");
	printf("%s", RESET_COLOR);
	printf("--- %s\n",  msg); 
	fflush(stdout);
}

void LOG_ERROR(char* msg) {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", ERROR_COLOR);
	printf(" ERROR ");
	printf("%s", RESET_COLOR);
	printf("--- %s\n",  msg); 
	fflush(stdout);
}

/* Setta lo stile della printf come se fosse un log stampato con le funzioni LOG_[INFO|WARN|ERROR](char* msg) */

void set_LOG_INFO() {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", INFO_COLOR);
	printf(" INFO  ");
	printf("%s", RESET_COLOR);
	printf("--- "); 
	fflush(stdout);
}

void set_LOG_WARN() {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", WARN_COLOR);
	printf(" WARN  ");
	printf("%s", RESET_COLOR);
	printf("--- "); 
	fflush(stdout);
}

void set_LOG_ERROR() {
	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 
	printf("%s", ERROR_COLOR);
	printf(" ERROR ");
	printf("%s", RESET_COLOR);
	printf("--- "); 
	fflush(stdout);
}
