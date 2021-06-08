/**
 * @author Emmanuel Lochin - ENAC
 * @file sr_tunnel.c
 * @brief Selective Repeat UDP tunnel
 */

#include "util.h"
#include "sr_buffer.h"
#include "network.h"

FILE *fp;		/* traces file */
bool trace = false;	/* enable traces */
int sock_fd;		/* network socket */

char *progname;
char connexion[] = "connexion";
struct mystats stats;

#define GOOD_STATE 0
#define BAD_STATE 1

struct GE {
	float p1;
	float p2;
	bool current_state;
};

struct GE ge_var;

extern char *optarg;


void ge_model(linked_list_PDU * p_pkt_list, int sock_fd,
	      struct sockaddr_in sout, char buf[BUFSIZE], int seqnum, ssize_t l)
{
	float randval = (rand() % 1000) / 1000.0;
	if (ge_var.current_state == GOOD_STATE) {
		if (randval <= ge_var.p1) {	/* if true drop packet */
			ge_var.current_state = BAD_STATE;	/* we are in BS */
			if (trace) {
				print_timestamp(fp);
				stats_buffer(p_pkt_list, &stats);
				fprintf(fp,
					"D %s:%i \tseqnum %d \tsize %zd \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
					inet_ntoa(sout.sin_addr),
					ntohs(sout.sin_port), seqnum, l,
					stats.lw, stats.rw, stats.nb_buffer,
					stats.nb_acked, stats.nb_nonacked);
			}
		} else {
			if (trace) {
				stats_buffer(p_pkt_list, &stats);
			}
			send_packet(sock_fd, sout, buf, seqnum, l, &stats);	/* send packet */
		}
	} else {
		if (randval <= ge_var.p2) {	/* if true drop packet */
			if (trace) {	/* we stay in BS */
				print_timestamp(fp);
				stats_buffer(p_pkt_list, &stats);
				fprintf(fp,
					"D %s:%i \tseqnum %d \tsize %zd \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
					inet_ntoa(sout.sin_addr),
					ntohs(sout.sin_port), seqnum, l,
					stats.lw, stats.rw, stats.nb_buffer,
					stats.nb_acked, stats.nb_nonacked);
			}
		} else {
			if (trace) {
				stats_buffer(p_pkt_list, &stats);
			}
			send_packet(sock_fd, sout, buf, seqnum, l, &stats);	/* send packet */
			ge_var.current_state = GOOD_STATE;
		}
	}
}

/*************************************************************
 * usage: prints usage										 *
 *************************************************************/
void usage(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s -i <ifacename> [-s|-c <serverIP>] [-p <port>]\n",
		progname);
	fprintf(stderr, "%s -h\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr,
		"-i <ifacename>: Name of interface to use (mandatory)\n");
	fprintf(stderr,
		"-s|-c <serverIP>: run in server mode (-s), or specify server address (-c <serverIP>) (mandatory)\n");
	fprintf(stderr,
		"-p <port>: port to listen on (if run in server mode) or to connect to (in client mode), default 30001\n");
	fprintf(stderr,
		"-d <#>: emulate link-layer losses by dropping sent packet, expressed in percentage of drop (from 0 to 100). Default is 0\n");
	fprintf(stderr,
		"-b <#>: average burst size. GE model for losses. Default is 1 if -d set\n");
	fprintf(stderr,
		"-t <file.out>: enable trace file. Default is stdout\n");
	fprintf(stderr, "-h: prints this help text\n");
	exit(1);
}

