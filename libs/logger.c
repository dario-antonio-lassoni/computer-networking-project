/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "logger.h"
#include <time.h>
//IMPLEMENTARE TIMESTAMP SU LOG

#define INFO_COLOR "\e[0;32m" // Verde
#define RESET_COLOR "\e[0m" // Bianco
#define WARN_COLOR "\e[0;33m" // Giallo
#define ERROR_COLOR "\e[0;31m" // Rosso

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

//void LOG_PERROR(char* msg) { /* Personalizzazione della LOG_ERROR per la perror() */ 
/*	size_t msg_len = strlen(msg);
	char* error_msg = (char*)malloc((msg_len + 20) * sizeof(char)); // [INFO][TIMESTAMP]_\0
	fflush(stdout); // Forzo il cambio di colore nello stdout prima di stampare con la perror()

	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	printf("%d-%d-%d %d:%d:%d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); 

	printf("%s", ERROR_COLOR);
	strcat(error_msg, " ERROR ");
	strcat(error_msg, msg);
	perror(error_msg);
	printf("%s", RESET_COLOR);
}
*/

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

/* Resetta lo stile del log 
void reset_LOG() {
	printf("%s", RESET_COLOR);
}
*/
