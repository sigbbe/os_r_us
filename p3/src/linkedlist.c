#include <stdio.h>
#include <stdlib.h>

#include "../include/linkedlist.h"

struct Node {
  int data;
  struct Node *next;
};

Node *createnode(int data) {
  Node *newNode = malloc(sizeof(Node));
  if (!newNode) {
    return NULL;
  }
  newNode->data = data;
  newNode->next = NULL;
  return newNode;
}

struct LinkedList {
  Node *head;
};

LinkedList *makelist() {
  LinkedList *list = malloc(sizeof(LinkedList));
  if (!list) {
    return NULL;
  }
  list->head = NULL;
  return list;
}

void display(LinkedList *list) {
  Node *current = list->head;
  if (list->head == NULL)
    return;

  for (; current != NULL; current = current->next) {
    printf("%d\n", current->data);
  }
}

void add(int data, LinkedList *list) {
  Node *current = NULL;
  if (list->head == NULL) {
    list->head = createnode(data);
  } else {
    current = list->head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = createnode(data);
  }
}

void delete (int data, LinkedList *list) {
  Node *current = list->head;
  Node *previous = current;
  while (current != NULL) {
    if (current->data == data) {
      previous->next = current->next;
      if (current == list->head)
        list->head = current->next;
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }
}

void reverse(LinkedList *list) {
  Node *reversed = NULL;
  Node *current = list->head;
  Node *temp = NULL;
  while (current != NULL) {
    temp = current;
    current = current->next;
    temp->next = reversed;
    reversed = temp;
  }
  list->head = reversed;
}

/**
 * Reversing the entire list by changing the direction of link from forward to
 * backward using two pointers
 */
void reverse_using_two_pointers(LinkedList *list) {
  Node *previous = NULL;
  while (list->head) {
    Node *next_node = list->head->next; // points to second node in list
    list->head->next = previous;        // at initial making head as NULL
    previous = list->head;  // changing the nextpointer direction as to point
                            // backward node
    list->head = next_node; // moving forward by next node
  }
  list->head = previous;
}

void destroy(LinkedList *list) {
  Node *current = list->head;
  Node *next = current;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  free(list);
}
