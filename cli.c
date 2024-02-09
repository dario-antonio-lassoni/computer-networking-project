/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libs/logger.h"
#include "libs/device_utils.h"
#include "libs/common_utils.h"
#include "libs/database.h"

#define REQUEST_SIZE 512
#define BUFFER_SIZE 1024

void print_menu() {
//	printf("\e[1;1H\e[2J");
	printf("Comandi disponibili:\n");
	printf("\tfind --> ricerca la disponibilità per una prenotazione\n");
	printf("\tbook --> invia una prenotazione\n");
	printf("\tesc  --> termina il client\n");
}

int check_cmd_book(char* book) {
	return -1;
}

int main(int argc, char* argv[]) {
	int ret, sd;
	struct sockaddr_in srv_addr;
	struct client_device cli_dev;
	char* buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	char* request = (char*)malloc(sizeof(char) * REQUEST_SIZE);
	struct cmd_struct* command;
	struct table *table_list, *temp_table;

	if(argc != 2) {
		printf("Argomenti errati. Specificare correttamente il comando come segue: ./cli <porta>\n");
		exit(0);
	}
		
	/* Creazione socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo del server */
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(4242);
	inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
	
	/* Creazione della struttura client_device per la fase di riconoscimento */
	cli_dev.port = atoi(argv[1]);
	cli_dev.type = CL;
	cli_dev.next = NULL;
	
	/* Connessione */
	ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore in fase di connessione: ");
		exit(1);
	}
	
	/* Invio della richiesta di riconoscimento verso il server */
	write_text_to_buffer((void*)&request, "RECOGNIZE_ME");
	ret = send(sd, request, strlen(request) + 1, 0);
	//ret = send_data(sd, request);
		
	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore in fase di riconoscimento (step 1): \n");
		exit(1);
	}

	/* Attesa da parte del server dell'acquisizione della richiesta */
	
	//ret = receive_data(sd, (void*)&buffer);
	
	ret = recv(sd, buffer, BUFFER_SIZE, 0);

	printf("Aspetto che ritorni la RECOGNIZE_ME dal server\n");
	fflush(stdout);
	
	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore in fase di riconoscimento (step 2): \n");
		exit(1);
	}
	
	printf("buffer: %s\n", buffer);
	fflush(stdout);

	if(strcmp("START_RECOGNIZE", buffer)) {
		set_LOG_ERROR();
		printf("Errore in fase di riconoscimento: segnale di inizio riconoscimento non ricevuto dal server");
		exit(1);
	}

	/* Invio la tipologia del client al server per la fase di riconoscimento  */
	sprintf(request, "%d %d", cli_dev.port, cli_dev.type);
	ret = send(sd, request, strlen(request) + 1, 0);
	//ret = send_data(sd, request);
	LOG_INFO("Invio tipologia del client al server");
	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore in fase di riconoscimento (step 3): \n");
		exit(1);
	}

	/* In attesa di ACK da parte del server per conferma di riconoscimento avvenuto */
	//ret = receive_data(sd, (void*)&buffer);
	ret = recv(sd, (void*)buffer, BUFFER_SIZE, 0);

	if(strcmp("END_RECOGNIZE", buffer)) {
		set_LOG_ERROR();
		printf("Errore in fase di riconoscimento: segnale di fine riconoscimento non ricevuto dal server");
		exit(1);
	}

	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore in fase di ricezione dell'ACK per avvenuto riconoscimento: \n");
		exit(1);
	}

	for(;;) {
		print_menu();
		fgets(request, REQUEST_SIZE, stdin);
		
		if(strcmp(request, "esc\n") == 0) {
			printf("Chiusura client...\n");
			exit(0);
		} else if(strncmp(request, "find", 4) == 0) { /* Controlla che la stringa inizi per "find" */
			
			command = create_cmd_struct_find(request);
			
			if(command == NULL) {
				printf("Sintassi del comando find errata.\nfind <cognome persone data ora>\ndove 'data' in formato GG-MM-AA e 'ora' in formato HH\n");

				continue; // Skip dell'invio, sintassi del comando errata	
			
			} else {
				printf("cmd: %s %s %d %d-%d-%d %d\n", (char*)command->cmd, (char*)command->args[0], 
				*((int*)command->args[1]), *((int*)command->args[2]), *((int*)command->args[3]), *((int*)command->args[4]), *((int*)command->args[5]));
				
				/* Invio del comando find */
				ret = send(sd, request, strlen(request) + 1, 0);
				//ret = send_data(sd, request);
				if(ret < 0) {
					perror("Errore in fase di invio comando: ");
					exit(1);
				}

				table_list = NULL;				
				
				for(;;) {
					/* Attesa della response con i tavoli prenotabili */
					
					//ret = receive_data(sd, (void*)&buffer);
					ret = recv(sd, (void*)buffer, BUFFER_SIZE, 0);

					printf("buffer: %s\n", buffer);	
					fflush(stdout);

					if(strcmp(buffer, "END_MSG\0") == 0) {
						printf("FINE!");
						fflush(stdout);
						break;
					}

					/* Aggiunta del tavolo nella table_list */
					temp_table = (struct table*)malloc(sizeof(struct table));
					temp_table->next = NULL;
					ret = sscanf(buffer, "%s %s %s", &temp_table->table, &temp_table->room, &temp_table->position);
					add_to_table_list(&table_list, temp_table);
						
					printf("ho aggiunto un tavolo\n");
					fflush(stdout);
					// INVIO DELLA BOOK ....
				}
				printf("HO FINITO DI RICEVERE I TAVOLI\n");
				fflush(stdout);
				//print_table_list(table_list);
				
			}

		} else if(strncmp(request, "book", 4) == 0) { /* Controlla che la stringa inizi per "book" */
			ret = check_cmd_book(request);
			printf("Errore: la prenotazione non può essere completata. Usare prima il comando 'find' e solo dopo il comando 'book'.\n");
		}
	}
}
