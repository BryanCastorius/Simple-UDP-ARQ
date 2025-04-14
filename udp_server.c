#include "headsock.h"

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addr_len;
    struct data_pkt pkt;
    struct ack_pkt ack;
    int n;
    FILE *fp;
    float error_probability;
    float err_count = 0;
    float total_count = 0;
    
    if(argc != 3) {
        printf("Arg: <output_filename> <error_probability>\n");
        exit(1);
    }
    
    error_probability = atof(argv[2]);

    fp = fopen(argv[1], "wb");
    if(fp == NULL) {
        printf("Error in writing");
        exit(1);
    }
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        printf("Error in socket!");
        exit(1);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MYUDP_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);
    
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("Error in binding");
        exit(1);
    }
    
    printf("Start listening in localhost\n");
    
    addr_len = sizeof(cli_addr);
    int expected_seq = 0;
    // If don't add this, error rate will be constant in all tries of the same inputted rate
    srand(time(NULL)); 
    
    while(1) {
        n = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&cli_addr, &addr_len);
        if(n < 0) {
            printf("Error in receiving packet");
            continue;
        }
        total_count++;
        printf("Packet received: seq=%d, len=%d\n", pkt.num, pkt.len);
        
        // Random val 0 to 999
        int rand_val = rand() % 1000;
        // If our inputted error is greater than the random value, simulate error is true
        int simulate_error = (rand_val < (int)(error_probability * 1000)); 
        
        if(simulate_error) {
            printf("Simulated error for packet %d. Sending NAK.\n", pkt.num);
            ack.num = pkt.num;
            ack.flag = 0;
            err_count++;
            sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&cli_addr, addr_len);
            continue;
        }

        // Sending is done once termination packet is received
        if(pkt.len == 0) {
            printf("Termination packet received.\n");
            ack.num = pkt.num;
            ack.flag = 1;
            sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&cli_addr, addr_len);
            break;
        }

        // Write and also check for error
        if(fwrite(pkt.data, 1, pkt.len, fp) != pkt.len) {
            printf("Error writing to file\n");
            exit(1);
        }
        
        // Send ACK
        ack.num = pkt.num;
        ack.flag = 1;
        sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&cli_addr, addr_len);
        printf("Sent ACK for packet %d\n", pkt.num);
        
        expected_seq++;
    }
    
    printf("File received successfully.\n");
    float error_rate = (err_count/total_count) * 100.0; 
    printf("Error rate: %.2f percent.\n", error_rate);
    fclose(fp);
    close(sockfd);
    return 0;
}