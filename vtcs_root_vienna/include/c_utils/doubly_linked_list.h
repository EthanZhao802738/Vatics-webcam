
/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */ 
#ifndef __DOUBLY_LINKED_LIST_H__
#define __DOUBLY_LINKED_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/* Doubly-linked lists. */
typedef struct dnode_s
{
	void *data;		/**< The data of one node in the doubly-linked lists. */
	struct dnode_s *prev;	/**< The pointer to point the previous node in the doubly-linked lists. */
	struct dnode_s *next;	/**< The pointer to point the next node in the doubly-linked lists. */
} dnode_t;

typedef struct dlinked_list
{
	size_t count;		/**< The number of node in the doubly-linked lists. */
	dnode_t *front;		/**< The pointer to point the first node in the doubly-linked lists. */
	dnode_t *rear;		/**< The pointer to point the last node in the doubly-linked lists. */
} dlinked_list_t;

typedef int (*COMP_FUNC_PTR)(const void* data1, const void* data2);

#define DLIST_FOR_EACH(node_ptr, list_ptr) \
	for((node_ptr) = (list_ptr)->front; (node_ptr); (node_ptr) = (node_ptr)->next)

void InitDList(dlinked_list_t *list);
dnode_t* CreateDListNode(void *data);
void* DestroyDListNode(dnode_t **node);
void DListInsertAfter(dlinked_list_t *list, dnode_t *node, dnode_t *new_node);
void DListInsertBefore(dlinked_list_t *list, dnode_t *node, dnode_t *new_node);
void DListPushFront(dlinked_list_t *list, dnode_t *new_node);
void DListPushBack(dlinked_list_t *list, dnode_t *new_node);
void DListRemove(dlinked_list_t *list, dnode_t *node);
void DListPushBackSort(dlinked_list_t *dlist, dnode_t *new_node, COMP_FUNC_PTR compare_func);
void DListSplice(dlinked_list_t *dest_dlist, dlinked_list_t *src_dlist);

#ifdef __cplusplus
}
#endif

#endif
