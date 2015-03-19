#include <list.h>

List *List_create()
{
    return calloc(1, sizeof(List));
}

void List_destroy(List *list)
{

    while(list->next!=NULL) {
      list=list->next;
        if(list->prev) {
            free(list->value);
            free(list->prev);
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
    if (node != NULL) {
      struct info data = *node->value;
      List_remove(list, node);
      return data;
    } else return NULL;
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
  ListNode * node = list -> first;
}
