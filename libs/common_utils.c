/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include "common_utils.h"
#include "common_header.h"
#include "database.h"
#include <time.h>


int check_cmd_find(struct cmd_struct* command, int ret) {
	
	if(command == NULL)
		return -1;

		/* Verifica primo argomento (cognome) */
	if(	(command->args[0] == NULL) ||
		
		/* Verifico secondo argomento (numero di persone) */
		(command->args[1] == NULL || *((int*)command->args[1]) < 1) ||
		
		/* Verifica gli argomenti rimanenti (data e ora) */
		(command->args[2] == NULL || *((int*)command->args[2]) < 1 || *((int*)command->args[2]) > 31) ||
		(command->args[3] == NULL || *((int*)command->args[3]) < 1 || *((int*)command->args[3]) > 12) ||
		(command->args[4] == NULL || *((int*)command->args[4]) > 99) ||
		(command->args[5] == NULL || *((int*)command->args[5]) < 1 || *((int*)command->args[5]) > 23)) 
	{
		return -1;
	}

	return ret ;
}

struct cmd_struct* create_cmd_struct_find(char* input) {
	
	int i;		

	/* Allocazione memoria per la cmd_struct */
	
	struct cmd_struct* command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->cmd = (char*)malloc(sizeof(char) * 4); 
	command->args[0] = (char*)malloc(sizeof(char) * 100);

	for(i = 1; i < 6; i++) 
		command->args[i] = (int*)malloc(sizeof(int));
	
	/* Popola la struct */
	
	strcpy(command->cmd, "find");
	
	int ret = sscanf(input, "find %s %d %d-%d-%d %d",  (char*)command->args[0], (int*)command->args[1] , (int*)command->args[2], (int*)command->args[3], 
				(int*)command->args[4], (int*)command->args[5]);
	
	/* Verifica che i parametri di ricerca inseriti siano corretti */

	ret = check_cmd_find(command, ret);
	
	if(ret < 6) {
		free(command);	
		return NULL;
	}

	return command;

}

int check_cmd_book(struct cmd_struct* command, struct table* list, int ret) {

	int num;

	if(list == NULL)
		return -1;

	num = count_elements_in_table_list(list); 
	
	if(command->args[0] == NULL || *((int*)command->args[0]) < 1 || *((int*)command->args[0]) > num) {
		return -1;
	}

	return ret;
}

struct cmd_struct* create_cmd_struct_book(char* input, struct table* list) {
	
	int i;

	/* Allocazione memoria per la cmd_struct */
	
	struct cmd_struct* command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->cmd = (char*)malloc(sizeof(char) * 4);
	command->args[0] = (int*)malloc(sizeof(int));
	
	for(i = 1; i < 6; i++)
		command->args[i] = NULL;

	/* Popola la struct */

	strcpy(command->cmd, "book");

	int ret = sscanf(input, "book %d", (int*)command->args[0]);

	/* Verifica che l'opzione inserita sia tra quelle disponibili */
	
	ret = check_cmd_book(command, list, ret);

	if(ret < 1) {
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
	if(ret < 0)
		return ret;

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
	
	/* Dealloco e rialloco il buffer così da non essere 
	 * dipendente da una dimensione massima del buffer */

	if(*buf != NULL) 
		free(*buf);
	*buf = (char*)malloc(sizeof(char) * len);

	/* Ricezione dei dati */
	ret = recv(sd, (void*)(*buf), len + 1, 0);

	return ret;

}

