/****************************************************************************\
*
*	List Container
*
*	Self Comment: I should use 3rd party library instead
*
*   COPYRIGHT:  2008 Suyandi Wu
*
\****************************************************************************/

#ifndef LIST_H_SDIWU_2007
#define LIST_H_SDIWU_2007

#ifdef __cplusplus
extern "C" {
#endif

/*
    Sample code showing how to iterate List:

        List list;
        // ...
        Node* node;
        for (node = list.begin; node != NULL; node = node->next) {
            // ...
        }
*/

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

void List_Init (List* list);
#define LIST_INIT(list)\
	(list)->begin = NULL;\
	(list)->end = NULL;\
	(list)->count = 0;

int List_Empty (List* list);
int List_PushFront (List* list, void* data);
int List_PushBack (List* list, void* data);
void List_PopFront (List* list, int freeData);
void List_PopBack (List* list, int freeData);
void* List_GetFront (List* list);
void* List_GetBack (List* list);
void List_Clear (List* list, int freeData);
int List_Insert (List* list, void* data, Node* nextNode);
int List_SortedInsert (List* list, void* data, int (*pfnBinPred)(void*, void*));
void List_Erase (List* list, Node* node, int freeData);
void List_Remove (List* list, void* data, int (*pfnBinPred)(void*, void*), int freeData);

#ifdef __cplusplus
}
#endif

#endif