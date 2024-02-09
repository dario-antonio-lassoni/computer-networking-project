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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libs/logger.h"
#include "libs/device_utils.h"
#include "libs/common_utils.h"
#include "libs/database.h"

#define BACKLOG_SIZE 10 /* Default backlog size del listener */
#define BUFFER_SIZE 1024 /* Dimensione massima del buffer usato per la comunicazione con i client */

int main(int argc, char* argv[]) {
	int ret, newfd, listener, i, addrlen, server_port, fdmax;
	fd_set master, read_fds;
	struct client_device* received_client = NULL;
	struct client_device* client_list = NULL;
	struct cmd_struct* command;	
	struct sockaddr_in server_addr, client_addr;
	char* buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	struct table *table_list, *temp_table;

	if(argc != 2) {
		printf("Argomenti errati. Specificare correttamente il comando come segue: ./server <porta>\n");
		exit(0);
	}

	server_port = atoi(argv[1]); /* Funziona ma fare meglio, controlli su numero di argoment, sulla porta ecc... */
	/* Check che la porta sia formata solo da numeri e non da lettere */

	LOG_INFO("Inizializzazione del server in corso...");
	
	/* Creazione socket */
	LOG_INFO("Creazione socket...");
	listener = socket(AF_INET, SOCK_STREAM, 0);

	/* Creazione indirizzo di bind */
	LOG_INFO("Creazione indirizzo server...");
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	
	/* Binding */
	LOG_INFO("Binding...");
	ret = bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0) {
		set_LOG_ERROR();
		perror("Errore durante la bind");
		exit(0);
	}
	
	/* Pongo il server in ascolto */
	listen(listener, BACKLOG_SIZE);

	/* Init dei set utilizzati per la select */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
			  
	FD_SET(listener, &master);
	fdmax = listener;
	LOG_INFO("Server avviato e in ascolto.");
	
	for(;;) {

		/* init del read_fds usato nella select() */
		read_fds = master;
		
		/* Mi blocco in attesa di descrittori pronti in lettura */
		ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL); 
		if(ret < 0) { 
			LOG_ERROR("Errore durante la select");
			exit(1);
		}

		/* Scorro tutti i descrittori nella read_fds */
		for(i = 0; i <= fdmax; i++) {
			/* Controllo se i è pronto */
			if(FD_ISSET(i, &read_fds)) {
				if(i == listener) {			
					LOG_INFO("Nuovo client rilevato, connessione in corso...");
					addrlen = sizeof(client_addr);
					newfd = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);	
					FD_SET(newfd, &master);
					LOG_INFO("Connessione effettuata con successo.");
					if(newfd > fdmax)
						fdmax = newfd;
				} else { /* Se viene rilevata una nuova richiesta da un client (tramite una send) */
					/* Inserisco il comando ricevuto dal client nel buffer */
					//memset(buffer, 0, BUFFER_SIZE);
					ret = recv(i, (void*)buffer, BUFFER_SIZE, 0);
					//ret = receive_data(i, (void*)&buffer);
				
					if(ret == 0) {

						received_client = find_client_device_by_fd(&client_list, i);
						set_LOG_INFO();
						printf("Chiusura client comunicante su porta %d rilevata\n", (*received_client).port); /* DIRE DI QUALE CLIENT SI TRATTA */	
						fflush(stdout);
						delete_client_device(&client_list, i); /* La free del received_client viene fatta dalla delete */
						close(i);
						FD_CLR(i, &master);

					} else if (strcmp(buffer, "RECOGNIZE_ME") == 0) { /* Se il client si è appena collegato e chiede di essere riconosciuto */
				
						LOG_INFO("Riconoscimento nuovo client in corso...");	
						// Invio conferma ricezione comando per l'inizio della fase di riconoscimento
						
						write_text_to_buffer((void*)&buffer, "START_RECOGNIZE");
						//strcpy(buffer, "START_RECOGNIZE");
					
						//ret = send_data(i, buffer);
						ret = send(i, (void*)buffer, strlen(buffer) + 1, 0); //BUFFER_SIZE
											      
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 1)");	
							delete_client_device(&client_list, i);	
							close(i);
							FD_CLR(i, &master);	
							continue;
						}

						//ret = receive_data(i, (void*)&buffer);
						ret = recv(i, (void*)buffer, strlen(buffer) + 1, 0);
						LOG_INFO("Ho ricevuto il client_device!");
						printf("buffer ricevuto: %s\n", buffer);
						fflush(stdout);

						if(ret < 0) {
							perror("Errore:\n");
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 2)");
							close(i);
							FD_CLR(i, &master);
							continue;
						}
						
						received_client = create_client_device(i, 0, 0);
						/* Popolo la struttura received_client con le informazioni del nuovo client */	
						sscanf(buffer, "%d %u", &(received_client->port), &(received_client->type));

						/* Inserisco nella lista dei client collegati il nuovo client */
						ret = add_client_device(&client_list, received_client);
						if(ret <= 0) {
							/* Se si è verificato un errore durante l'aggiunta alla lista dei device collegati */
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 3)");
							close(i);
							FD_CLR(i, &master);
							continue;
						}

						/* Invio ACK al client per segnalare il completamento della fase di riconoscimento */
						LOG_INFO("Invio ACK per segnalare la fine della fase di riconoscimento al client");
						strcpy(buffer, "END_RECOGNIZE");
						
						ret = send(i, (void*)buffer, BUFFER_SIZE, 0); //BUFFER_SIZE
						//ret = send_data(i, buffer);
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 4)");	
							delete_client_device(&client_list, i);	
							close(i);
							FD_CLR(i, &master);	
							continue;
						}

					} else { /* Logiche per i comandi ricevuti */
						/* Riconoscimento del client (attraverso il type) */	
						received_client = find_client_device_by_fd(&client_list, i);

						if(received_client == NULL) { /* Il client non è tra quelli che hanno chiesto di essere riconosciuti */
							
							LOG_ERROR("Errore: un client ha tentato di inviare un comando prima di essere riconosciuto");
							continue;	

						} else if(received_client->type == CL) {
						
							LOG_INFO("Il comando è stato ricevuto da un Client");
						
							/* Riconoscimento del comando */
							if(strncmp(buffer, "find", 4) == 0) { /* Controlla che il comando ricevuto inizi per "find" */
						
								command = create_cmd_struct_find(buffer);
								set_LOG_INFO();
								printf("Comando ricevuto: %s %s %d %d-%d-%d %d\n", (char*)command->cmd, (char*)command->args[0], 
									*((int*)command->args[1]), *((int*)command->args[2]), *((int*)command->args[3]), 
									*((int*)command->args[4]), *((int*)command->args[5]));
							
								
								/* Popolo struct lista tavoli prenotabili da inviare */
								table_list = get_bookable_table(*((int*)command->args[1]), *((int*)command->args[2]), 
										*((int*)command->args[3]), *((int*)command->args[4]), *((int*)command->args[5]));
								
								/* Invio dei tavoli prenotabili */
								temp_table = table_list;

								while(temp_table != NULL) {
									LOG_INFO("INVIO LA TABLE");
									//memset(buffer, 0, BUFFER_SIZE);
								//	write_text_to_buffer(&temp_table->table);
									sprintf(buffer, "%s %s %s", &temp_table->table[0], &temp_table->room[0], &temp_table->position[0]);
								//	ret = send_data(i, (void*)buffer); 
									ret = send(i, (void*)buffer, BUFFER_SIZE, 0); //BUFFER_SIZE
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio dei tavoli prenotabili. Chiudo la comunicazione");
										delete_client_device(&client_list, i);
										close(i);
										FD_CLR(i, &master);
										break;
									}
								

									temp_table = temp_table->next;
									if(temp_table == NULL) {
										strcpy(buffer, "END_MSG\0");	
										ret = send(i, (void*)buffer, BUFFER_SIZE, 0); //BUFFER_SIZE
										//send_data(i, (void*)buffer);
										LOG_WARN("INVIO L'ULTIMO!");
									}
								}
								
								LOG_INFO("Invio table completato");
							}

						} else if(received_client->type == KD) {
							LOG_INFO("Il comando è stato ricevuto da un Kitchen Device");		
						}
					}	
				}
			}
		}
	}	
}
