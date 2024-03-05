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

int main(int argc, char* argv[]) {
	int ret, newfd, listener, i, addrlen, server_port, fdmax;
	fd_set master, read_fds;
	struct device *received_device, *device_list, *temp_device;
	struct cmd_struct* command;	
	struct sockaddr_in server_addr, device_addr;
	char *buffer, *input;
	struct table *table_list, *temp_table;
	struct dish *dish_list, *temp_dish;
	struct order* temp_order;

	received_device = NULL;
	device_list = NULL;
	temp_device = NULL;
	command = NULL;
	buffer = NULL;
	input = NULL;
	table_list = NULL;
	temp_table = NULL;
	dish_list = NULL;
	temp_dish = NULL;
	temp_order = NULL;

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

						if(check_shutdown(device_list)) {
							LOG_INFO("Invio richiesta di disconnessione ai Table Device e Kitchen Device");
							
							temp_device = device_list;
						
							while(temp_device != NULL) {
									
								write_text_to_buffer((void*)&buffer, "SHUTDOWN");
								ret = send_data(temp_device->fd, buffer);
											      
								if(ret < 0) {
									LOG_ERROR("Errore di comunicazione con il device in fase di spegnimento! Chiudo la comunicazione.");	
									close(i);
									FD_CLR(i, &master);
								}

								temp_device = temp_device->next;
							}		

							LOG_INFO("Arresto del server in corso...");
							sleep(2);
							LOG_INFO("Arresto completato");	

							free_table_list(&table_list);
							free_mem((void*)&temp_table);
							free_mem((void*)&command);
							free_mem((void*)&buffer);
							free_mem((void*)&input);

							exit(0);

						} else {
							LOG_WARN("Impossibile arrestare il server. Ci sono ancora comande nello stato 'in preparazione' o 'in attesa'.");
						}
	
					} else if(strncmp(input, "stat", 4) == 0) {
						
						temp_device = device_list;

						buffer = (char*)malloc(sizeof(char) * TABLE_LEN);
						
						/* stat: Restituisce lo stato di tutte le comande giornaliere */
						if(strcmp(input, "stat\n") == 0) {
							while(temp_device != NULL) {
								print_all_orders(temp_device->comande);
								temp_device = temp_device->next;
							}
						} 	
						/* stat <status>: Restituisce tutte le comande in corso con lo stato indicato */
						else if(strcmp(input, "stat a\n") == 0) { // stato 'in attesa'
							while(temp_device != NULL) {
								print_orders_by_state(temp_device->comande, 'a');
								temp_device = temp_device->next;
							}
						} else if(strcmp(input, "stat p\n") == 0) {	
							while(temp_device != NULL) {
								print_orders_by_state(temp_device->comande, 'p');
								temp_device = temp_device->next;
							}
						} else if(strcmp(input, "stat s\n") == 0) {	
							while(temp_device != NULL) {
								print_orders_by_state(temp_device->comande, 's');
								temp_device = temp_device->next;
							}
						}
						/* stat <table>: Restituisce tutte le comande relative al tavolo table */
						else if(sscanf(input, "stat %s", buffer) == 1) {
							while(temp_device != NULL) {
								print_orders_by_table(temp_device->comande, buffer);
								temp_device = temp_device->next;
							}
						}

						free_mem((void*)&buffer);
					}
				} else if(i == listener) {

					LOG_INFO("Nuovo device rilevato, connessione in corso...");
					addrlen = sizeof(device_addr);
					newfd = accept(listener, (struct sockaddr*)&device_addr, (socklen_t*)&addrlen);	
					FD_SET(newfd, &master);
					LOG_INFO("Connessione effettuata con successo.");
					if(newfd > fdmax)
						fdmax = newfd;
				} else { /* Se viene rilevata una nuova richiesta da un device (tramite una send) */
					/* Inserisco il comando ricevuto dal device nel buffer */
					ret = receive_data(i, (void*)&buffer);
				
					if(ret == 0) {

						received_device = find_device_by_fd(&device_list, i);
						set_LOG_INFO();
						printf("Chiusura device comunicante su porta %d rilevata\n", (*received_device).port); /* DIRE DI QUALE CLIENT SI TRATTA */	
						fflush(stdout);
						delete_device(&device_list, i); // La free del received_device viene fatta dalla delete
						close(i);
						FD_CLR(i, &master);

					} else if (strcmp(buffer, "RECOGNIZE_ME") == 0) { /* Se il device si è appena collegato e chiede di essere riconosciuto */
				
						LOG_INFO("Riconoscimento nuovo device in corso...");	
						
						/* Invio conferma ricezione comando per l'inizio della fase di riconoscimento */
						write_text_to_buffer((void*)&buffer, "START_RECOGNIZE");
						ret = send_data(i, buffer);
											      
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del device! Chiudo la comunicazione. (STEP 1)");	
							close(i);
							FD_CLR(i, &master);
							continue;
						}

						ret = receive_data(i, (void*)&buffer);
						LOG_INFO("Ho ricevuto il device!");

						if(ret < 0) {
							perror("Errore:\n");
							LOG_ERROR("Errore in fase di riconoscimento del device! Chiudo la comunicazione. (STEP 2)");
							close(i);
							FD_CLR(i, &master);
							continue;
						}
						
						received_device = create_device(i, 0, 0);
						/* Popolo la struttura received_device con le informazioni del nuovo device */	
						sscanf(buffer, "%d %u", &(received_device->port), &(received_device->type));

						/* Inserisco nella lista dei device collegati il nuovo device */
						ret = add_device(&device_list, received_device);
						if(ret <= 0) {
							/* Se si è verificato un errore durante l'aggiunta alla lista dei device collegati */
							LOG_ERROR("Errore in fase di riconoscimento del device! Chiudo la comunicazione. (STEP 3)");
							close(i);
							FD_CLR(i, &master);
							continue;
						}

						/* Invio ACK al device per segnalare il completamento della fase di riconoscimento */
						LOG_INFO("Invio ACK per segnalare la fine della fase di riconoscimento al device");
						write_text_to_buffer((void*)&buffer, "END_RECOGNIZE\0");
						
						ret = send_data(i, buffer);
						if(ret < 0) {
							LOG_ERROR("Errore in fase di riconoscimento del device! Chiudo la comunicazione. (STEP 4)");	
							delete_device(&device_list, i);	
							close(i);
							FD_CLR(i, &master);	
							continue;
						}

					} else { /* Logiche per i comandi ricevuti */

						/* Riconoscimento del device attraverso il type */	
						received_device = find_device_by_fd(&device_list, i);

						if(received_device == NULL) { // Il device non è tra quelli che hanno chiesto di essere riconosciuti
							
							LOG_ERROR("Errore: un device ha tentato di inviare un comando prima di essere riconosciuto");
							continue;	

						} else if(received_device->type == CL) {
						
							LOG_INFO("Il comando è stato ricevuto da un Client");
						
							/* Riconoscimento del comando */
							if(strncmp(buffer, "find", 4) == 0) { // Comando 'find'
						
								command = create_cmd_struct_find(buffer);
								
								if(command == NULL) {
									LOG_ERROR("Struttura della find errata!");
								}	

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
										delete_device(&device_list, i);
										close(i);
										FD_CLR(i, &master);
									}

									continue; // Sia che la send vada in errore, sia che vada a buon fine
								}
								
								while(temp_table != NULL) {
									free_mem((void*)&buffer);
									buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);
									
									sprintf(buffer, "%s %s %s", &temp_table->table[0], &temp_table->room[0], &temp_table->position[0]);
									ret = send_data(i, (void*)buffer); 
								
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio dei tavoli prenotabili. Chiudo la comunicazione");
										delete_device(&device_list, i);
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
											delete_device(&device_list, i);
											close(i);
											FD_CLR(i, &master);
											break;
										}
									}
								}
								
								LOG_INFO("Invio dei tavoli prenotabili completato.");

								/* Salvataggio della lista dei tavoli prenotabili per questo device,
								 * per poterla poi riutilizzare alla ricezione del comando 'book' */
							
								received_device->bookable_table = table_list;
								table_list = NULL;

								/* Salvataggio del command find, in particolare dei parametri passati,
								 * per poterli poi riutilizzare alla ricezione del comando 'book' */
								
								received_device->find_cmd = command;

							} else if(strncmp(buffer, "book", 4) == 0) { // Comando 'book'
								
								command = create_cmd_struct_book(buffer, received_device->bookable_table);
								
								/* Aggiunta della prenotazione */ 
								// save_booking ritorna il codice di prenotazione (char*) se va tutto bene altrimenti BOOK_KO
								ret = save_booking(command, received_device->find_cmd, received_device->bookable_table, &buffer);
								// Scrittura su buffer con write_text_to_buffer dentro la save_booking
								
								if(ret == -1) {
									write_text_to_buffer((void*)&buffer, "BOOK_KO"); // Segnalo che ci sono stati problemi al device
									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore in fase di prenotazione. Chiudo la comunicazione");
										delete_device(&device_list, i);
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
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									continue;
								}
								
								free_table_list(&received_device->bookable_table);
								LOG_INFO("Prenotazione effettuata");
							}

						} else if(received_device->type == TD) {
							LOG_INFO("Il comando è stato ricevuto da un Table Device");	
						
							/* Riconoscimento del comando */
							if(strncmp(buffer, "login", 5) == 0) { // Verifica codice prenotazione
								
								command = create_cmd_struct_login(buffer);
								
								/* Verifica il booking code passato dal device.
								 * Se il booking code esiste allora il buffer verrà valorizzato con il messaggio
								 * BOOKING_CODE_IS_VALID altrimenti BOOKING_CODE_IS_NOT_VALID*/

								verify_booking_code(
										(char*)command->args[0], // args[0]: Contiene il booking code nella command 'login'
										(void*)&buffer);
								
								received_device->booking = get_booking_from_code((char*)command->args[0]); // args[0]: Contiene il booking code nella command 'login'
								
								if(strcmp(buffer, "BOOKING_CODE_IS_VALID") == 0) {	
									set_LOG_INFO();
									printf("Si è collegato il TD associato alla prenotazione %s, tavolo: %s\n", (char*)command->args[0], (received_device->booking)->table);
									fflush(stdout);
								}

								free_mem((void*)&command);
								
								ret = send_data(i, (void*)buffer);
									
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio della response per la verifica del codice prenotazione");
									delete_device(&device_list, i);
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
									buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);

									sprintf(buffer, "%d %s - %s", temp_dish->price, &temp_dish->identifier[0], &temp_dish->description[0]);
									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio dei piatti del menu. Chiudo la comunicazione");
										delete_device(&device_list, i);
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
											delete_device(&device_list, i);
											close(i);
											FD_CLR(i, &master);
											break;
										}
									}
								}

								//free_mem dish list
								
								LOG_INFO("Invio dei piatti del menu completato.");

							} else if(strncmp(buffer, "comanda", 7) == 0) { // Comando 'comanda'
								
								command = create_cmd_struct_comanda(buffer, (received_device->booking)->table, received_device->fd);
							
								/* Setta lo stato della comanda 'in attesa' */
								((struct order*)command->args[0])->state = 'a';

								/* Check che la comanda contenga solo piatti che fanno parte del menu del giorno */
								temp_dish = ((struct order*)command->args[0])->dish_list;
								ret = check_dishes(temp_dish);
								
								if(ret < 0) {
									LOG_WARN("La comanda contiene piatti non presenti nel menu. Invio segnalazione al Table Device");

									write_text_to_buffer((void*)&buffer, "DISH_NOT_PRESENT");	
									ret = send_data(i, (void*)buffer);
								
									if(ret < 0) {
										set_LOG_ERROR();
										perror("Errore durante la comunicazione con il TD. Chiudo la comunicazione");
										delete_device(&device_list, i);
										close(i);
										FD_CLR(i, &master);
									}

									continue;
								}

								/* Aggiunta della comanda nella lista delle comande */
								add_to_orders_list_with_increment( // Si occupa anche di incremenare il contatore delle comande!!!
										&received_device->comande, // Lista delle comande relative al Table Device che ha inviato la comanda
										(struct order*)command->args[0] // Nuova comanda da aggiungere
										);


								/* Invio notifica ai Kitchen Device per segnalare l'arrivo della comanda */
								
								send_notify_to_all_kd(device_list);

								/* Invio segnalazione al TD per comanda ricevuta */
								free_mem((void*)&buffer);
								buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);	
								
								sprintf(buffer, "COMANDA RICEVUTA (%s)", (char*)((struct order*)command->args[0])->com_count);
								ret = send_data(i, (void*)buffer);
								
								if(ret < 0) {
									set_LOG_ERROR();
									perror("Errore durante l'invio segnalazione per comanda ricevuta. Chiudo la comunicazione");
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									continue;
								}

							} else if(strncmp(buffer, "conto", 5) == 0) { // Comando 'conto'
								
								dish_list = get_all_dishes_by_order(received_device->comande);
								temp_dish = dish_list;

								if(dish_list == NULL) { // Se non esistono comande 
									write_text_to_buffer((void*)&buffer, "NO_ORDERS\0");
									ret = send_data(i, (void*)buffer);

									if(ret < 0) {
										set_LOG_ERROR();
										perror("Errore durante l'invio dei tavoli prenotabili");
									}

									continue; // Sia che la send vada in errore, sia che vada a buon fine
								}
								
								while(temp_dish != NULL) {
									free_mem((void*)&buffer);
									buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);
									
									sprintf(buffer, "%s %d %d", &temp_dish->identifier[0], temp_dish->quantity, temp_dish->price);
									ret = send_data(i, (void*)buffer); 
								
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio del conto");
										break;
									}
								
									temp_dish = temp_dish->next;
								}

								write_text_to_buffer((void*)&buffer, "LAST_DISH\0");	
								ret = send_data(i, (void*)buffer);

								if(ret < 0) {
									set_LOG_ERROR();
									perror("Errore durante l'invio del conto");
									continue;
								}

								buffer = get_total_cost_by_dish_list(dish_list);
								ret = send_data(i, (void*)buffer);
										
								if(ret < 0) {
									set_LOG_ERROR();
									perror("Errore durante l'invio del conto");
									continue;
								}
								
								LOG_INFO("Invio del conto completato.");

							}

						} else if(received_device->type == KD) {	

							LOG_INFO("Il comando è stato ricevuto da un Kitchen Device");

							/* Riconoscimento del comando */
							if(strncmp(buffer, "take", 4) == 0) { // Comando 'take'
						
								/* Recupero della comanda meno recente ancora in stato 'in attesa' */
								
								temp_order = get_oldest_order_in_pending(device_list);

								if(temp_order == NULL) {
									LOG_WARN("NO ORDER!");
									write_text_to_buffer((void*)&buffer, "NO_ORDERS");
									ret = send_data(i, (void*)buffer);
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio delle comande");
									}

									continue;	
								}

								/* Invio della comanda (senza la lista piatti) */
								free_mem((void*)&buffer);
								buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);	
								
								sprintf(buffer, "order %s-%s", &temp_order->table[0], &temp_order->com_count[0]);
								ret = send_data(i, (void*)buffer); 
								
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio dei tavoli prenotabili. Chiudo la comunicazione");
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									break;
								}

								/* Invio dei piatti della comanda */
								
								temp_dish = temp_order->dish_list;

								while(temp_dish != NULL) {
									
									free_mem((void*)&buffer);
									buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);

									sprintf(buffer, "dish %s-%d", &temp_dish->identifier[0], temp_dish->quantity);
									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio dei piatti della comanda. Chiudo la comunicazione");
										delete_device(&device_list, i);
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
											perror("Errore durante l'invio dei piatti della comanda. Chiudo la comunicazione");
											delete_device(&device_list, i);
											close(i);
											FD_CLR(i, &master);
											break;
										}
									}
								}

								LOG_INFO("Invio della comanda meno recente in stato di attesa completato.");

								/* Cambio stato della comanda da 'in attesa' a 'in preparazione' */	
							
								temp_order->state = 'p';

								/* Invio notifica al Table Device per segnalare il cambio di stato della comanda */
								free_mem((void*)&buffer);
								buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);
								
								sprintf(buffer, "COMANDA IN PREPARAIZONE (%s)",
										temp_order->com_count); // com_count
							
								ret = send_data(temp_order->sd, (void*)buffer); 
								
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio dei piatti della comanda. Chiudo la comunicazione");
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									break;
								}
								
								LOG_INFO("Notifico il TD per il passaggio di stato della comanda.");

							} if(strncmp(buffer, "ready", 4) == 0) { // Comando 'ready'
								
								command = create_cmd_struct_ready(buffer);
								
								if(command == NULL) {
									LOG_ERROR("Struttura della ready errata!");
									continue;
								}

								/* Ricerca della comanda */
								
								temp_device = device_list;

								while(temp_device != NULL) {
									
									temp_order = find_order_in_orders_list(temp_device->comande,
											(char*)command->args[0],  // com_count
											(char*)command->args[1]); // table

									if(temp_order != NULL) {
										break;
									}

									temp_device = temp_device->next;
								}

								if(temp_order == NULL) {
									write_text_to_buffer((void*)buffer, "ERR_CHG_STATE");
									LOG_ERROR("Errore durante il cambio di stato della comanda!");

									ret = send_data(i, (void*)buffer); 
									
									if(ret < 0) {
										LOG_ERROR("Errore durante l'invio del messaggio");
										delete_device(&device_list, i);
										close(i);
										FD_CLR(i, &master);
										break;
									}
								
									continue;
								}

								/* Set della comanda 'in servizio' */
								temp_order->state = 's';

								write_text_to_buffer((void*)&buffer, "OK_CHG_STATE");

								ret = send_data(i, (void*)buffer); 
									
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio del messaggio");
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									break;
								}

								set_LOG_INFO();
								printf("Comanda %s-%s in servizio!\n", 
									(char*)command->args[0],  // com_count
									(char*)command->args[1]); // table
								fflush(stdout);
	
								/* Invio notifica al Table Device per segnalare il cambio di stato della comanda */
							
								free_mem((void*)&buffer);
								buffer = (char*)malloc(sizeof(char) * GENERIC_DATA_LEN);
								
								sprintf(buffer, "COMANDA IN SERVIZIO (%s)",
										(char*)command->args[0]); // com_count

								ret = send_data(temp_order->sd, (void*)buffer); 
								
								if(ret < 0) {
									LOG_ERROR("Errore durante l'invio dei piatti della comanda. Chiudo la comunicazione");
									delete_device(&device_list, i);
									close(i);
									FD_CLR(i, &master);
									break;
								}
								
								LOG_INFO("Notifico il TD per il passaggio di stato della comanda.");
							}
						}
					}	
				
				}
			}
		}
	}	
}
