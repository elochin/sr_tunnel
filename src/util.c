/**
 * @author Emmanuel Lochin - ENAC
 * @file util.c
 * @brief Various functions
 */

#include "util.h"

void quit(int sig)
{
   // deallocate_list(linked_list_PDU *p_list);
   // close(client_socket);
    if (trace)
        fclose(fp);
    close(sock_fd);
    exit(EXIT_SUCCESS);
}

void float_error(int sig)
{
    //close(client_socket);
    fprintf(stderr, "\nFloat error\n");
    exit(EXIT_FAILURE);
}

void print_timestamp(FILE *fp) 
{
       struct timeval tv;
       gettimeofday(&tv, NULL);
       fprintf(fp, "%ld.%ld\t", tv.tv_sec, tv.tv_usec);
}

void manage_signal()
{
    signal(SIGINT, quit);
    signal(SIGTERM, quit);
    signal(SIGKILL, quit);
    signal(SIGQUIT, quit);
    signal(SIGHUP, SIG_IGN);
    signal(SIGFPE, float_error);
}

/*******************************************************/
/* tun_alloc: allocates or reconnects a tun device */
/*******************************************************/
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  if( (fd = open(clonedev , O_RDWR)) < 0 ) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
}
