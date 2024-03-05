/**
 *
 * Dario Antonio Lassoni
 * Matricola: 565721
 *
 */

#ifndef DEVICE_UTILS_H 
#define DEVICE_UTILS_H

#include "common_utils.h"

struct device* find_device_by_fd(struct device** head, int i);
struct device* find_device_by_port(struct device** head, int i);
struct device* create_device(int fd, int port, enum device_type type);
int delete_device(struct device** head, int fd);
int add_device(struct device** head, struct device* device);
int check_shutdown(struct device* list);

#endif
