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
#include <ctype.h>
#include "libs/logger.h"
#include "libs/device_utils.h"
#include "libs/common_utils.h"
#include "libs/database.h"

#define INPUT_SIZE 512

void print_command_list() {	
	printf("\e[1;1H\e[2J");
	printf("Comandi disponibili:\n");
	printf("\tfind --> ricerca la disponibilità per una prenotazione\n");
	printf("\tbook --> invia una prenotazione\n");
	printf("\tesc  --> termina il client\n");
}

int main(int argc, char* argv[]) {

	int ret, sd, i;
	struct sockaddr_in srv_addr;
	struct client_device cli_dev;
	char *input, *buffer;
	struct cmd_struct* command;
	struct table *table_list, *temp_table;

	input = (char*)malloc(sizeof(char) * INPUT_SIZE);
	buffer = NULL;
	command = NULL;
	table_list = NULL;
	temp_table = NULL;

	if(!check_port(argc, argv)) {
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
		perror("Errore in fase di connessione: ");
		exit(1);
	}
	
	/* Invio della richiesta di riconoscimento verso il server */
	write_text_to_buffer((void*)&buffer, "RECOGNIZE_ME");
	ret = send_data(sd, buffer);
		
	if(ret < 0) {
		perror("Errore in fase di riconoscimento (step 1): \n");
		exit(1);
	}

	/* Attesa da parte del server dell'acquisizione della richiesta */
	ret = receive_data(sd, (void*)&buffer);
	
	if(ret < 0) {
		perror("Errore in fase di riconoscimento (step 2): \n");
		exit(1);
	}
	
	if(strcmp("START_RECOGNIZE", buffer)) {
		printf("Errore in fase di riconoscimento: segnale di inizio riconoscimento non ricevuto dal server");
		exit(1);
	}

	/* Invio la tipologia del client al server per la fase di riconoscimento  */
	sprintf(buffer, "%d %d", cli_dev.port, cli_dev.type);
	ret = send_data(sd, buffer);
	
	if(ret < 0) {
		perror("Errore in fase di riconoscimento (step 3): \n");
		exit(1);
	}

	/* In attesa di ACK da parte del server per conferma di riconoscimento avvenuto */
	ret = receive_data(sd, (void*)&buffer);

	if(strcmp("END_RECOGNIZE", buffer)) {
		printf("Errore in fase di riconoscimento: segnale di fine riconoscimento non ricevuto dal server");
		exit(1);
	}

	if(ret < 0) {
		perror("Errore in fase di ricezione dell'ACK per avvenuto riconoscimento: \n");
		exit(1);
	}

	free_mem((void*)&buffer);
	
	print_command_list();

	for(;;) {

		fgets(input, INPUT_SIZE, stdin);
		
		if(strcmp(input, "esc\n") == 0) {
			
			free_mem((void*)&buffer);
			free_mem((void*)&input);

			temp_table = table_list;
			
			while(table_list != NULL) {
				temp_table = temp_table->next;
				free_mem((void*)&table_list);
				table_list = temp_table;
			}

			temp_table = NULL;

			printf("Chiusura client...\n");
			exit(0);

		} else if(strncmp(input, "find", 4) == 0) { /* Controlla che la stringa inizi per "find" */
			
			command = NULL;
			command = create_cmd_struct_find(input);

			if(command == NULL) {
				printf("I parametri di ricerca inseriti non sono corretti o la sintassi del comando 'find' è errata.\n");
				printf("Sintassi: find <cognome> <persone> <data> <ora> dove:\n\t");
				printf("'cognome' è formato da max 20 caratteri\n\t");
				printf("'persone' è formato da 2 cifre\n\t");
				printf("'data' è in formato GG-MM-AA\n\t");
				printf("'ora' in formato HH\n");
				continue; // Skip dell'invio, sintassi del comando errata	
			}
			
			free_mem((void*)&command->cmd);

			for(i = 0; i < 6; i++) {
				free_mem((void*)&command->args[i]);
			}

			free_mem((void*)&command);

			/* Copio il contenuto di 'input' in 'buffer' per poter sfruttare la send_data
			 * ed evitare di deallocare e riallocare con un'altra dimensione 'input'
			 */
				
			write_text_to_buffer((void*)&buffer, input);
				
			/* Invio del comando find */
			ret = send_data(sd, buffer);
				
			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1);
			}

			table_list = NULL;			
			temp_table = NULL;

			for(;;) {

				/* Attesa della response con i tavoli prenotabili */	
				ret = receive_data(sd, (void*)&buffer);

				if(strcmp(buffer, "NO_TABLE\0") == 0) {
					table_list = NULL;
					break;
				}

				if(strcmp(buffer, "END_MSG\0") == 0) {
					break;
				}

				/* Aggiunta del tavolo nella table_list */
				temp_table = (struct table*)malloc(sizeof(struct table));
				temp_table->next = NULL;
				ret = sscanf(buffer, "%s %s %s", &temp_table->table[0], &temp_table->room[0], &temp_table->position[0]);
				add_to_table_list(&table_list, temp_table); // Questa add prende il puntatore di temp_table
			}

			free_mem((void*)&buffer);

			if(table_list == NULL) {
				printf("Non è disponibile nessun tavolo con i parametri selezionati\n");
				continue;
			} else {
				print_bookable_tables(table_list);
			}

			/* Da qui in poi, a seguito della find, i comandi possibili sono 'book' e 'esc' */
			
			fgets(input, INPUT_SIZE, stdin);
			
			if(strcmp(input, "esc\n") == 0) {
				
				free_mem((void*)&buffer);
				free_mem((void*)&input);

				temp_table = table_list;
			
				while(table_list != NULL) {
					temp_table = temp_table->next;
					free_mem((void*)&table_list);
					table_list = temp_table;
				}

				temp_table = NULL;

				printf("Chiusura client...\n");
				exit(0);

			} else if(strncmp(input, "book", 4) == 0) { // Controlla che la stringa inizi per 'book'
				
				command = create_cmd_struct_book(input, table_list);
				
				if(command == NULL) {
					printf("Sintassi del comando book errata.\nbook <opz> dove 'opz' è una delle opzioni tra i tavoli disponibili\n");
					continue; // Skip dell'invio, sintassi del comando errata	
				}
			
				free_mem((void*)&command->cmd);
				free_mem((void*)&command->args[0]);	
				free_mem((void*)&command);

				/* Copio il contenuto di 'input' in 'buffer' per poter sfruttare la send_data
			 	 * ed evitare di deallocare e riallocare con un'altra dimensione 'input'
				 */
				
				write_text_to_buffer((void*)&buffer, input);
				
				/* Invio del comando book */
				ret = send_data(sd, buffer);
				
				if(ret < 0) {
					perror("Errore in fase di invio comando: ");
					exit(1);
				}
	
				/* Attesa della response con la conferma prenotazione */
				ret = receive_data(sd, (void*)&buffer);

				if(strcmp(buffer, "BOOK_KO\0") == 0) {
					printf("Si è verificato un errore durante la ricezione della conferma della prenotazione\n");
				} else {
					printf("La prenotazione è andata a buon fine.\n");
					printf("%s\n", buffer); // Stampa del codice, tavolo e sala associati alla prenotazione (inviati dal server)
				}	
				
				free_mem((void*)&buffer);

			} else {
				printf("Dopo la find gli unici comandi consentiti sono 'book' o 'esc'! Ripetere la sequenza di comandi correttamente\n");
			}
			
		} else if(strncmp(input, "book", 4) == 0) { // Controlla che la stringa inizi per 'book'	
			printf("La prenotazione non può essere completata. Usare prima il comando 'find' e solo dopo il comando 'book'.\n");
		} else {	
			printf("Comando errato. Utilizzare solo i comandi consentiti\n");
		}
	}
}
