#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <time.h>
#include "list.h"

void (*fList_add)(List*, const char*, const char*, const char*, const char*, int, const char*);
List * (*fList_create)();
void (*fList_destroy)(List*);
void (*fList_remove)(List*, ListNode*);
ListNode* (*fList_search)(List*, const char *);
void (*fList_sort)(List*);

//time
clock_t start_time;
clock_t last_time;
clock_t a_time;
static struct tms start_tms;
static struct tms last_tms;
static struct tms a_tms;

void get_start_time() {
  start_time = last_time = clock();
  times(&start_tms);
  last_tms = start_tms;
}
void print_times() {
  times(&a_tms);
  a_time = clock();
  printf("real\tdelta_last= %.10lf, delta_start = %.10lf\n",
          (double) (a_time - last_time) / CLOCKS_PER_SEC, (double) (a_time - start_time) / CLOCKS_PER_SEC);
  printf("user\tdelta_last= %.10lf, delta_start = %.10lf\n",
          (double) (a_tms.tms_utime - last_tms.tms_utime) / CLOCKS_PER_SEC, (double) (a_tms.tms_utime - start_tms.tms_utime) / CLOCKS_PER_SEC);
  printf("system\tdelta_last= %.10lf, delta_start = %.10lf\n",
          (double) (a_tms.tms_stime - last_tms.tms_stime) / CLOCKS_PER_SEC, (double) (a_tms.tms_stime - start_tms.tms_stime) / CLOCKS_PER_SEC);
  last_time = a_time;
  last_tms = a_tms;
  printf("======\t=======================================\n");
}
//end time

int main(int argc, char **argv) {
  get_start_time();
  void * lib;
  lib = dlopen("liblist.so", RTLD_NOW);

  *(void **)(&fList_add) = dlsym(lib,"List_add");
  *(void **)(&fList_create) = dlsym(lib,"List_create");
  *(void **)(&fList_destroy) = dlsym(lib,"List_destroy");
  *(void **)(&fList_remove) = dlsym(lib,"List_remove");
  *(void **)(&fList_search) = dlsym(lib,"List_search");
  *(void **)(&fList_sort) = dlsym(lib,"List_sort");

  List * list = (*fList_create)();

  (*fList_add)(list, "Jakub", "Kruczek", "01.01.1990", "as@qwe", 888888888, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Weq", "Kasa", "01.01.1990", "as@qwe", 232621474, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Jakub", "Lsda", "01.01.1990", "as@qwe", 897652354, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Prop", "Lama", "01.01.1990", "as@qwe", 124796358, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Hey", "Poka", "01.01.1990", "as@qwe", 587625875, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Collable", "Buka", "01.01.1990", "as@qwe", 879269598, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Mass", "Strach", "01.01.1990", "as@qwe", 251475520, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Partycja", "Runnable", "01.01.1990", "as@qwe", 155486546, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Rekt", "Sad", "01.01.1990", "as@qwe", 357454541, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Maja", "Iopwq", "01.01.1990", "as@qwe", 457895478, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "Lsasd", "Nasda", "01.01.1990", "as@qwe", 565899658, "ul. asddwqe, Kraków 123-213");
  (*fList_add)(list, "RRww", "Lassa", "01.01.1990", "as@qwe", 658987895, "ul. asddwqe, Kraków 123-213");
  printf("Czas po dodaniu elementów:\n");
  print_times();
  for (int i = 0; i<100000; i++)
    (*fList_sort)(list);
  printf("Przesortowanie 100000x:\n");
  print_times();
  (*fList_remove)(list, list->first->next);
  (*fList_sort)(list);
  printf("Usuniecie i sortowanie:\n");
  print_times();

  return 0;
}
