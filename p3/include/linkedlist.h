/**
 * Credits to https://github.com/skorks/c-linked-list
 */

#ifndef __LINKEDLIST_H
#define __LINKEDLIST_H

#include <fcntl.h>

typedef struct Node Node;

Node *createnode(pid_t pid, char *name);

pid_t get_pid(Node *node);

typedef struct LinkedList LinkedList;

LinkedList *makelist();

void add(Node *node, LinkedList *list);

void del(Node *node, LinkedList *list);

void display(LinkedList *list);

void reverse(LinkedList *list);

void reverse_using_two_pointers(LinkedList *list);

void destroy(LinkedList *list);

void foreach (LinkedList *list, void(fn_ptr)(Node *));

#endif