#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#define MYUDP_PORT 5350
#define DATA_LEN 1400 // Maximum data length

struct data_pkt{ 
    uint32_t num; // The sequence number
    uint32_t len; // The packet length
    char data[DATA_LEN]; // The packet data
};

struct ack_pkt {
    uint32_t num;   // The sequence number
    uint32_t flag;  // 1 = ACK, 0 = NAK 
};