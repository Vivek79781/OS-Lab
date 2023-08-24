#include "goodmalloc.h"
void *mem;
symbolTable* globalSymbolTable;
symbolTable* freeSymbolTable;

free_list* globalFreeList;
int scope;
int init_size_list=0;
int init_size_tab=0;

void free_list::init(){
    head = NULL;
    tail = NULL;
    size = 0;
}

void free_list::insert(node* node1){
    if(head == tail && head==NULL){
        head = node1;
        tail = node1;
        node *n = (node*)(node1);
        n->prev = NULL;
        n->next = NULL;
    } else {
        node1->prev = tail;
        node1->next = NULL;
        tail->next = node1;
        tail = node1;
    }
    size += 1;
}

node* free_list::remove(){
    if(head == tail && tail == NULL)
        return NULL;
    if(head == tail && tail != NULL){
        node* n = head;
        head = NULL;
        tail = NULL;
        size -= 1;
        n -> next = NULL;
        return n;
    }
    node* n = head;
    head = head->next;
    head->prev = NULL;
    size -= 1;
    n -> next = NULL;
    return n;
}

void symbolTable::init(){
    head = NULL;
    tail = NULL;
}

void symbolTable::add(char* name, symbol *s){
    strcpy(s->name, name);
    s->scope = scope;
    if(head == NULL){
        head = s;
        tail = s;
        s->prev = NULL;
        s->next = NULL;
    } else {
        // Add at the head
        s->next = head;
        s->prev = NULL;
        head->prev = s;
        head = s;
    }
    size++;
}

//remove scope fundction

void symbolTable::removeScope(){
    if(head == tail && tail == NULL)
        return;
    if(head == tail && tail != NULL){
        symbol* s = head;
        if(s->scope == scope){
            head = NULL;
            tail = NULL;
            // add into freeSymbolTable+
            cout<<"Removed "<<s->name<<" "<<s->scope<<endl;
            strcpy(s->name,"free");
            freeSymbolTable->add(s->name, s);

        }
        return;
    }
    // remove symbol having scope = given scope
    symbol* s = head;
    symbol * sn;
    while(s != NULL){
        sn=s->next;
        if(s->scope == scope){
            if(s->prev == NULL){
                head = s->next;
                s->next->prev = NULL;
            } else if(s->next == NULL){
                tail = s->prev;
                s->prev->next = NULL;
            } else {
                s->prev->next = s->next;
                s->next->prev = s->prev;
            }
            s->next = NULL;
            s->prev = NULL;
            // add into freeSymbolTable
            cout<<"Removed "<<s->name<<" "<<s->scope<<endl;
            strcpy(s->name,"free");
            freeSymbolTable->add(s->name, s);

        }
        s = sn;
    }
    return;
    
}

symbol* symbolTable::remove(char* name){
    if(head == tail && tail == NULL)
        return NULL;
    if(head == tail && tail != NULL){
        symbol* s = head;
        if(strcmp(head->name, name) == 0){
            head = NULL;
            tail = NULL;
            s->next = NULL;
            s->prev = NULL;
            s->size--;
            return s;
        }
        return NULL;
    }
    // remove symbol having name = given name
    symbol* s = head;
    while(s != NULL){
        if(strcmp(s->name, name) == 0){
            if(s->prev == NULL){
                head = s->next;
                s->next->prev = NULL;
            } else if(s->next == NULL){
                tail = s->prev;
                s->prev->next = NULL;
            } else {
                s->prev->next = s->next;
                s->next->prev = s->prev;
            }
            s->next = NULL;
            s->prev = NULL;
            s->size--;
            return s;
        }
        s = s->next;
    }
    return NULL;
}

symbol* symbolTable::get(char* name, int scope = -1){
    if(head == tail && tail == NULL)
        return NULL;
    if(head == tail && tail != NULL){
        symbol* s = head;
        if(strcmp(head->name, name) == 0 && (scope == -1 || head->scope == scope)){
            return s;
        }
        return NULL;
    }
    // get symbol having name = given name
    symbol* s = head;
    while(s != NULL){
        if(strcmp(s->name, name) == 0 && (scope == -1 || s->scope == scope)){
            return s;
        }
        s = s->next;
    }
    return NULL;
}

