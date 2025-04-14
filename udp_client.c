#include "headsock.h"

void tv_sub(struct timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) < 0) {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int data_unit_size;
    FILE *fp;
    long total_bytes = 0;
    int seq = 0;
    int n;
    
    if(argc != 3) {
        printf("Arg: <server_host> <data_unit_size>\n");
        exit(1);
    }
    
    data_unit_size = atoi(argv[2]);
    
    if((fp = fopen ("myfile.txt","r+t")) == NULL) {
		printf("File doesn't exit\n");
		exit(1);
	}

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        printf("Error in socket!");
        exit(1);
    }
    
    // Get hostname
	if ((server=gethostbyname(argv[1]))==NULL) {            
		printf("Error getting hostname, try using <localhost>");
		exit(1);
	}
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MYUDP_PORT);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    struct data_pkt pkt;
    struct ack_pkt ack;
    int bytes_read;
    
    struct timeval start, end, diff;
    gettimeofday(&start, NULL);
    
    // Send data per data_unit_size
    while ((bytes_read = fread(pkt.data, 1, data_unit_size, fp)) > 0) {
        pkt.num = seq;
        pkt.len = bytes_read;
        
        // Stop and wait ARQ
        while (1) {
            n = sendto(sockfd, &pkt, sizeof(int)*2 + bytes_read, 0,
                       (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if (n < 0) {
                printf("Fatal error detected, exiting.");
                exit(1);
            }
            // Wait for ACK
            n = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
            if (n < 0) {
                // Retransmit packet if not receive ACK
                printf("Error waiting for ACK for seq %d. Resending packet..\n", seq);
                continue;
            } else {
                // If correct ACK and flag is 1
                if(ack.num == seq && ack.flag == 1) {
                    // Good ACK, break out loop
                    total_bytes += bytes_read;
                    printf("Received ACK for packet %d (%d bytes)\n", seq, bytes_read);
                    seq++;
                    break;
                } else {
                    // Resend it, loop again
                    printf("NAK or wrong ACK for seq %d. Resending packet..\n", seq);
                }
            }
        }
    }
    
    // Send termination packet once done (same logic)    
    pkt.num = seq; 
    pkt.len = 0;
    while(1) {
        n = sendto(sockfd, &pkt, sizeof(int)*2, 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if(n < 0) {
            printf("Fatal error detected, exiting.");
            exit(1);
        }
        n = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
        if(n < 0) {
            printf("Error in termination ACK. Resending termination packet");
            continue;
        } else {
            if(ack.num == seq && ack.flag == 1) { // Process completed
                printf("Received termination ACK.\n");
                break;
            } else {
                printf("NAK or wrong termination ACK. Resending termination packet\n");
            }
        }
    }
    
    gettimeofday(&end, NULL);
    tv_sub(&end, &start);
    diff = end;
    double elapsed_ms = diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;
    double throughput = (total_bytes / 1000.0) / (elapsed_ms / 1000.0); // Kbytes/s
    
    printf("Transfer complete: %ld bytes sent.\n", total_bytes);
    printf("Total transfer time: %.3f ms, Throughput: %.3f Kbytes/s\n", elapsed_ms, throughput);
    
    fclose(fp);
    close(sockfd);
    return 0;
}