int main(int argc, char *argv[])
{

	int tun_fd, option;
	int flags = IFF_TUN;
	char if_name[IFNAMSIZ] = "";
	int maxfd;
	char buf[BUFSIZE];
	char remote_ip[16] = "";	/* dotted quad IP string */
	unsigned short int port = 30001;
	int cliserv = -1;	/* must be specified on cmd line */
	int plr = 0;
	int avg_burst = 1;
	char *filename;
	struct sockaddr_in sin, from, sout;
	socklen_t fromlen, soutlen, l;

	fromlen = sizeof(from);	/* to get correct IP */
	soutlen = sizeof(sout);

	fd_set fdset;

	progname = argv[0];

	/* Check command line options */
	while ((option = getopt(argc, argv, "i:sc:p:d:b:t:h")) > 0) {
		switch (option) {
		case 'h':
			usage();
			break;
		case 'i':
			strncpy(if_name, optarg, IFNAMSIZ - 1);
			break;
		case 's':
			cliserv = SERVER;
			break;
		case 'c':
			cliserv = CLIENT;
			strncpy(remote_ip, optarg, 15);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'd':
			plr = atoi(optarg);
			break;
		case 'b':
			avg_burst = atoi(optarg);
			break;
		case 't':
			filename = optarg;
			trace = true;
			if (!strcmp(optarg, "stdout")) {
				fp = stdout;
			} else {
				fp = fopen(filename, "w");
				if (fp == NULL) {
					fprintf(stderr, "Cannot open file\n");
					exit(0);
				}
			}
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", option);
			usage();
		}
	}

	/* GE model */
	ge_var.current_state = GOOD_STATE;

	if (plr != 0) {
		ge_var.p2 = 1.0 - (1.0 / avg_burst);
		ge_var.p1 = (plr / 100.0) / (avg_burst * (1 - (plr / 100.0)));
		printf("plr: %f p1: %f p2: %f burst: %d\n", (plr / 100.0),
		       ge_var.p1, ge_var.p2, avg_burst);
	}

	manage_signal();

	argv += optind;
	argc -= optind;

	if (argc > 0) {
		fprintf(stderr, "Too many options!\n");
		usage();
	}

	if (*if_name == '\0') {
		fprintf(stderr, "Must specify interface name!\n");
		usage();
	} else if (cliserv < 0) {
		fprintf(stderr, "Must specify client or server mode!\n");
		usage();
	} else if ((cliserv == CLIENT) && (*remote_ip == '\0')) {
		fprintf(stderr, "Must specify server address!\n");
		usage();
	}

	/* tun init */
	if ((tun_fd = tun_alloc(if_name, flags | IFF_NO_PI)) < 0) {
		fprintf(stderr, "Error connecting to tun interface %s!\n",
			if_name);
		exit(1);
	}

	fprintf(stderr, "Successfully connected to interface %s\n", if_name);

	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		PERROR("socket()");
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);
	if (bind(sock_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		PERROR("bind()");

	//struct timeval read_timeout;
	//read_timeout.tv_sec = 0;
	//read_timeout.tv_usec = TIMEOUT * 1000L;
	//setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

	if (cliserv == SERVER) {
		while (1) {
			l = recvfrom(sock_fd, buf, sizeof(buf), 0,
				     (struct sockaddr *)&from, &fromlen);
			if (l < 0)
				PERROR("recvfrom()");
			if (strcmp(connexion, buf) == 0) {
				printf("Connexion depuis %s:%i demandée\n",
				       inet_ntoa(from.sin_addr),
				       ntohs(from.sin_port));
				break;
			}
		}
		l = sendto(sock_fd, connexion, sizeof(connexion), 0,
			   (struct sockaddr *)&from, sizeof(from));
		if (l < 0)
			PERROR("sendto()");
	} else {
		from.sin_family = AF_INET;
		from.sin_port = htons(port);
		inet_aton(remote_ip, &from.sin_addr);
		l = sendto(sock_fd, connexion, sizeof(connexion), 0,
			   (struct sockaddr *)&from, sizeof(from));
		if (l < 0)
			PERROR("sendto()");
		printf("Demande de connexion à %s:%i\n",
		       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
		l = recvfrom(sock_fd, buf, sizeof(buf), 0,
			     (struct sockaddr *)&from, &fromlen);
		if (l < 0)
			PERROR("recvfrom()");
		//TODO CHECK TIMEOUT
	}
	printf("Connection established with %s:%i\n", inet_ntoa(from.sin_addr),
	       ntohs(from.sin_port));

	int seqnum = 1;
	int left_win = 1;
	int right_win = 1;

	stats.lw = left_win;
	stats.rw = right_win;

	struct PDU data_pkt;

	/* Init linked list to store set but non acknowledged packets */
	linked_list_PDU pkt_list;
	linked_list_PDU *p_pkt_list = &pkt_list;
	nil(p_pkt_list);

	bool window_not_full = true;

	while (1) {

		maxfd = (tun_fd > sock_fd) ? tun_fd : sock_fd;	/* use select() to handle both TUN and SOCKET descriptors at once */
		FD_ZERO(&fdset);
		FD_SET(tun_fd, &fdset);
		FD_SET(sock_fd, &fdset);

		int ret_select = select(maxfd + 1, &fdset, NULL, NULL, NULL);	//&read_timeout);

		if (ret_select < 0)
			PERROR("select()");

		if (FD_ISSET(tun_fd, &fdset)) {	/* There is something to read on tun_fd */

			if (right_win < left_win + MAXWIN) {	/* if we did not send the full window */

				l = read(tun_fd, buf, sizeof(buf));
				if (l < 0)
					PERROR("read()");

				if (plr) {
					ge_model(p_pkt_list, sock_fd, from, buf,
						 seqnum, l);
				} else {
					if (trace) {
						stats_buffer(p_pkt_list,
							     &stats);
					}
					send_packet(sock_fd, from, buf, seqnum, l, &stats);	/* send packet */
				}

				enqueue_packet(p_pkt_list, buf, seqnum, l);	/* add packet to the sending buffer */
				seqnum++;	/* we increment the next seqnum to buffer */
				right_win++;	/* we increment right_win by one each packet sent */
				stats.rw = right_win;
			}

			if (right_win == left_win + MAXWIN) {	/* window full : retransmit non-acked packets in timeout */
				window_not_full = false;

				for (int i = left_win; i < right_win; i++) {
					if (trace) {
						stats_buffer(p_pkt_list, &stats);
					}
					timeout_retransmit(sock_fd, from,
							   p_pkt_list, i, &stats);
					left_win = clean_buffer(p_pkt_list, left_win, right_win, &stats);	/* remove all consecutive acked packets from the sending buffer */
					stats.lw = left_win;
				}
			}
		}

		if (FD_ISSET(sock_fd, &fdset)) {	/* There is a packet received on sock_fd */
			memset(&data_pkt, 0, sizeof(data_pkt));

			l = recvfrom(sock_fd, &data_pkt, sizeof(data_pkt), 0,
				     (struct sockaddr *)&sout, &soutlen);

			if (l < 0) {
				PERROR("recvfrom()");
			}

			if (data_pkt.type == PKT) {	/* if a data packet is received */
				if (trace) {
					stats_buffer(p_pkt_list, &stats);
					print_timestamp(fp);
					fprintf(fp,
						"p %s:%i \tseqnum %d \tsize %d \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
						inet_ntoa(sout.sin_addr),
						ntohs(sout.sin_port), data_pkt.seqnum,
						data_pkt.length, stats.lw,
						stats.rw, stats.nb_buffer,
						stats.nb_acked,
						stats.nb_nonacked);
				}

				if (write
				    (tun_fd, &data_pkt.data,
				     data_pkt.length) < 0)
					PERROR("write()");
				else {
					if (trace) {
						stats_buffer(p_pkt_list,
							     &stats);
					}
					send_ack(sock_fd, sout,
						 data_pkt.seqnum, &stats);
				}
			} else if (data_pkt.type == ACK) {	/* if an ACK is received */

				if (trace) {
					stats_buffer(p_pkt_list, &stats);
					print_timestamp(fp);
					fprintf(fp,
						"a %s:%i \tacknum %d \tsize %d \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
						inet_ntoa(sout.sin_addr),
						ntohs(sout.sin_port), data_pkt.seqnum, l,
						stats.lw, stats.rw,
						stats.nb_buffer, stats.nb_acked,
						stats.nb_nonacked);
				}

				if (data_pkt.seqnum < left_win) {	/* should not occur - ignore this ACK */
					fprintf(stderr,
						"*** WARNING received an ACK %d lower than left_win %d ***\n",
						data_pkt.seqnum, left_win);
				} else if (data_pkt.seqnum > right_win) {	/* should not occur - ERROR */
					fprintf(stderr,
						"*** ERROR received an ACK %d higher than right_win %d ***\n",
						data_pkt.seqnum, left_win);
					exit(1);
				}
				setacked(p_pkt_list, data_pkt.seqnum);	/* mark corresponding packet in the buffer as acked */
				left_win = clean_buffer(p_pkt_list, left_win, right_win, &stats);	/* remove all consecutive acked packets from the sending buffer */
				stats.lw = left_win;
			}
		}		// else FD_ISSET
	}			// while
}
