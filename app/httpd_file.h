#ifndef HTTPD_FILE_H
#define HTTPD_FILE_H

#include "error_code.h"
#include "network.h"

error_code_t httpd_file_send(network_client_t* client, const char* filename);
error_code_t httpd_file_init(void);

#endif // HTTPD_FILE_H
