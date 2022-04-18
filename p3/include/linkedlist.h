/**
 * Credits to https://github.com/skorks/c-linked-list
 */

#ifndef __LINKEDLIST_H
#define __LINKEDLIST_H

typedef struct Node Node;

Node *createnode(int data);

typedef struct LinkedList LinkedList;

LinkedList *makelist();

void add(int data, LinkedList *list);

void del(int data, LinkedList *list);

void display(LinkedList *list);

void reverse(LinkedList *list);

void reverse_using_two_pointers(LinkedList *list);

void destroy(LinkedList *list);

#endif
