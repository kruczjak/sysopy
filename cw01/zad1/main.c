#include<list.h>
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char **argv) {
  static List * list = NULL;
  list = List_create();
  struct info data;
  data.firstName = "Jakub";
  data.lastName = "Kruczek";
  data.birthDate = "01.01.1990";
  data.email = "kruczjak@gmail.com";
  data.phone = 88888888;
  data.address = "ul. Budryka 6, Kraków 33-333";
  List_push(list, &data);

  return 0;
}
