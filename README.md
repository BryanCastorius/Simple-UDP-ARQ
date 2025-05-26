# Simple UDP with ARQ Implementation in C

My program implements a UDP-based file transfer between a client and a server using
stop-and-wait ARQ. The server creates a socket (family AF_INET, type SOCK_DGRAM,
protocol 0), binds it to the server’s address using the bind() function, and waits to receive
packets using recvfrom(). Upon receiving a packet, it simulates transmission errors based on a
given probability by the user. If an error is simulated, it replies with a NAK (flag == 0); otherwise,
it writes the data to a file and sends an ACK (flag == 1), and the amount of actual error is
tracked. If a termination packet (len == 0) is received, it sends a final ACK and ends the
session, printing a success message and the actual error rate.

On the client side, a socket is created the same way as the server, and the file (myfile.txt) is
read in chunks of size based on the user inputted data unit size. Each chunk is sent to the
server using sendto(). The client then waits for a response using recvfrom(). If a valid ACK is
received, it proceeds to the next packet; if not, it resends the current one. After all data is sent,
the client transmits a termination packet and waits for its acknowledgment. Finally, transfer
statistics such as elapsed time and throughput are computed at the end.
