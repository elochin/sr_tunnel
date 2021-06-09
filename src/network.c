/**
 * @author Emmanuel Lochin - ENAC
 * @file network.c
 * @brief Network operations for selective Repeat UDP tunnel
 */

#include "util.h"
#include "sr_buffer.h"

void send_packet(int sock_fd, struct sockaddr_in sout, char buf[BUFSIZE],
		 int seqnum, ssize_t l, struct mystats *stats)
{
	struct PDU data_pkt;

	memset(&data_pkt, 0, sizeof(data_pkt));	/* build the packet */
	data_pkt.type = PKT;
	data_pkt.seqnum = seqnum;
	data_pkt.length = l;
	memcpy(data_pkt.data, buf, l);

	if (sendto
	    (sock_fd, &data_pkt, (sizeof(int) * 3) + data_pkt.length, 0,
	     (struct sockaddr *)&sout, sizeof(sout)) < 0)
		PERROR("PKT sendto()");

	if (trace) {
		print_timestamp(fp);
		fprintf(fp,
			"P %s:%i \tseqnum %d \tsize %d \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
			inet_ntoa(sout.sin_addr), ntohs(sout.sin_port), seqnum,
			data_pkt.length, stats->lw, stats->rw, stats->nb_buffer,
			stats->nb_acked, stats->nb_nonacked);
	}
}

void timeout_retransmit(int sock_fd, struct sockaddr_in sout,
			int timeout, linked_list_PDU * p_pkt_list, int seqnum, struct mystats *stats)
{
	struct cell_PDU cell;
	struct timeval now;
	struct PDU data_pkt;

	// GET PKT and return BUF then SEND
	memset(&cell, 0, sizeof(cell));	/* reset get_pkt struct */
	cell = get_packet(p_pkt_list, seqnum);	/* retreive seqnum from the buffer if non-acked */
	gettimeofday(&now, NULL);

	if (cell.seqnum != 0 && cell.acked == false
	    && (now.tv_sec - cell.sent_at.tv_sec) * 1000000L + (now.tv_usec -
								cell.sent_at.
								tv_usec) >
	    timeout * 1000L) {
		memset(&data_pkt, 0, sizeof(data_pkt));
		data_pkt.type = PKT;	// build the retransmission 
		data_pkt.seqnum = cell.seqnum;
		data_pkt.length = cell.length;
		gettimeofday(&now, NULL);
		settime(p_pkt_list, cell.seqnum, now);	// sent time when restransmitted 
		memcpy(data_pkt.data, cell.data, cell.length);

		if (sendto
		    (sock_fd, &data_pkt, (sizeof(int) * 3) + data_pkt.length, 0,
		     (struct sockaddr *)&sout, sizeof(sout)) < 0)
			PERROR("PKT sendto()");

		if (trace) {
			print_timestamp(fp);
			fprintf(fp,
				"T %s:%i \tseqnum %d \tsize %d \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
				inet_ntoa(sout.sin_addr), ntohs(sout.sin_port),
				seqnum, data_pkt.length, stats->lw, stats->rw,
				stats->nb_buffer, stats->nb_acked,
				stats->nb_nonacked);
		}
	}
}

void send_ack(int sock_fd, struct sockaddr_in sout, int acknum, struct mystats *stats)
{
	struct PDU data_pkt;

	memset(&data_pkt, 0, sizeof(data_pkt));
	data_pkt.type = ACK;
	data_pkt.seqnum = acknum;
	data_pkt.length = 0;
	if (sendto
	    (sock_fd, &data_pkt, (sizeof(int) * 3), 0, (struct sockaddr *)&sout,
	     sizeof(sout)) < 0)
		PERROR("ACK sendto()");
	else {
		if (trace) {
			print_timestamp(fp);
			fprintf(fp,
				"A %s:%i \tacknum %d \tsize %2d \tlw %d \trw %d \tnbb %d \tnba %d \tnbna %d\n",
				inet_ntoa(sout.sin_addr), ntohs(sout.sin_port),
				acknum, data_pkt.length, stats->lw, stats->rw,
				stats->nb_buffer, stats->nb_acked,
				stats->nb_nonacked);
		}
	}
}

