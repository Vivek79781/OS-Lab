#ifndef GOODMALLOC_H
#define GOODMALLOC_H
#include <bits/stdc++.h>
#include <sys/time.h>
using namespace std;

struct node{
    int value;
    node* prev,*next;
};

struct symbol {
    char name[56];
    node* start;
    symbol* prev,*next;
    int scope;
    int size;
};

struct symbolTable{
    symbol *head;
    symbol *tail;
    int size = 0;
    void init();
    void removeScope();
    void add(char*, symbol*);
    symbol* remove(char*);
    symbol* get(char*,int);
};

struct free_hole{
    short start; // start index of the free hole
    int size;// size of the free hole
    struct free_hole* next;// pointer to the next free hole
    void init(int,short);// initialize the free hole
};

struct free_list{
    // List of node
    node* head;
    node* tail;
    int size;
    void init();
    void insert(node*);
    node* remove();
};

// Global memory

int createMem(int);
int createList(int, char*);
int assignVal(char*, int, int,int);
int freeElem(char*);
int getVal(char*,int,int);

//Global symbol table
int memory_footprint();

//Global free list

void init_scope();
void start_scope();
void end_scope();
int get_scope();



#endif



