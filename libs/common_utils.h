/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef COMMON_UTILS_H 
#define COMMON_UTILS_H

#include "database.h"
#include "common_header.h"

int check_port(int argc, char* argv[]);
struct cmd_struct* create_cmd_struct_find(char* input);
struct cmd_struct* create_cmd_struct_book(char* input, struct table* list);
struct cmd_struct* create_cmd_struct_login(char* input);
struct cmd_struct* create_cmd_struct_comanda(char* input);
/* Scrive il testo indicato sul buffer, riallocandolo con la lunghezza corretta */
void write_text_to_buffer(void** buf, char* text);
/* Send indipendente dalla dimensione del messaggio e che utilizza il Text Protocol */
int send_data(int sd, void* buf);
/* Receive indipendente dalla dimensione del messaggio e che utilizza il Text Protocol */
int receive_data(int sd, void** buf);
/* Wrapper della free, si limita a fare dei controlli in più */
void free_mem(void** ptr);

#endif
