/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef COMMON_UTILS_H 
#define COMMON_UTILS_H

struct cmd_struct {
	char* cmd;
	void* args[6];
};

struct cmd_struct* create_cmd_struct_find(char* request);

void write_text_to_buffer(void** buf, char* text);
/* Send indipendente dalla dimensione del messaggio e che utilizza il Text Protocol */
int send_data(int sd, void* buf);
/* Receive indipendente dalla dimensione del messaggio e che utilizza il Text Protocol */
int receive_data(int sd, void** buf);

#endif
