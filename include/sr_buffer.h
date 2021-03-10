#ifndef LINKED_LIST_H
#define LINKED_LIST_H


#include "util.h"

/**
 * @brief An alias for the structure representing the cells
 */
typedef struct cell_PDU cell_PDU;

/**
 * @brief The structure representing the cells of the list
 */
struct cell_PDU {
    int seqnum;
    int length;
    bool acked;
    struct timeval sent_at;
    char data[BUFSIZE];
    /** the next cell in the list */
    cell_PDU *next;
};

/**
 * @brief An alias for the structure representing the linked list
 */
typedef cell_PDU *linked_list_PDU;

/**
 * @brief Creates an empty list
 *
 * @param p_list  a pointer to the list to be set to nil
 *
 * @post After the call, p_list points to a NULL cell.
 */
void nil(linked_list_PDU *p_list);

/**
 * @brief Append an element at the beginning of a list
 *
 * @param p_list  a pointer to the list to be modified
 * @param buf		a buffer containing the full data packet captured from the TUN interface
 * @param seqnum	the sequence number of the packet enqueued	
 * @param length	the length of a buffer containing the full data packet captured from the TUN interface
 *
 * @post The first cell of the list is now a new cell
 *       containing the value passed as parameter. The
 *       other cells of the list are not modified and the
 *       new first cell has the previously first cell for
 *       next cell.
 */
void enqueue_packet(linked_list_PDU *p_pkt_list, char buf[BUFSIZE], int seqnum, int length);

/**
 * @brief The length of the buffer list
 *
 * @param p_list	a pointer to the list identifying the buffer
 *
 * @return		the length of the list (i.e. the number of distinct cells in the list)
 */
int list_length(linked_list_PDU *p_list);


/**
 * @brief Buffer statistics. Allows to get the number of acked and unacked packets stored. These stats are stored in the struct stats.
 *
 * @param p_list	a pointer to the list identifying the buffer
 *
 * @post		struct stats contains the statistics computed
 */
void stats_buffer(linked_list_PDU *p_list, struct mystats *stats);

/**
 * @brief get an element from the buffer, for retransmission purpose for instance
 *
 * @param p_list	a pointer to the list to be modified
 * @param seqnum	the seqnum to get
 *
 * @return		the element if found
 */
cell_PDU get_packet(linked_list_PDU *p_list, int seqnum);

void settime(linked_list_PDU *p_list, int seqnum, struct timeval now);

/**
 * @brief Mark a packet as acked in the buffer.
 *
 * @param p_list	a pointer to the list identifying the buffer
 * @param seqnum	the seqnum to set as acked
 *
 * @post		the acked field of the cell is set to true	
 */
void setacked(linked_list_PDU *p_list, int seqnum);

/**
 * @brief Pop an element from the buffer because acked. If the acked field of the element is false, the element is not popped
 *
 * @param p_list	a pointer to the list to be modified
 * @param seqnum	the seqnum to suppress/pop from the buffer
 *
 * @return		0 if popped 1 if not
 */
int pop_packet(linked_list_PDU *p_list, int seqnum);

int clean_buffer(linked_list_PDU * p_pkt_list, int left_win, int right_win, struct mystats *stats);

/**
 * @brief Deallocate a list
 *
 * @param p_list  a pointer to the list to be deallocated
 *
 * @post After the call to `deallocate_buffer`, all cells composing `list`
 *       are deallocated AND the structure representing the list is also
 *       deallocated. `p_list` is `NULL` after the call.
 */
void deallocate_buffer(linked_list_PDU *p_list);

/**
 * @brief Print a list
 *
 * @param p_list  a pointer to the list to be printed
 *
 * @post After the call to `print_list`, the list is printed on the console
 *       in classical format using `[]`.
 */
void print_buffer(linked_list_PDU *p_list);

#endif
