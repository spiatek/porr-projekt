#ifndef _LIST_C
#define _LIST_C

#include <stdio.h>
#include <stdlib.h>
/**
  * This sample is about how to implement a list in c
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
typedef struct NodeL {
    int item;
	int value;
    struct NodeL* next;
	struct NodeL* prev;
} NodeL;
/**
 * The List struct, contains the pointers that
 * point to first node and last node, the size of the List,
 * and the function pointers.
 */
typedef struct List {
    NodeL* head;
	NodeL* curr;

    void (*add) (struct List*, int, int); // add item to tail
    void (*remove_) (struct List*, int);
    int (*getNext) (struct List*);
	int (*getPrev) (struct List*);
	int (*getCurr) (struct List*);
	int (*getCurrValue) (struct List*);
	int (*setCurrTo) (struct List*, int);
	void (*setCurrToHead) (struct List*);
	void (*clear) (struct List*);
	int (*isEmpty) (struct List*);

} List;
/**
 * Push an item into list, if this is the first item,
 * both list->head and list->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
/*void _add (List* list, int item, int cost);
void _remove_ (List* list, int item);
int _getNext (List* list);
int _getPrev (List* list);
int _getCurr (List* list);
int _getCurrValue (List* list);
int _setCurrTo (List* list, int item);
void _setCurrToHead (List* list);
void _clear (List* list);
int _isEmpty (List* list);
*/


/**
 * Push an item into List, if this is the first item,
 * both List->head and List->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
void _add (List* list, int item, int cost) {
    // Create a new node
    NodeL* n = (NodeL*) malloc (sizeof(NodeL));
    n->item = item;
	n->value = cost;
    n->next = NULL;
	n->prev = NULL;

    if (list->head == NULL) { // no head
        list->head = n;
		list->head->prev = n;
		list->curr = list->head;
    } else{
		NodeL* tmp = list->head->prev;
		n->prev = tmp;
		tmp->next = n;
		
		list->head->prev = n;
		list->curr = n;
    }
}

void _remove_ (List* list, int item) {
    // Remove the node
	int result = 0;
	NodeL* n = NULL;
	result = list->setCurrTo(list, item);

	if (list->curr == list->head)
	{
		free(list->head);
		list->head = NULL;
		list->curr = NULL;
		return;
	}

	if (result <= 0)
	{
		list->curr->prev->next = list->curr->next;
		list->curr->next->prev = list->curr->prev;
		n = list->curr;
		list->curr = n->prev;
		free(n);
	}
}
/**
 * Return and remove_ the first item.
 */
int _getNext (List* list) {
    
	int item;
	
	if (list->curr == NULL)
		return -1;

	item = list->curr->item;
	
	if (list->curr->next == NULL)
	{
		return -1;
	}
	else
	{
		list->curr = list->curr->next;
	}
	return item;
}

int _getPrev (List* list) {
    
	int item = list->curr->item;
	
	if (list->curr == NULL)
		return -1;

	
	if (list->curr->prev == list->head)
	{
		return -1;
	}
	else
	{
		list->curr = list->curr->prev;
	}
	return item;
}



int _getCurr (List* list) {

	if (list->curr == NULL)
		return -1;
	else
		return list->curr->item;
}

int _getCurrValue (List* list) {

	if (list->curr == NULL)
		return -1;
	else
		return list->curr->value;
}

int _setCurrTo (List* list, int item)
{
    list->setCurrToHead(list);

	if (list->curr == NULL)
		return -1;

	while (list->curr->next != NULL)
	{
		if (list->curr->item == item)
		{
			return list->curr->item;
		}
		list->curr = list->curr->next;
	}

	return -1;
}

void _setCurrToHead (List* list)
{
    list->curr = list->head;
}

void _clear (List* list)
{
	NodeL* tmp = NULL;
    list->setCurrToHead(list);

	if (list->curr == NULL)
		return;

	while (list->curr->next != NULL)
	{
		tmp = list->curr->next;
		free(list->curr);
		list->curr = tmp;
	}
}


int _isEmpty(List* list)
{
	if (list->head == NULL)
		return 1;
	else
		return 0;
}

/**
 * Return but not remove_ the first item.
 */
/**


* Create and initiate a List
 */
List createList () {
    List list;
    list.head = NULL;
	list.curr = NULL;
    list.add = &_add;
    list.remove_ = &_remove_;
    list.getNext = &_getNext;
    list.getPrev = &_getPrev;
	list.getCurr = &_getCurr;
	list.getCurrValue = &_getCurrValue;
	list.setCurrTo = &_setCurrTo;
	list.setCurrToHead = &_setCurrToHead;
	list.clear = &_clear;
	list.isEmpty = &_isEmpty;
    return list;
}

#endif
