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

void List_add(List * list, const char * firstName, const char * lastName, const char * birthDate, const char * email, int phone, const char * address) {
  struct info data;
  data.firstName = firstName;
  data.lastName = lastName;
  data.birthDate = birthDate;
  data.email = email;
  data.phone = phone;
  data.address = address;
  List_push(list, &data);
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

  //replace smallest node with the first :

  //1- find smallest node :
  ListNode *curr, *smallest, *prev, *smallestPrev;
  curr = node;
  smallest = node;
  prev = node;
  smallestPrev = node;
  while(curr != NULL) {
          if(strcmp(curr->value->lastName,smallest->value->lastName) < 0 ) {
              smallestPrev = prev;
              smallest = curr;
          }
          prev = curr;
          curr = curr->next;

      }

  //2- switching firt node and smallest node :
  ListNode * tmp;
  if(smallest != node)
  {
      smallestPrev->next = node;
      tmp = node->next;
      node->next = smallest->next;
      smallest->next = tmp;
  }

  // now smallest is the first node of the node.

  // calling the function again with the sub node :
  //            node minus its first node :
  smallest->next = sortList(smallest->next);
  return smallest;
}
