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

int check_port(int argc, char* argv[]) {

	int port;

	if(argc != 2 || sscanf(argv[1], "%d", &port) != 1) {
		if(port == 0) {
			return 0;
		}
	}

	return 1;
}

int check_cmd_find(struct cmd_struct* command, int ret) {

  	time_t now;
	time_t input_date;
	struct tm input_date_tm = {0};

	if(command == NULL)
		return -1;

	/* Check che il timestamp in input sia successivo a quello attule */
    	time(&now);

    	input_date_tm.tm_mday = *((int*)command->args[2]); // Giorno	
	input_date_tm.tm_mon = *((int*)command->args[3]) - 1; // Mese
	input_date_tm.tm_year = *((int*)command->args[4]) + 100; // Anno 
	input_date_tm.tm_hour = *((int*)command->args[5]); // Ora

    	input_date = mktime(&input_date_tm);

	if(command->args[0] == NULL || command->args[1] == NULL ||
	   command->args[2] == NULL || command->args[3] == NULL ||
	   command->args[4] == NULL || command->args[5] == NULL) {
		return -1;
	}

	/* Check che ha senso fare solo a seguito della chiamata a mktime */
	if(input_date_tm.tm_isdst == 1) { // Annullo gli effetti dell'ora legale
		input_date_tm.tm_hour -= 1;
		/* Se la precedente mktime ha capito che nell'orario selezionato vale l'ora legale, 
		 * ne annullo gli effetti prima di fare i successivi controlli */
		mktime(&input_date_tm);
	}

	
	/* Verifica primo argomento (cognome) */
	if((strlen(command->args[0]) > 20) ||
		
		/* Verifico secondo argomento (numero di persone) */
		(*((int*)command->args[1]) < 1 || *((int*)command->args[1]) > 99) ||
			
		/* Verifica gli argomenti rimanenti (data e ora) */
		(difftime(input_date, now) < 0) || // Verifica se il timestamp in input è precedente a quello attuale

		/* Validazione data e ora.
		 * Check se a seguito della valorizzazione della struct tm i valori rimangano coerenti, 
		 * altrimenti la data non è valida */
	       	(input_date_tm.tm_mday != *((int*)command->args[2])) ||
		(input_date_tm.tm_mon != (*((int*)command->args[3]) - 1)) ||
		(input_date_tm.tm_year != (*((int*)command->args[4]) + 100)) ||
		(*((int*)command->args[5]) < 0 || *((int*)command->args[5]) > 23)) 
	{
		return -1;
	}

	return ret; // Ritorno la ret passata come parametro se passa il check
}

struct cmd_struct* create_cmd_struct_find(char* input) {
	
	int i;		

	/* Allocazione memoria per la cmd_struct */
	
	struct cmd_struct* command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->cmd = (char*)malloc(sizeof(char) * 5); 
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
		free_mem((void*)&command->cmd);
		for(i = 0; i < 6; i++)
			free_mem((void*)&command->args[i]);
		free_mem((void*)&command);	
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
	command->cmd = (char*)malloc(sizeof(char) * 5);
	command->args[0] = (int*)malloc(sizeof(int));
	
	for(i = 1; i < 6; i++)
		command->args[i] = NULL;

	/* Popola la struct */

	strcpy(command->cmd, "book");

	int ret = sscanf(input, "book %d", (int*)command->args[0]);

	/* Verifica che l'opzione inserita sia tra quelle disponibili */
	
	ret = check_cmd_book(command, list, ret);

	if(ret < 1) {
		free_mem((void*)&command->cmd);
		free_mem((void*)&command->args[0]);
		free_mem((void*)&command);
		return NULL;
	}

	return command;
}

void write_text_to_buffer(void** buf, char* text) {

	int len = strlen(text);
	*buf = (char*)malloc(sizeof(char) * (len + 1));
	memset(*buf, 0, len);
	strcpy(*buf, text);

}

int send_data(int sd, void* buf) {

	uint16_t lmsg;
	int ret, len;

	len = strlen(buf) + 1;

	/* Conversione in formato network */
	lmsg = htons(len);
	
	/* Invio la dimensione dei dati che invierò con la seconda send */
	ret = send(sd, (void*)&lmsg, sizeof(uint16_t), 0);
	if(ret < 0)
		return ret;

	/* Invio dei dati */
	
	ret = send(sd, buf, len, 0);

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

	free_mem((void*)&(*buf));

	*buf = (char*)malloc(sizeof(char) * len);

	/* Ricezione dei dati */
	ret = recv(sd, *buf, len, 0);

	return ret;

}

/* Wrapper della free, si limita a fare dei controlli in più */
void free_mem(void** ptr) {

	/* Evito di fare la free se la memoria è già stata liberata */	
	if(*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
	}
}
