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

struct cmd_struct* create_cmd_struct_login(char* input) {
	
	int i;

	/* Allocazione memoria per la cmd_struct */
	
	struct cmd_struct* command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->cmd = (char*)malloc(sizeof(char) * 6); //Usare CONST DA COMMON_HEADER
	command->args[0] = (char*)malloc(sizeof(char) * 10);
	
	for(i = 1; i < 6; i++)
		command->args[i] = NULL;

	/* Popola la struct */

	strcpy(command->cmd, "login");

	int ret = sscanf(input, "login %s", (char*)command->args[0]);

	if(ret != 1) {
		free_mem((void*)&command->cmd);
		free_mem((void*)&command->args[0]);
		free_mem((void*)&command);
		return NULL;
	}

	return command;
}

struct cmd_struct* create_cmd_struct_comanda(char* input, char* table, int sd_td) {
	
	int i, input_is_valid;
	char* token, *buffer;
	struct dish* current_dish;
	struct comanda* comanda;
	struct cmd_struct* command;

	/* Check che la comanda contenga almeno un piatto */
	input_is_valid = 0; // Se input_is_valid rimane false allora vorrà dire che non c'è almeno un piatto nella comanda

	/* Creo una copia temporanea dell'input per poterla utilizzare con la strtok */
	write_text_to_buffer((void*)&buffer, input);

	/* Allocazione memoria per la cmd_struct */
	command = (struct cmd_struct*)malloc(sizeof(struct cmd_struct));
	command->args[0] = (struct comanda*)malloc(sizeof(struct comanda));
	comanda = (struct comanda*)command->args[0];
	current_dish = (struct dish*)malloc(sizeof(struct dish));
	((struct comanda*)command->args[0])->dish_list = current_dish;

	for(i = 1; i < 6; i++)
		command->args[i] = NULL;

	/* Popola la struct */
	write_text_to_buffer((void*)&command->cmd, "comanda");
	
	if(table != NULL)
		strcpy(comanda->table, table);

	comanda->sd = sd_td; // Socket Descriptor del Table Device (lato server) 
	
	
	//((struct comanda*)command->args[0])->next = NULL;	
	comanda->next = NULL;

	token = strtok(buffer, " ");
	token = strtok(NULL, " "); // Indico alla funzione strtok di saltare direttamente al primo token (" ") 
				   // così da poter leggere la prima coppia nome_piatto-quantità

	while (token != NULL && sscanf(token, "%2s-%d", current_dish->identifier, &current_dish->quantity) == 2) {
		
		if(input_is_valid == 0) 
			input_is_valid = 1;

		token = strtok(NULL, " "); // Avanzo al prossimo token
					   
		if(token != NULL) {
			current_dish->next = (struct dish*)malloc(sizeof(struct dish));
			current_dish = current_dish->next;
		} else {
			current_dish->next = NULL;
		}
	}

	/* Se l'input non è valido allora pulisco la memoria */

	if(!input_is_valid) {
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