//  initialize the memory
int createMem(int size) {
    mem=malloc(size);
    if (mem==NULL) {
        cout<<"Error: Memory not allocated"<<endl;
        return 0;
    }
    else {
        // initialise free list with the whole memory as a free hole
        globalFreeList = (struct free_list*)mem;
        globalFreeList->init();
        struct node* freeHole = (struct node*)(mem+sizeof(free_list ));
        // add all node to the free list
        int i=sizeof(node);
        while(i<0.7 * size)
        {
            globalFreeList->insert(freeHole);
            freeHole=freeHole+1;
            i=i+sizeof(node);
        }
        
        // allocate symbol table its address
        globalSymbolTable = (struct symbolTable*)(mem+sizeof(free_list)+(int)(0.7*size));
        // initialise the symbol table
        globalSymbolTable->init();
        freeSymbolTable = (struct symbolTable*)(mem+sizeof(free_list)+(int)(0.7*size)+sizeof(symbolTable));
        freeSymbolTable->init();
        symbol *s = (symbol*)(mem+sizeof(free_list)+(int)(0.7*size)+sizeof(symbolTable)+sizeof(symbolTable));
        i = sizeof(symbol);
        while(i<0.1*size){
            freeSymbolTable->add("free",s);
            s = s+1;
            i = i+sizeof(symbol);
        }
        init_size_list=globalFreeList->size;
        init_size_tab=freeSymbolTable->size;
        cout<<"Memory created"<<endl;
        return 1;
    }
}

int createList(int size, char* name){
    if(freeSymbolTable->size == 0){
        cout<<"Error: Not enough memory"<<endl;
        return 0;
    }
    if(globalFreeList->size < size){
        cout<<"Error: Not enough memory"<<endl;
        return 0;

    } else {
        struct node* list = globalFreeList->remove();
        struct node* list1 = list;
        int i = 1;
        while(i < size){
            struct node* list2 = globalFreeList->remove();
            list1->next = list2;
            list2->prev = list1;
            list1 = list2;
            i++;
        }
        list1->next = NULL;

        symbol* freeSymbol = freeSymbolTable->remove("free");
        strcpy(freeSymbol->name, name);
        freeSymbol->start = list;
        globalSymbolTable->add(name, freeSymbol);
        cout<<"List created"<<endl;
    }
    return 1;
}


int assignVal(char* name, int offset, int val, int scope)
{
    // get the address of the list from the symbol table
    symbol* s = globalSymbolTable->get(name,scope);
    // if the list is not found
    if(s == NULL)
    {
        // return 0
        cout<<"List not found"<<endl;
        return 0;
    }
    struct node* list = s->start;
    int i = 0;
    while(i < offset){
        list = list->next;
        i++;
    }
    int value = val;
    list->value=val;
    cout<<"Name: "<<name<<" Scope: "<<scope<<" Offset: "<<offset<<" Value: "<<list->value<<endl;
    return 1;
}

int getVal(char* name, int offset, int scope)
{
    symbol* s = globalSymbolTable->get(name,scope);
    // if the list is not found
    if(s == NULL)
    {
        cout<<"Error: "<<name<<" not found in the given scope"<<endl;
        return 0;
    }
    struct node* list = s->start;

    int i = 0;
    while(i < offset){
        list = list->next;
        i++;
    }
   return list->value;
}

int freeElem(char* name)
{
    // get the address of the list from the symbol table
    symbol* sym = globalSymbolTable->get(name);
    // if the list is not found
    if(sym == NULL)
    {
        // return 0
        cout<<"Error: "<<name<<" not found"<<endl;
        return 0;
    }
    struct node* list = sym->start;
    // add the list's head to freelist ke tail ka next and then freelist ke tail ko update karo
    // remove from symbol table
    symbol* re=globalSymbolTable->remove(name);
    
    // add in free list and update the free list
    while(list != NULL){
        node *next = list->next;
        globalFreeList->insert(list);
        list = next;
    }
    
    // return 1
    cout<<"List freed with name: "<<name<<" and address: "<<re->start<<" and scope"<<re->scope<<endl;
    return 1;
}

void start_scope()
{
    scope++;
}

void end_scope()
{
    // remove all elements from the symbol table which have scope = current scope
    globalSymbolTable->removeScope();
    scope--;
}

void init_scope()
{
    scope = 0;
}

int get_scope()
{
    return scope;
}

int memory_footprint()
{
    return (init_size_list-globalFreeList->size)*sizeof(struct node) + (init_size_tab-freeSymbolTable->size)*(sizeof(struct symbol));
}