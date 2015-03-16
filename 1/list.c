#include <list.h>

List *List_create()
{
    return calloc(1, sizeof(List));
}

void List_destroy(List *list)
{
    LIST_FOREACH(list, first, next, cur) {
        if(cur->prev) {
            free(cur->prev);
        }
    }

    free(list->last);
    free(list);
}


void List_clear(List *list)
{
    LIST_FOREACH(list, first, next, cur) {
        free(cur->value);
    }
}


void List_clear_destroy(List *list)
{
    List_clear(list);
    List_destroy(list);
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

    list->count++;

}

struct info *List_pop(List *list)
{
    ListNode *node = list->last;
    return node != NULL ? List_remove(list, node) : NULL;
}

struct info * List_remove(List *list, ListNode *node)
{
    struct info *result = NULL;

    if (list->first==NULL || list ->last==NULL || node == NULL)
      return NULL;

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

    list->count--;
    result = node->value;
    free(node);
}

/*
 * Finding after name
 */
struct info * List_find(List * list, char * name) {
  ListNode * node = list->first;
  while(name!=node->value->firstName && node->next!=NULL && node!=NULL) {
    node = node -> next;
  }
  return node->value;
}

struct info * List_sort(List * list) {
  ListNode * node = list -> first;
}
