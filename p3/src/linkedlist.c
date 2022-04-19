#include <stdio.h>
#include <stdlib.h>

#include "../include/linkedlist.h"

struct Node {
  pid_t pid;
  char *name;
  Node *next;
};

Node *createnode(pid_t pid, char *name) {
  Node *newNode = malloc(sizeof(Node));
  if (!newNode) {
    return NULL;
  }
  newNode->pid = pid;
  newNode->name = name;
  newNode->next = NULL;
  return newNode;
}

pid_t get_pid(Node *node) { return node->pid; }

char *get_name(Node *node) { return node->name; }

Node *get_next(Node *node) { return node->next; }

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

Node *get_head(LinkedList *list) { return list->head; }

void display(LinkedList *list) {
  Node *current = list->head;
  if (list->head == NULL)
    return;

  for (; current != NULL; current = current->next) {
    printf("[%d] %s\n", current->pid, current->name);
  }
}

void add(Node *node, LinkedList *list) {
  Node *current = NULL;
  if (list->head == NULL) {
    list->head = createnode(node->pid, node->name);
  } else {
    current = list->head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = createnode(node->pid, node->name);
  }
}

void del(Node *node, LinkedList *list) {
  Node *current = list->head;
  Node *previous = current;
  while (current != NULL) {
    if (current->pid == node->pid) {
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

void foreach (LinkedList *list, void(fn_ptr)(Node * node)) {
  Node *current = list->head;
  while (current != NULL) {
    fn_ptr(current);
    current = current->next;
  }
}