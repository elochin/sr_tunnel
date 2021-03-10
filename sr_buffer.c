/**
 * @author Emmanuel Lochin - ENAC
 * @file sr_buffer.c
 * @brief 0 Selective repeat buffer management
 */

#include "sr_buffer.h"

// functions from the signature
void nil(linked_list_PDU *p_list) {
    *p_list = NULL;
}

void enqueue_packet(linked_list_PDU *p_pkt_list, char buf[BUFSIZE], int seqnum, int length)
{
    cell_PDU *p_new_cell = (cell_PDU *) malloc(sizeof(cell_PDU));

    gettimeofday(&p_new_cell->sent_at, NULL);     
    p_new_cell->seqnum = seqnum;                     /* set pkt_to_buf struct   */
    p_new_cell->length = length;
    p_new_cell->acked = false;
    memcpy (p_new_cell->data, buf, length);
    p_new_cell->next  = *p_pkt_list;
    *p_pkt_list = p_new_cell;
#ifdef DEBUG_LL
    fprintf(fp, "enqueue_packet(%d) size %d acked %d\n", pkt_to_buf.seqnum, pkt_to_buf.length, pkt_to_buf.acked );
#endif
}

int list_length(linked_list_PDU *p_list) {
    int length = 0;
    cell_PDU *p_current_cell = *p_list;

    while (p_current_cell != NULL) {
        p_current_cell = p_current_cell->next;
        length++;
    }

    return length;
}

void stats_buffer(linked_list_PDU *p_list, struct mystats *stats) {
    cell_PDU *p_current_cell = *p_list;

    stats->nb_acked = 0;
    stats->nb_nonacked = 0;
    stats->nb_buffer = 0;

    while (p_current_cell != NULL) {
        if (p_current_cell->acked)
            stats->nb_acked++;
        else
            stats->nb_nonacked++;
        p_current_cell = p_current_cell->next;
        stats->nb_buffer++;
    }
}

cell_PDU get_packet(linked_list_PDU *p_list, int seqnum) {
    cell_PDU *p_current_cell = *p_list;
    cell_PDU tmp = {.seqnum = 0, .length = 0};

    while (p_current_cell != NULL) {
        if (p_current_cell->seqnum != seqnum) {
            p_current_cell = p_current_cell->next;
         } else {
#ifdef DEBUG_LL
            printf("get_packet(%d)\n", p_current_cell->seqnum);
#endif
            return *p_current_cell;
         }
    }
    return tmp;
}

void settime(linked_list_PDU *p_list, int seqnum, struct timeval now) {
    cell_PDU *p_current_cell = *p_list;

    while (p_current_cell != NULL) {
        if (p_current_cell->seqnum != seqnum) {
            p_current_cell = p_current_cell->next;
         } else {
            p_current_cell->sent_at.tv_sec = now.tv_sec;
            p_current_cell->sent_at.tv_usec = now.tv_usec;
#ifdef DEBUG_LL
            printf("settime(%d)\n", p_current_cell->seqnum);
#endif
            break;
         }
    }
}

void setacked(linked_list_PDU *p_list, int seqnum) {
    cell_PDU *p_current_cell = *p_list;

    while (p_current_cell != NULL) {
        if (p_current_cell->seqnum != seqnum) {
            p_current_cell = p_current_cell->next;
         } else {
            p_current_cell->acked = true;
#ifdef DEBUG_LL
            fprintf(fp, "setacked(%d)\n", p_current_cell->seqnum);
#endif
            break;
         }
    }
}

int pop_packet(linked_list_PDU *p_list, int seqnum) {
    cell_PDU *p_current_cell = *p_list;
    cell_PDU *p_previous_cell = NULL;

    if (p_list == NULL) {
        return 0;
    }

    if (list_length(p_list) == 1) {
        if (p_current_cell->acked == true) {
            free(p_current_cell);
            p_current_cell = NULL;
            *p_list = NULL; 
            return 1;
        } else
            return 0;
    }

    while (p_current_cell != NULL) {
        if (p_current_cell->seqnum != seqnum) {
            p_previous_cell = p_current_cell;
            p_current_cell = p_current_cell->next;
         } else {
            break;
         }
    }

    if (p_current_cell != NULL && p_current_cell->acked == true) {

        if (p_previous_cell != NULL) {
            p_previous_cell->next = p_current_cell->next;
            free(p_current_cell);
#ifdef DEBUG_LL
            printf("pop_packet(%d)\n", p_current_cell->seqnum);
#endif
        }
        return 1;
    } else {
#ifdef DEBUG_LL
        printf("pop_packet(%d) failed\n", p_current_cell->seqnum);
#endif
        return 0;
    }
}

int clean_buffer(linked_list_PDU * p_pkt_list, int left_win, int right_win, struct mystats *stats)
{
	while (left_win <= right_win) {
		if (pop_packet(p_pkt_list, left_win)) {	/* pop_packet() with acked flag, returns 1 if popped */
			left_win++;
			stats->lw = left_win;
		} else {	/* else pop_packet() returns 0 */
			break;	/* we stop as this packet is not acked, do not have to check the next */
		}
	}
	return left_win;
}

void deallocate_buffer(linked_list_PDU *p_list) {
    cell_PDU *p_temp_cell    = NULL;
    cell_PDU *p_current_cell = *p_list;

    while (p_current_cell != NULL) {
        p_temp_cell    = p_current_cell;
        p_current_cell = p_current_cell->next;
        free(p_temp_cell);
    }

    nil(p_list);
}

void print_buffer(linked_list_PDU *p_list) {
    cell_PDU *p_current_cell = *p_list;

    fprintf(fp, "[ ");

    while (p_current_cell != NULL) {
        fprintf(fp, "%d %d", p_current_cell->seqnum, p_current_cell->acked);

        if (p_current_cell->next != NULL) {
            fprintf(fp, ", ");
            p_current_cell = p_current_cell->next;
        } else {
            fprintf(fp, " ");
            break;
        }
    }

    fprintf(fp, "]\n");
}
