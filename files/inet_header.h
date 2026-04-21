#ifndef HEADER_H_
#define HEADER_H_

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>

#define QUIT_CMD "quit\n"
#define QUIT_CODE 418

#define SV_ADDR "127.0.0.1"
#define PORT_NUM 30000
#define CT_ADDR INADDR_ANY

#endif // HEADER_H_
