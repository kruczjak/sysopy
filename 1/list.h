#ifndef List_h
#define List_h

#include <stdlib.h>

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    struct info *value;
} ListNode;

typedef struct List {
    ListNode *first;
    ListNode *last;
} List;

struct info {
  const char * firstName;
  const char * lastName;
  const char * birthDate;
  const char * email;
  int phone;
  const char * address;
};

/**
 * Tworzenie listy
 */
List * List_create();
/**
 * Usuwanie listy
 */
void List_destroy(List *list);
/**
 * Dodawanie elementu na koniec
 */
void List_push(List *list, struct info *value);
/**
 * Usunięcie elementu z końca
 */
struct info *List_pop(List *list);
/**
 * Wyszukiwanie po nazwisku
 */
ListNode *List_find(List * list, char * name);
/**
 * Sortowanie po imieniu
 */
void List_sort(List * list);
/**
 * Usunięcie określonego elementu
 */
void List_remove(List *list, ListNode *node);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
