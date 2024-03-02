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

void print_start_menu() {	
	//printf("\e[1;1H\e[2J");
	printf("Comandi disponibili:\n");
	printf("take\t\t--> accetta la comanda nello stato di attesa da più tempo\n");
	printf("show\t\t--> mostra le comande accettate (in preparazione)\n");
	printf("ready <comanda-tavolo>\t--> imposta allo stato 'in servizio' la comanda del tavolo indicato\n");
}

int main(int argc, char* argv[]) {

	int ret, sd;
	struct sockaddr_in srv_addr;
	struct client_device cli_dev;
	char *input, *buffer;
	struct cmd_struct* command;
	struct table *table_list, *temp_table;
	struct dish *dish_list, *temp_dish;
	struct comanda *in_preparation, *in_service, *curr_order;

	input = (char*)malloc(sizeof(char) * INPUT_SIZE);
	buffer = NULL;
	command = NULL;
	table_list = NULL;
	temp_table = NULL;
	dish_list = NULL;
	temp_dish = NULL;


	/* Inizializzazione della lista di comande gestite dal Kitchen Device */
	in_preparation = NULL;
	in_service = NULL;
	curr_order = NULL;

	/* Check della porta */
	if(!check_port(argc, argv)) {
		printf("Argomenti errati. Specificare correttamente il comando come segue: ./kd <porta>\n");
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
	cli_dev.type = KD;
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

	print_start_menu();

	for(;;) {
		
		do {
			fgets(input, INPUT_SIZE, stdin);
		} while(strcmp(input, "\n") == 0); // Non sono ammessi input vuoti

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

		} else if(strcmp(input, "take\n") == 0) {

			/* Recupero della comanda meno recente ancora in stata 'in attesa' */

			write_text_to_buffer((void*)&buffer, input);
				
			ret = send_data(sd, buffer);
				
			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1);
			}

			ret = receive_data(sd, (void*)&buffer);

			if(ret < 0) {
				perror("Errore in fase di ricezione comanda");
				exit(1);
			}

			if(strcmp(buffer, "NO_ORDERS\0") == 0) {
				printf("Nessuna nuova comanda trovata\n");
				free_mem((void*)&buffer);
				continue;
			}
			
			curr_order = (struct comanda*)malloc(sizeof(struct comanda));
			curr_order->next = NULL;
			sscanf(buffer, "order %[^-]-%[^\n]", curr_order->table, curr_order->com_count);
			curr_order->dish_list = NULL;

			/* Recupero di tutti i piatti della comanda */

			for(;;) {

				/* Attesa della response con i piatti della comanda */
				ret = receive_data(sd, (void*)&buffer);

				if(strcmp(buffer, "END_MSG\0") == 0)
					break;

				/* Aggiunta del piatto nella dish_list*/
				temp_dish = (struct dish*)malloc(sizeof(struct dish));
				temp_dish->next = NULL;
				ret = sscanf(buffer, "dish %[^-]-%d-%[^\n]", &temp_dish->identifier[0], &temp_dish->quantity, &temp_dish->description[0]) ;
				add_to_dish_list(&curr_order->dish_list, temp_dish);
			}
			
			/* Inserimento della comanda nella lista delle comande in preparazione */
		
			add_to_orders_list(&in_preparation, curr_order);

			free_mem((void*)&buffer);
			
			print_taken_order(curr_order);
			
		} else if(strncmp(input, "comanda", 7) == 0) { // Controlla che la stringa inizi per 'comanda'
			
			command = create_cmd_struct_comanda(input, NULL, -1);
			
			if(command == NULL) {
				printf("Sintassi del comando 'comanda' è errata.\n");
				printf("Sintassi: comanda {<piatto_1-quantità_1> <piatto_2-quantità_2> ... <piatto_n-quantità_n>}\n");
				continue; // Skip dell'invio, sintassi del comando errata	
			}

			/* Invio della comanda */
			write_text_to_buffer((void*)&buffer, input);
			ret = send_data(sd, buffer);

			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1);
			}

			/* Attesa response avvenuta ricezione comanda */
			
			ret = receive_data(sd, (void*)&buffer);

			if(ret < 0) {
				perror("Errore durante l'attesa di ricezione\n");
				exit(1);
			}

			if(strcmp("ORDER_RECEIVED", buffer) == 0)
				printf("COMANDA RICEVUTA\n");
			else if(strcmp("DISH_NOT_PRESENT", buffer) == 0)
				printf("Nella comanda sono presenti piatti che non fanno parte del menu\n");
			else
				printf("Errore in fase di ricezione response per avvenuta ricezione comanda\n");

		} else if(strcmp(input, "conto\n") == 0) {
			/* Invio richiesta di conto */
			write_text_to_buffer((void*)&buffer, input);
			ret = send_data(sd, buffer);

			if(ret < 0) {
				perror("Errore in fase di invio comando: ");
				exit(1); // CAPIRE COME AGIRE IN QUESTI CASI, EVITANDO DI FARE LA EXIT
			}

			for(;;) {
				
				/* Ricezione del conto */
				ret = receive_data(sd, (void*)&buffer);

				if(ret < 0) {
					perror("Errore in fase di invio comando: ");
					exit(1); // CAPIRE COME AGIRE IN QUESTI CASI, EVITANDO DI FARE LA EXIT
				}

				if(strcmp(buffer, "NO_ORDERS\0") == 0) {
					printf("Impossibile calcolare il conto: non è presente nessuna comanda associata\n");
					break;
				}

				if(strcmp("LAST_DISH\0", buffer) == 0) {
					ret = receive_data(sd, (void*)&buffer);

					if(ret < 0) {
						perror("Errore in fase di ricezione comando");
						exit(1); // CAPIRE COME AGIRE IN QUESTI CASI, EVITANDO DI FARE LA EXIT
					}

					printf("%s\n", buffer);
					break;
				}
				
				printf("%s\n", buffer);	
			}

			fflush(stdout);

			/* Pulizia memoria e disconnessione dopo aver chiesto il conto */
			free_mem((void*)&buffer);
			free_mem((void*)&input);

			temp_table = table_list;
			
			while(table_list != NULL) {
				temp_table = temp_table->next;
				free_mem((void*)&table_list);
				table_list = temp_table;
			}

			temp_table = NULL;

			printf("Grazie e arrivederci!\n");
			exit(0);

		} else {	
			printf("Comando errato. Utilizzare solo i comandi consentiti\n");
		}
	}
}
