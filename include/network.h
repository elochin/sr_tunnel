#ifndef NETWORK_H
#define NETWORK_H

void send_packet(int sock_fd, struct sockaddr_in sout, char buf[BUFSIZE], int seqnum, ssize_t l, struct mystats *stats);
void timeout_retransmit(int sock_fd, struct sockaddr_in sout, int timeout, linked_list_PDU * p_pkt_list, int seqnum, struct mystats *stats);
void send_ack(int sock_fd, struct sockaddr_in sout, int acknum, struct mystats *stats);

#endif
