/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "common_utils.h"
#include "common_header.h"

struct cmd_struct* create_cmd_struct_find(char* request) {
	
	int i;		

	/* Allocazione memoria per la cmd_struct */
	struct cmd_struct* command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->cmd = (char*)malloc(sizeof(char) * 4); 
	command->args[0] = (char*)malloc(sizeof(char) * 100);

	for(i = 1; i < 6; i++) 
		command->args[i] = (int*)malloc(sizeof(int));
	
	/* Popola la struct */
	strcpy(command->cmd, "find");
	int ret = sscanf(request, "find %s %d %d-%d-%d %d",  (char*)command->args[0], (int*)command->args[1] , (int*)command->args[2], (int*)command->args[3], 
				(int*)command->args[4], (int*)command->args[5]);
	
	printf("campi della find trovati %d\n", ret);

	if(ret < 6) {
		free(command);	
		return NULL;
	}

	return command;
}

void write_text_to_buffer(void** buf, char* text) {
	int len = strlen(text);
	*buf = (char*)malloc(sizeof(char) * len);
	memset(*buf, 0, len);
	strcpy(*buf, text);
}

int send_data(int sd, void* buf) {
	uint16_t lmsg;
	int ret, len;

	len = strlen(buf);
	/* Conversione in formato network */
	lmsg = htons(len);
	
	/* Invio la dimensione dei dati che invierò con la seconda send */
	ret = send(sd, (void*)&lmsg, sizeof(uint16_t), 0);
	
	/* Invio dei dati */
	ret = send(sd, (void*)buf, len + 1, 0);

	return ret;
}

int receive_data(int sd, void** buf) {
	
	uint16_t lmsg;
	int ret, len;

	ret = recv(sd, (void*)&lmsg, sizeof(uint16_t), 0);

	/* Riconversione in formato host */
	len = ntohs(lmsg);
	printf("receive_data len letto: %d\n", len);
	
	/* Dealloco e rialloco il buffer così da non essere 
	 * dipendente da una dimensione massima del buffer */

	if(*buf != NULL) 
		free(*buf);
	*buf = (char*)malloc(sizeof(char) * len);

	/* Ricezione dei dati */
	ret = recv(sd, (void*)(*buf), len + 1, 0);
	printf("receive_data buf ha: %s\n", (char*)*buf);
	fflush(stdout);	
	return ret;
}

