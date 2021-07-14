#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>

#define PERROR(x) do { perror(x); exit(1); } while (0)
#define BUFSIZE 1500   	/**< buffer for reading from tun interface, must be >= 1500 */
#define CLIENT 0
#define SERVER 1
#define PKT 0x50
#define ACK 0x41
#define MAXWIN 8	/**< maximum SR window */
#define TIMEOUT 500	/**< timeout expressed in ms */

/**
  * Network PDU structure sent over the network
  */
struct PDU {
    char type;		/**< ACK or PKT */
    int seqnum;
    int length;
    char data[BUFSIZE];
};

/**
  * Structure for statistics collection
  */
struct mystats {
    int lw;		/**< current left_win value */
    int rw;		/**< current right_win value */
    int nb_buffer;	/**< number of packets inside the sending buffer */
    int nb_acked;	/**< number of acked packets inside the sending buffer */
    int nb_nonacked;	/**< number of non-acked packets inside the sending buffer */
};

extern FILE *fp;	/**< file descriptor for the traces file */
extern bool trace;	/**< enable the traces */	
extern int sock_fd;	/**< network socket */

void quit(int sig);
void float_error(int sig);
void print_timestamp(FILE *fp);
void manage_signal();
int tun_alloc(char *dev, int flags);

#endif
