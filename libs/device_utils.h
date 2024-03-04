/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef DEVICE_UTILS_H 
#define DEVICE_UTILS_H

#include "common_utils.h"

struct client_device* find_client_device_by_fd(struct client_device** head, int i);
struct client_device* find_client_device_by_port(struct client_device** head, int i);
struct client_device* create_client_device(int fd, int port, enum client_type type);
int delete_client_device(struct client_device** head, int fd);
int add_client_device(struct client_device** head, struct client_device* client);
int check_shutdown(struct client_device* list);

#endif
