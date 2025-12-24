#ifndef NETWORK_H
#define NETWORK_H

#include "error_code.h"

#include <stdint.h>

typedef struct network_s network_t;
typedef struct network_client_s network_client_t;
typedef void (*network_connected_callback_t)(network_client_t* client, void* userdata);

error_code_t network_read(network_client_t* client, void* buffer, uint32_t* length);
error_code_t network_write(network_client_t* client, const void* buffer, uint32_t length);
error_code_t network_close(network_client_t* client);

error_code_t network_listen(network_t* network, uint16_t port, network_connected_callback_t connected, void* userdata);
error_code_t network_create(network_t** network);
error_code_t network_init(void);

#endif // NETWORK_H
