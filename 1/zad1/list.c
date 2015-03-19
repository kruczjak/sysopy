#include "list.h"
#include <string.h>


static ListNode * sortList(ListNode * node);


List *List_create()
{
    return calloc(1, sizeof(List));
}

void List_destroy(List *list)
{
    ListNode * node = list->first;
    while(node->next!=NULL) {
      node=node->next;
        if(node->prev) {
            free(node->value);
            free(node->prev);
        }
    }

    free(list->last);
    free(list);
}


void List_push(List *list, struct info *value)
{
    ListNode *node = calloc(1, sizeof(ListNode));

    node->value = value;

    if(list->last == NULL) {
        list->first = node;
        list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

}

struct info List_pop(List *list)
{
    ListNode *node = list->last;
    struct info data;
    if (node != NULL) {
      data = *node->value;
      List_remove(list, node);
      return data;
    } else return data;
}

void List_remove(List *list, ListNode *node)
{

    if(node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if(node == list->first) {
        list->first = node->next;
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = node->prev;
        list->last->next = NULL;
    } else {
        ListNode *after = node->next;
        ListNode *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    free(node->value);
    free(node);
}

/*
 * Finding after name
 */
ListNode * List_find(List * list, char * name) {
  ListNode * node = list->first;
  while(name!=node->value->lastName && node->next!=NULL && node!=NULL) {
    node = node -> next;
  }
  return node;
}

void List_sort(List * list) {
  list->first = sortList(list -> first);
}

static ListNode * sortList(ListNode * node) {

  if(node == NULL || node->next == NULL)
      return node; // the node is sorted.

  //replace largest node with the first :

  //1- find largest node :
  ListNode *curr, *largest, *prev, *largestPrev;
  curr = node;
  largest = node;
  prev = node;
  largestPrev = node;
  while(curr != NULL) {
          if(strcmp(curr->value->lastName,largest->value->lastName) > 0 ) {
              largestPrev = prev;
              largest = curr;
          }
          prev = curr;
          curr = curr->next;

      }
  //largest node is in largest.

  //2- switching firt node and largest node :
  ListNode * tmp;
  if(largest != node)
  {
      largestPrev->next = node;
      tmp = node->next;
      node->next = largest->next;
      largest->next = tmp;
  }

  // now largest is the first node of the node.

  // calling the function again with the sub node :
  //            node minus its first node :
  largest->next = sortList(largest->next);
  return largest;
}
