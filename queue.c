#ifndef _QUEUE_C
#define _QUEUE_C

#include <stdio.h>
#include <stdlib.h>
/**
  * This sample is about how to implement a queue in c
  *
  * Type of item is int
  * Add item to tail
  * Get item from head
  * Can get the size
  * Can display all content
  */
/**
 * The Node struct,
 * contains item and the pointer that point to next node.
 */
typedef struct Node {
    int item;
    struct Node* next;
} Node;
/**
 * The Queue struct, contains the pointers that
 * point to first node and last node, the size of the Queue,
 * and the function pointers.
 */
typedef struct Queue {
    Node* head;
    Node* tail;

    void (*push) (struct Queue*, int); // add item to tail
    // get item from head and remove it from queue
    int (*pop) (struct Queue*);
    // get item from head but keep it in queue
    int (*peek) (struct Queue*);
    // display all element in queue
    void (*display) (struct Queue*);

	void (*queue_clear) (struct Queue*);
	
	void (*queue_remove) (struct Queue*, int);
	
    // size of this queue
    int size;
} Queue;
/**
 * Push an item into queue, if this is the first item,
 * both queue->head and queue->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
void _push (Queue* queue, int item);
/**
 * Return and remove the first item.
 */
int _pop (Queue* queue);
/**
 * Return but not remove the first item.
 */
int _peek (Queue* queue);
/**
 * Show all items in queue.
 */
void _display (Queue* queue);

void _queue_clear (Queue* queue);

void _queue_remove (Queue* queue, int item);
/**
 * Create and initiate a Queue
 */

/*
Queue createQueue ();

int main () {
    Queue queue = createQueue();
    queue.display(&queue);

    printf("push item 2\n");
    queue.push(&queue, 2);    
    printf("push item 3\n");
    queue.push(&queue, 3);
    printf("push item 6\n");
    queue.push(&queue, 6);

    queue.display(&queue);

    printf("peek item %d\n", queue.peek(&queue));
    queue.display(&queue);

    printf("pop item %d\n", queue.pop(&queue));
    printf("pop item %d\n", queue.pop(&queue));
    queue.display(&queue);

    printf("pop item %d\n", queue.pop(&queue));
    queue.display(&queue);
    printf("push item 6\n");
    queue.push(&queue, 6);
/
    queue.display(&queue);
    system("PAUSE");
}

*/

/**
 * Push an item into queue, if this is the first item,
 * both queue->head and queue->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
void _push (Queue* queue, int item) {
    // Create a new node
    Node* n = (Node*) malloc (sizeof(Node));
    n->item = item;
    n->next = NULL;

    if (queue->head == NULL) 
    { // no head
        queue->head = n;
    } 
    else
    {
        queue->tail->next = n;
    }
    queue->tail = n;
    queue->size++;
}


/**
 * Return and remove the first item.
 */
int _pop (Queue* queue) {
    // get the first item
    Node* head = queue->head;
    if (head == NULL)
    	return -1;
    
    int item = head->item;
    // move head pointer to next node, decrease size
    queue->head = head->next;
    queue->size--;
    // free the memory of original head
    free(head);
    return item;
}
/**
 * Return but not remove the first item.
 */
int _peek (Queue* queue) {
    Node* head = queue->head;
    return head->item;
}
/**
 * Show all items in queue.
 */
void _display (Queue* queue) {
    printf("\nDisplay: ");
    // no item
    if (queue->size == 0)
        printf("No item in queue.\n");
    else { // has item(s)
        Node* head = queue->head;
        int i, size = queue->size;
        printf("%d item(s):\n", queue->size);
        for (i = 0; i < size; i++) {
            if (i > 0)
                printf(", ");
            printf("%d", head->item);
            head = head->next;
        }
    }
    printf("\n\n");
}

void _queue_clear (Queue* queue)
{
	Node* tmp = NULL;
    Node* curr = queue->head;

    if (curr == NULL)
    {
    	return;
    }

	while (curr->next != NULL)
	{
		tmp = curr->next;
		free(curr);
		curr = tmp;
	}
}

void _queue_remove (Queue* queue, int item)
{
	Node* tmp = NULL;
    Node* curr = queue->head;
    Node* prev = curr;
  
    if (prev == NULL)
    {
    	return;
    }
    if (prev->item == item)
    {
    	queue->head = prev->next;
    	queue->size--;
    	free(prev);
    	
    	return;
    }

	while (curr->next != NULL)
	{
		prev = curr;
		curr = curr->next;
		if (curr->item == item)
		{
			//printf("remove item=%d from queue\n", item);
			queue->size--;
			tmp = curr;
			curr = curr->next;
			prev->next=curr;
			free(tmp);
			if (curr == NULL)
			{
				queue->tail = prev;
				return;
			}
			return;
		}
	}
}


/**
 * Create and initiate a Queue
 */
Queue createQueue () {
    Queue queue;
    queue.size = 0;
    queue.head = NULL;
    queue.tail = NULL;
    queue.push = &_push;
    queue.pop = &_pop;
    queue.peek = &_peek;
    queue.display = &_display;
	queue.queue_clear = &_queue_clear;
	queue.queue_remove = &_queue_remove;
    return queue;
}

#endif
