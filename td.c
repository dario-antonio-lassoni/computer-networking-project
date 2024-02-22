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

void print_help() {
	printf(" menu\n\tmostra il menu dei piatti\n");
	printf(" comanda {<piatto_1-quantità_1>...<piatto_n-quantità_n>}\n\tinvia una comanda con i piatti e la quantità indicata\n");
	printf(" conto\n\tchiede il conto\n");
}

void print_start_menu() {	
	//printf("\e[1;1H\e[2J");
	printf("***************************** BENVENUTO *****************************\n");
	printf("Digita un comando:\n\n");
	printf("1) help\t\t--> mostra i dettagli dei comandi\n");
	printf("2) menu\t\t--> mostra il menu dei piatti\n");
	printf("3) comanda\t--> invia una comanda\n");
	printf("4) conto\t--> chiede il conto\n");
}

int main(int argc, char* argv[]) {

	int ret, sd;
	struct sockaddr_in srv_addr;
	struct client_device cli_dev;
	char *input, *buffer;
	struct cmd_struct* command;
	struct table *table_list, *temp_table;
	struct dish *dish_list, *temp_dish;
	
	input = (char*)malloc(sizeof(char) * INPUT_SIZE);
	buffer = NULL;
	command = NULL;
	table_list = NULL;
	temp_table = NULL;
	dish_list = NULL;
	temp_dish = NULL;

	if(!check_port(argc, argv)) {
		printf("Argomenti errati. Specificare correttamente il comando come segue: ./td <porta>\n");
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
	cli_dev.type = TD;
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

	
	for(;;) {
		printf("Inserire il codice di prenotazione: ");
		fflush(stdout);
		
		buffer = (char*)malloc(sizeof(char) * CMD_LOGIN_LEN); // login + BOOKING_CODE_LEN
		strcpy(buffer, "login ");

		/* Prelevo in input il codice di prenotazione */
		fgets(input, INPUT_SIZE, stdin);
		
		/* Concatenzione del comando login con il codice di prenotazione da inviare al server */
		strcat(buffer, input);

		/* Invio del comando login <BOOKING_CODE> */
		ret = send_data(sd, buffer);
				
		if(ret < 0) {
			perror("Errore in fase di invio codice di prenotazione");
			exit(1);
		}
		
		/* Attesa della response da parte del server per conferma verifica */
		ret = receive_data(sd, (void*)&buffer);

		if(ret < 0) {
			perror("Errore in fase di ricezione verifica codice prenotazione");
			exit(1);
		}

		if(strcmp(buffer, "BOOKING_CODE_IS_VALID\0") == 0) {
			printf("Codice prenotazione corretto\n");
			break;
		} else if(strcmp(buffer, "BOOKING_CODE_IS_NOT_VALID\0") == 0) {
			printf("Codice di prenotazione non valido.\n");
		} else {
			printf("Errore: è stato ricevuto un messaggio di validazione errato dal server\n");
		}
	}

	print_start_menu();

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

		} else if(strcmp(input, "help\n") == 0) {
			/* Stampa i dettagli dei comandi */
			print_help();		
			continue;
		} else if(strcmp(input, "menu\n") == 0) {
			/* Recupera i piatti del menu dal server */

			write_text_to_buffer((void*)&buffer, input);
				
			/* Invio del comando menu */
			ret = send_data(sd, buffer);
				
			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1);
			}

			dish_list = NULL;			
			temp_dish = NULL;

			for(;;) {

				/* Attesa della response con i piatti del menu */	
				ret = receive_data(sd, (void*)&buffer);

				if(strcmp(buffer, "END_MSG\0") == 0)
					break;
				
				/* Aggiunta del tavolo nella table_list */
				temp_dish = (struct dish*)malloc(sizeof(struct dish));
				temp_dish->next = NULL;
				ret = sscanf(buffer, "%d %s - %[^\n]", &temp_dish->price, &temp_dish->identifier[0], &temp_dish->description[0]);
				add_to_dish_list(&dish_list, temp_dish);
			}

			free_mem((void*)&buffer);

			print_menu_dishes(dish_list);
			
		} else if(strncmp(input, "comanda", 7) == 0) { // Controlla che la stringa inizi per 'comanda'
			
			/* Comando 'comanda' con i piatti in una lista puntata di dish su arg0 di command */	
			
			command = create_cmd_struct_comanda(input);
				
			if(command == NULL) {
				printf("Sintassi del comando 'comanda' è errata.\n");
				printf("Sintassi: comanda {<piatto_1-quantità_1> <piatto_2-quantità_2> ... <piatto_n-quantità_n>}\n\t");
				continue; // Skip dell'invio, sintassi del comando errata	
			}

			/* Invio della comanda */

			ret = send_data(sd, buffer);

			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1);
			}



		} else {	
			printf("Comando errato. Utilizzare solo i comandi consentiti\n");
		}
	}
}
