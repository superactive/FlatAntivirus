/****************************************************************************\
*
*	List Container
*
*	Self Comment: I should use 3rd party library instead
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#include <stdlib.h>

/*typedef int (*UNARY_PRED)(void*);*/
/*typedef int (*BINARY_PRED)(void*, void*);*/

typedef struct Node_t
{
	struct Node_t* prev;
	struct Node_t* next;
	void* data;
} Node;

typedef struct
{
	Node* begin;
	Node* end;
	unsigned long count;
} List;

void List_Init (List* list)
{
	list->begin = NULL;
	list->end = NULL;
	list->count = 0;
}

int List_Empty (List* list)
{
	return (list->count == 0);
}

int List_PushFront (List* list, void* data)
{
	Node* node;

	node = (Node*) malloc (sizeof(Node));
	if (node != NULL) {
		node->data = data;
		node->next = list->begin;
		node->prev = NULL;
		
		if (list->begin != NULL)
			list->begin->prev = node;
		list->begin = node;	

		if (list->end == NULL)
			list->end = node;
		++list->count;
		return 1;
	}
	return 0;
}

int List_PushBack (List* list, void* data)
{
	Node* node;

	node = (Node*) malloc (sizeof(Node));
	if (node != NULL) {
		node->data = data;
		node->next = NULL;
		node->prev = list->end;
		
		if (list->end != NULL)
			list->end->next = node;
		list->end = node;

		if (list->begin == NULL)
			list->begin = node;
		++list->count;
		return 1;
	}
	return 0;
}

void List_PopFront (List* list, int freeData)
{
	Node* node;

	node = list->begin;
	list->begin = node->next;
	if (freeData)
		free (node->data);
	free (node);

	if (list->begin == NULL)
		list->end = NULL;
	else
		list->begin->prev = NULL;
	--list->count;
}

void List_PopBack (List* list, int freeData)
{
	Node* node;

	node = list->end;
	list->end = node->prev;
	if (freeData)
		free (node->data);
	free (node);

	if (list->end == NULL)
		list->begin = NULL;
	else
		list->end->next = NULL;
	--list->count;
}

void* List_GetFront (List* list)
{
	return list->begin->data;
}

void* List_GetBack (List* list)
{
	return list->end->data;
}

void List_Clear (List* list, int freeData)
{
	Node* node;

	while (list->begin != NULL) {
		node = list->begin;
		list->begin = list->begin->next;

		if (freeData)
			free (node->data);
		free (node);
	}
	list->end = NULL;
	list->count = 0;
}

/*
	List_Insert
		Insert data before nextNode in a list. 
		If nextNode is null, data will be inserted into the end of the list.
*/
int List_Insert (List* list, void* data, Node* nextNode)
{
	if (nextNode != NULL) {
		Node* n;

		for (n = list->begin; n != NULL; n = n->next) {
			if (n == nextNode) {
				Node* node = (Node*) malloc (sizeof(Node));
				if (node != NULL) {
					node->data = data;
					node->prev = n->prev;
					node->next = n;

					if (n->prev != NULL)
						n->prev->next = node;
					else
						list->begin = node;
					n->prev = node;
					return 1;
				}
				else
					return 0;
			}
		}
		return 0;
	}
	else
		return List_PushBack (list, data);
}

/*
	List_SortedInsert
		Insert data in sorted position in a list using user-defined less-than comparison 
		callback function.
*/
int List_SortedInsert (List* list, void* data, int (*pfnBinPred)(void*, void*))
{
	Node* n;

	for (n = list->begin; n != NULL; n = n->next) {
		if (pfnBinPred  (data, n->data))
			return List_Insert (list, data, n);
	}
	return List_PushBack (list, data);
}

/*
	List_Erase
		Erase data from a node in a list.
		If node is null, the last data will be removed.
*/
void List_Erase (List* list, Node* node, int freeData)
{
	if (node != NULL) {
		Node* n;

		for (n = list->begin; n != NULL; n = n->next) {
			if (n == node) {
				if (n->prev != NULL)
					n->prev->next = n->next;
				else
					list->begin = list->begin->next;

				if (n->next != NULL)
					n->next->prev = n->prev;
				else
					list->end = list->end->prev;

				if (freeData)
					free (n->data);
				free (n);
				return;
			}
		}
	}
	else
		List_PopBack (list, freeData);
}

void List_Remove (List* list, void* data, int (*pfnBinPred)(void*, void*), int freeData)
{
	Node* n;

	for (n = list->begin; n != NULL;) {
		if (pfnBinPred (n->data, data)) {
			Node* node = n;
			n = n->next;

			if (node->prev != NULL)
				node->prev->next = node->next;
			else
				list->begin = list->begin->next;

			if (node->next != NULL)
				node->next->prev = node->prev;
			else
				list->end = list->end->prev;

			if (freeData)
				free (node->data);
			free (node);
		}
		else
			n = n->next;
	}
}
