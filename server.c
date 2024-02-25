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
#define INPUT_SIZE 512

int main(int argc, char* argv[]) {
	int ret, newfd, listener, i, addrlen, server_port, fdmax;
	fd_set master, read_fds;
	struct client_device *received_client, *client_list, *temp_client;
	struct cmd_struct* command;	
	struct sockaddr_in server_addr, client_addr;
	char *buffer, *input;
	struct table *table_list, *temp_table;
	struct dish *dish_list, *temp_dish;

	received_client = NULL;
	client_list = NULL;
	temp_client = NULL;
	command = NULL;
	buffer = NULL;
	input = NULL;
	table_list = NULL;
	temp_table = NULL;
	dish_list = NULL;
	temp_dish = NULL;

	if(!check_port(argc, argv)) {
		printf("Argomenti errati. Specificare correttamente il comando come segue: ./server <porta>\n");
		exit(0);
	}

	server_port = atoi(argv[1]);

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
			  
	FD_SET(0, &master); // Descrittore STDIN del server
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
				
				if(i == 0) { // Gestione comandi ricevuti dallo Standard Input del server
					
					input = (char*)malloc(sizeof(char) * INPUT_SIZE);
					memset(input, 0, INPUT_SIZE);
					fgets(input, INPUT_SIZE, stdin);
					
					if(strcmp(input, "stop\n") == 0) {

						LOG_INFO("Chiusura server in corso...");
						//shutdown_server(client_list, fdmax);	i
						
						struct client_device* prec_cli;
					
						prec_cli = NULL;
					
						while(client_list != NULL) {
							//delete all client device
							set_LOG_INFO();
							printf("elimino client %d", client_list->fd);
							fflush(stdout);
							prec_cli = client_list;
							client_list = client_list->next;
							free_mem((void*)&prec_cli);
						}

						free_table_list(&table_list);
						free_mem((void*)&temp_table);
						free_mem((void*)&command);
						free_mem((void*)&buffer);
						free_mem((void*)&input);

						for(i = 0; i < fdmax; i++) {
							close(i);	
						}
					
						exit(0);
	
					} else if(strncmp(input, "stat", 4) == 0) {
						
						// Valutare se spostare tutte queste in funzioni apposite in libreria esterna
						temp_client = client_list;

						buffer = (char*)malloc(sizeof(char) * TABLE_LEN);
						
						/* stat: Restituisce lo stato di tutte le comande giornaliere */
						if(strcmp(input, "stat\n") == 0) {
							while(temp_client != NULL) {
								print_all_orders(temp_client->comande);
								temp_client = temp_client->next;
							}
						} 	
						/* stat <status>: Restituisce tutte le comande in corso con lo stato indicato */
						else if(strcmp(input, "stat a\n") == 0) { // stato 'in attesa'
							while(temp_client != NULL) {
								print_orders_by_state(temp_client->comande, 'a');
								temp_client = temp_client->next;
							}
						} else if(strcmp(input, "stat p\n") == 0) {	
							while(temp_client != NULL) {
								print_orders_by_state(temp_client->comande, 'p');
								temp_client = temp_client->next;
							}
						} else if(strcmp(input, "stat s\n") == 0) {	
							while(temp_client != NULL) {
								print_orders_by_state(temp_client->comande, 's');
								temp_client = temp_client->next;
							}
						}
						/* stat <table>: Restituisce tutte le comande relative al tavolo table */
						else if(sscanf(input, "stat %s", buffer) == 1) {
							while(temp_client != NULL) {
								print_orders_by_table(temp_client->comande, buffer);
								temp_client = temp_client->next;
							}
						}

						free_mem((void*)&buffer);
					}
				} else if(i == listener) {

					LOG_INFO("Nuovo client rilevato, connessione in corso...");
					addrlen = sizeof(client_addr);
					newfd = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);	
					FD_SET(newfd, &master);
					LOG_INFO("Connessione effettuata con successo.");
					if(newfd > fdmax)
						fdmax = newfd;
				} else { /* Se viene rilevata una nuova richiesta da un client (tramite una send) */
					/* Inserisco il comando ricevuto dal client nel buffer */
					ret = receive_data(i, (void*)&buffer);
				
					if(ret == 0) {

						received_client = find_client_device_by_fd(&client_list, i);
						set_LOG_INFO();
						printf("Chiusura client comunicante su porta %d rilevata\n", (*received_client).port); /* DIRE DI QUALE CLIENT SI TRATTA */	
						fflush(stdout);
						delete_client_device(&client_list, i); // La free del received_client viene fatta dalla delete
						close(i);
						FD_CLR(i, &master);

					} else if (strcmp(buffer, "RECOGNIZE_ME") == 0) { /* Se il client si è appena collegato e chiede di essere riconosciuto */
				
						LOG_INFO("Riconoscimento nuovo client in corso...");	
						
						/* Invio conferma ricezione comando per l'inizio della fase di riconoscimento */
						write_text_to_buffer((void*)&buffer, "START_RECOGNIZE");
						ret = send_data(i, buffer);
											      
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 1)");	
							close(i);
							FD_CLR(i, &master);
							continue;
						}

						ret = receive_data(i, (void*)&buffer);
						LOG_INFO("Ho ricevuto il client_device!");

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
						write_text_to_buffer((void*)&buffer, "END_RECOGNIZE\0");
						
						ret = send_data(i, buffer);
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del client! Chiudo la comunicazione. (STEP 4)");	
							delete_client_device(&client_list, i);	
							close(i);
							FD_CLR(i, &master);	
							continue;
						}

					} else { /* Logiche per i comandi ricevuti */

						/* Riconoscimento del client attraverso il type */	
						received_client = find_client_device_by_fd(&client_list, i);

						if(received_client == NULL) { // Il client non è tra quelli che hanno chiesto di essere riconosciuti
							
							LOG_ERROR("Errore: un client ha tentato di inviare un comando prima di essere riconosciuto");
							continue;	

						} else if(received_client->type == CL) {
						
							LOG_INFO("Il comando è stato ricevuto da un Client");
						
							/* Riconoscimento del comando */
							if(strncmp(buffer, "find", 4) == 0) { // Comando 'find'
						
								command = create_cmd_struct_find(buffer);
								
								/* Popolo struct lista tavoli prenotabili da inviare */
								table_list = get_bookable_table(*((int*)command->args[1]), *((int*)command->args[2]), 
										*((int*)command->args[3]), *((int*)command->args[4]), *((int*)command->args[5]));
								
								/* Invio dei tavoli prenotabili */
								temp_table = table_list;
							
								if(table_list == NULL) { // Se non esistono tavoli prenotabili per i parametri di ricerca selezionati
									write_text_to_buffer((void*)&buffer, "NO_TABLE\0");
									ret = send_data(i, (void*)buffer);

									if(ret < 0) {
										set_LOG_ERROR();
										perror("Errore durante l'invio dei tavoli prenotabili. Chiudo la comunicazione");
										delete_client_device(&client_list, i);
										close(i);
										FD_CLR(i, &master);
									}

									continue; // Sia che la send vada in errore, sia che vada a buon fine
								}
								
								while(temp_table != NULL) {
									// DA VERIFICARE SE C'E' DA FARE UNA MALLOC DEL BUFFER QUI!
									sprintf(buffer, "%s %s %s", &temp_table->table[0], &temp_table->room[0], &temp_table->position[0]);
									ret = send_data(i, (void*)buffer); 
								
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
										ret = send_data(i, (void*)buffer);
										
										if(ret < 0) {
											set_LOG_ERROR();
											perror("Errore durante l'invio dei tavoli prenotabili. Chiudo la comunicazione");
											delete_client_device(&client_list, i);
											close(i);
											FD_CLR(i, &master);
											break;
										}
									}
								}
								
								LOG_INFO("Invio dei tavoli prenotabili completato. ");

								/* Salvataggio della lista dei tavoli prenotabili per questo client,
								 * per poterla poi riutilizzare alla ricezione del comando 'book' */
							
								received_client->bookable_table = table_list;
								table_list = NULL;

								/* Salvataggio del command find, in particolare dei parametri passati,
								 * per poterli poi riutilizzare alla ricezione del comando 'book' */
								
								received_client->find_cmd = command;

							} else if(strncmp(buffer, "book", 4) == 0) { // Comando 'book'
								
								command = create_cmd_struct_book(buffer, received_client->bookable_table);
								
								/* Aggiunta della prenotazione */ 
								// save_booking ritorna il codice di prenotazione (char*) se va tutto bene altrimenti BOOK_KO
								ret = save_booking(command, received_client->find_cmd, received_client->bookable_table, &buffer);
								// Scrittura su buffer con write_text_to_buffer dentro la save_booking
								
								if(ret == -1) {
									write_text_to_buffer((void*)&buffer, "BOOK_KO"); // Segnalo che ci sono stati problemi al client
									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore in fase di prenotazione. Chiudo la comunicazione");
										delete_client_device(&client_list, i);
										close(i);
										FD_CLR(i, &master);
										continue;
									}
								}

								/* Send codice prenotazione con il seguente formato: BOOK_OK_<BOOKING_CODE> */
								
								ret = send_data(i, (void*)buffer); 
									
								if(ret < 0) {
									set_LOG_ERROR();
									perror("Errore durante l'invio di conferma prenotazione. Chiudo la comunicazione");
									delete_client_device(&client_list, i);
									close(i);
									FD_CLR(i, &master);
									continue;
								}
								
								free_table_list(&received_client->bookable_table);
								LOG_INFO("Prenotazione effettuata");
							}

						} else if(received_client->type == TD) {
							LOG_INFO("Il comando è stato ricevuto da un Table Device");	
						
							/* Riconoscimento del comando */
							if(strncmp(buffer, "login", 5) == 0) { // Verifica codice prenotazione
								
								command = create_cmd_struct_login(buffer);
								
								/* Verifica il booking code passato dal client.
								 * Se il booking code esiste allora il buffer verrà valorizzato con il messaggio
								 * BOOKING_CODE_IS_VALID altrimenti BOOKING_CODE_IS_NOT_VALID*/

								verify_booking_code(
										(char*)command->args[0], // args[0]: Contiene il booking code nella command 'login'
										(void*)&buffer);
								
								received_client->booking = get_booking_from_code((char*)command->args[0]); // args[0]: Contiene il booking code nella command 'login'
								
								set_LOG_INFO();
								printf("Si è collegato il TD associato alla prenotazione %s, tavolo: %s\n", (char*)command->args[0], (received_client->booking)->table);
								fflush(stdout);

								free_mem((void*)&command);
								
								ret = send_data(i, (void*)buffer);
									
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio della response per la verifica del codice prenotazione");
									delete_client_device(&client_list, i);
									close(i);
									FD_CLR(i, &master);
									continue;
								}

							} else if(strncmp(buffer, "menu", 4) == 0) { // Comando 'menu'
						
								/* Popolo la struct lista piatti da inviare */
								dish_list = load_menu_dishes();
							
								/* Invio dei piatti del menu */
								temp_dish = dish_list;
							
								while(temp_dish != NULL) {
									free_mem((void*)&buffer);
									buffer = (char*)malloc(sizeof(char) * 2048);

									sprintf(buffer, "%d %s - %s", temp_dish->price, &temp_dish->identifier[0], &temp_dish->description[0]);
									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio dei piatti del menu. Chiudo la comunicazione");
										delete_client_device(&client_list, i);
										close(i);
										FD_CLR(i, &master);
										break;
									}
								
									temp_dish = temp_dish->next;

									if(temp_dish == NULL) {
										write_text_to_buffer((void*)&buffer, "END_MSG");
										ret = send_data(i, (void*)buffer);
										
										if(ret < 0) {
											set_LOG_ERROR();
											perror("Errore durante l'invio dei piatti del menu. Chiudo la comunicazione");
											delete_client_device(&client_list, i);
											close(i);
											FD_CLR(i, &master);
											break;
										}
									}
								}

								//free_mem dish list
								
								LOG_INFO("Invio dei piatti del menu completato.");

							} else if(strncmp(buffer, "comanda", 7) == 0) { // Comando 'comanda'
								
								command = create_cmd_struct_comanda(buffer, (received_client->booking)->table, received_client->fd);
							
								/* Setta lo stato della comanda 'in attesa' */
								((struct comanda*)command->args[0])->state = 'a';

								/* Check che la comanda contenga solo piatti che fanno parte del menu del giorno */
								temp_dish = ((struct comanda*)command->args[0])->dish_list;
								ret = check_dishes(temp_dish);
								
								if(ret < 0) {
									LOG_WARN("La comanda contiene piatti non presenti nel menu. Invio segnalazione al Table Device");

									write_text_to_buffer((void*)&buffer, "DISH_NOT_PRESENT");	
									ret = send_data(i, (void*)buffer);
								
									if(ret < 0) {
										set_LOG_ERROR();
										perror("Errore durante la comunicazione con il TD. Chiudo la comunicazione");
										delete_client_device(&client_list, i);
										close(i);
										FD_CLR(i, &master);
									}

									continue;
								}

								/* Aggiunta delle comanda nella lista delle comande */
								add_to_orders_list( // Si occupa anche di incremenare il contatore delle comande!!!
										&received_client->comande, // Lista delle comande relative al Table Device che ha inviato la comanda
										(struct comanda*)command->args[0] // Nuova comanda da aggiungere
										);

								/* Invio segnalazione al TD per comanda ricevuta */
								
								write_text_to_buffer((void*)&buffer, "ORDER_RECEIVED");
								ret = send_data(i, (void*)buffer);
								
								if(ret < 0) {
									set_LOG_ERROR();
									perror("Errore durante l'invio segnalazione per comanda ricevuta. Chiudo la comunicazione");
									delete_client_device(&client_list, i);
									close(i);
									FD_CLR(i, &master);
									continue;
								}
								
								/* Inserimento della comanda nella lista dei piatti ordinati dal client_device TD */
								//add_to_dish_list(&received_client->dishes_ordered, ((struct comanda*)(command->args[0]))->dish_list);
								// Da ccancellare!


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
