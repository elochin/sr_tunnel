/**
 * @author Emmanuel Lochin - ENAC
 * @file test_buffer.c
 * @brief Buffer management tests
 */

#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <sys/time.h>
#include "sr_buffer.h"

FILE *fp;
bool trace;
int sock_fd;

int main() {
    fp=stderr;
    linked_list_PDU test_list;
    linked_list_PDU *p_test_list = &test_list;
    nil(p_test_list);

    printf("\nTests begin\n\n");
    printf("empty list: \n");
    print_buffer(p_test_list);

    cell_PDU mycell, tmp;
    
    char* string="ABCDEFGHIJKL";

    for (int i = 0; i < 4; i++) {
        memset(&mycell, 0, sizeof(mycell));
        mycell.seqnum=i+1;
        mycell.length=12-i;
        memcpy(mycell.data, string, mycell.length);
	enqueue_packet(p_test_list, mycell.data, mycell.seqnum, mycell.length);
    }
    
    printf("\nget element 1\n");
    tmp = get_packet(p_test_list, 1);
    if (tmp.seqnum > 0)
        //printf("get %d %d %s %s\n", tmp.seqnum, tmp.length, tmp.sent_at, tmp.data);
        printf("get %d %d %s\n", tmp.seqnum, tmp.length, tmp.data);
    else
        printf("no element\n");
   
    printf("\nget element 42\n");
    memset(&tmp, 0, sizeof(tmp));     /* reset struct */
    tmp = get_packet(p_test_list, 42);
    if (tmp.seqnum == 42)
        //printf("get %d %d %s %s\n", tmp.seqnum, tmp.length, tmp.sent_at, tmp.data);
        printf("get %d %d %s\n", tmp.seqnum, tmp.length, tmp.data);
    else
        printf("no element\n");

    printf("\npop element 1\n");
    pop_packet(p_test_list, 1);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));
    
    printf("\nack element 1\n");
    setacked(p_test_list, 1);

    printf("\npop element 1\n");
    pop_packet(p_test_list, 1);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));
    
    printf("\npop element 2\n");
    pop_packet(p_test_list, 2);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));
    
    printf("\nack element 2\n");
    setacked(p_test_list, 2);
    
    printf("\npop element 2 a second time\n");
    pop_packet(p_test_list, 2);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));
    
    printf("\npop non existant element 48\n");
    pop_packet(p_test_list, 48);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));
    
    
    printf("\nget element 2\n");
    tmp = get_packet(p_test_list, 2);
    if (tmp.seqnum > 0)
        printf("get %d %d %s\n", tmp.seqnum, tmp.length, tmp.data);
    else
        printf("no element\n"); 

    printf("\nget non existant element 48\n");
    tmp = get_packet(p_test_list, 48);
    if (tmp.seqnum > 0)
        printf("get %d %d %s\n", tmp.seqnum, tmp.length, tmp.data);
    else
        printf("no element\n");
    
    printf("\npop element 1\n");
    pop_packet(p_test_list, 1);
    print_buffer(p_test_list);
    printf("\nlist length: %d\n", list_length(p_test_list));

    printf("\n");
    printf("pop element 3\n");
    pop_packet(p_test_list, 3);
    print_buffer(p_test_list);
    printf("\n");
    printf("list length: %d\n", list_length(p_test_list));

    printf("\n");
    printf("pop element 4\n");
    pop_packet(p_test_list, 4);
    print_buffer(p_test_list);
    printf("\n");
    printf("list length: %d\n", list_length(p_test_list));
    
    printf("\n");
    printf("pop element 1 second time\n");
    pop_packet(p_test_list, 1);
    print_buffer(p_test_list);
    printf("\n");
    printf("list length: %d\n", list_length(p_test_list));
   
    printf("\nRebuild the list\n");
    for (int i = 0; i < 4; i++) {
        memset(&mycell, 0, sizeof(mycell));
        mycell.seqnum=i+1;
        mycell.length=12-i;
        memcpy(mycell.data, string, mycell.length);
	enqueue_packet(p_test_list, mycell.data, mycell.seqnum, mycell.length);
    }

    print_buffer(p_test_list);
    printf("\n");
    printf("list length: %d\n", list_length(p_test_list));

    printf("\nDeallocate list\n");
    deallocate_buffer(p_test_list);
    printf("\nTests over\n");
    
    return 0;
}
