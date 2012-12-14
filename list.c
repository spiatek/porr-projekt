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
	int cost;
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

    void (*add) (struct List*, int); // add item to tail
    void (*remove) (struct List*, int);
    int (*getNext) (struct List*);
	int (*getPrev) (struct List*);
	int (*getCurr) (struct List*);
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
void add (List* list, int item, int cost);
void remove (List* list, int item);
int getNext (List* list);
int getPrev (List* list);
int getCurr (List* list);
int getCurrValue (List* list);
int setCurrTo (List* list, int item);
void setCurrToHead (List* list);
void clear (List* list);
int isEmpty (List* list);



/**
 * Push an item into List, if this is the first item,
 * both List->head and List->tail will point to it,
 * otherwise the oldtail->next and tail will point to it.
 */
void add (List* list, int item, int cost) {
    // Create a new node
    NodeL* n = (NodeL*) malloc (sizeof(NodeL));
    n->item = item;
	n->cost = cost;
    n->next = NULL;
	n->prev = NULL:

    if (list->head == NULL) { // no head
        list->head = n;
		list->head->prev = n;
		curr = head;
    } else{
		NodeL* tmp = list->head->prev;
		n->prev = tmp;
		tmp->next = n;
		
		list->head->prev = n;
		curr = n;
    }
}

void remove (List* list, int item) {
    // Remove the node
	int result = 0;
	NodeL* n = NULL;
	result = setCurrTo(list, item);

	if (curr == head)
	{
		free(head);
		head = NULL;
		curr = NULL;
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
 * Return and remove the first item.
 */
int getNext (List* list) {
    
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

int getPrev (List* list) {
    
	int item = list->curr->item;
	
	if (list->curr == NULL)
		return -1;

	
	if (list->curr->prev == list->head)
	{
		return -1;
	}
	else
	{
		curr = curr->prev;
	}
	return item;
}



int getCurr (List* list) {

	if (list->curr == NULL)
		return -1;
	else
		return list->curr->item;
}

int getCurrValue (List* list) {

	if (list->curr == NULL)
		return -1;
	else
		return list->curr->value;
}

int setCurrTo (List* list, int item)
{
    list->setCurrToHead(list);

	if (list->curr == NULL)
		return -1;

	while (list->curr->next != null)
	{
		if (list->curr->item == item)
		{
			return list->curr->item;
		}
		list->curr = list->curr->next;
	}

	return -1;
}

void setCurrToHead (List* list)
{
    list->curr = list->head;
}

void clear (List* list)
{
	NodeL* tmp = NULL;
    list->setCurrToHead(list);

	if (list->curr == NULL)
		return;

	while (list->curr->next != null)
	{
		tmp = list->curr->next;
		free(curr);
		curr = tmp;
	}
}


int isEmpty(List* list)
{
	if (list->head == NULL)
		return 1;
	else
		return 0;
}

/**
 * Return but not remove the first item.
 */
/**


* Create and initiate a List
 */
List createList () {
    List list;
    list.head = NULL;
	list.curr = NULL;
    list.add = &add;
    list.remove = &remove;
    list.getNext = &getNext;
    list.getPrev = &getPrev;
	list.getCurr = &getCurr;
	list.getCurrValue = &getCurrValue;
	list.setCurrTo = &setCurrTo;
	list.setCurrToHead = &setCurrToHead;
	list.clear = &clear;
	list.isEmpty = &isEmpty;
    return list;
}