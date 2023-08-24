/*
Assignment 6
*/
#include "goodmalloc.h"
#define MAXVAL 100001
#define SIZE 50000

// take 250mb
// 50000 elems
// 1-100000 val
// recursive merge sort

int max_mem_print=0;

void merge(char *name, int size1, int size2, char *str1, char *str2,int scope)
{
    // traverse str1 and str2 array and copying the values accordingly into name array
    int i = 0, j = 0, l = 0;
    while (i < size1 && j < size2)
    {
        int val1 = getVal(str1, i,scope);
        int val2 = getVal(str2, j,scope);
        if (val1 < val2)
        {
            assignVal(name, l, val1,scope-1);
            i++;
        }
        else
        {
            assignVal(name, l, val2,scope-1);
            j++;
        }
        l++;
    }
    while (i < size1)
    {
        int val1 = getVal(str1, i,scope);
        assignVal(name, l, val1,scope-1);
        i++;
        l++;
    }
    while (j < size2)
    {
        int val2 = getVal(str2, j,scope);
        assignVal(name, l, val2,scope-1);
        j++;
        l++;
    }
    return;
}

void merge_sort(char *name, int size)
{
    if (size < 2)
        return;
    int scope = get_scope();
    start_scope();
    int size1 = size / 2;
    int size2 = size - size1;
    char str1[20], str2[20];
    // cout<<name<<" "<<str1<<" "<<str2<<endl;

    // create two lists of appropriate size
    if (!createList(size1, "a"))
    {
        printf("List creation failed");
        return;
    }
    if (!createList(size2, "b"))
    {
        printf("List creation failed");
        return;
    }

    // populate the lists
    for (int i = 0; i < size1; i++)
    {
        int val = getVal(name, i, scope);
        assignVal("a", i, val,scope+1);
    }
    for (int i = 0; i < size2; i++)
    {
        int val = getVal(name, i + size1, scope);
        assignVal("b", i, val,scope+1);
    }

    // recursive call for merge sort on right and left halves
    // cout<<str1<<" "<<str2<<endl;
    merge_sort("a", size1);
    merge_sort("b", size2);
    int scopea = get_scope();
    // merge the two sorted lists into the original list
    max_mem_print=max(max_mem_print,memory_footprint());
    // cout<<"Merge Started"<<endl;
    merge(name, size1, size2, "a", "b",scopea);

    // free the two lists
    // freeElem("a");
    // freeElem("b");

    end_scope();
    return;
}

int main()
{
    // create 250mb memory
    // start the time
    struct timeval begin;gettimeofday(&begin,0);
    init_scope();
    start_scope();
    if (!createMem(262144000))
    {
        printf("Memory creation failed");
        return 0;
    }

    // create list of 50000 elements
    if (!createList(SIZE, "arr"))
    {
        printf("List creation failed");
        return 0;
    }

    // assign random values to the list
    int scope = get_scope();
    for (int i = 0; i < SIZE; i++)
    {
        int val = rand() % 100000 + 1;
        assignVal("arr", i, val,scope);
    }

    // initialize scope

    // call merge sort
    // int count = 0;
    merge_sort("arr", SIZE);

    // open a file to write the sorted list
    FILE *fp = fopen("output.txt", "w");

    // write the sorted list to the file
    int val = getVal("arr", 0,scope);
    fprintf(fp, "%d ", val);
    for(int j = 1; j < SIZE; j++)
    {
        int val = getVal("arr", j,scope);
        fprintf(fp, "-> %d ", val);
    }
    cout<<scope<<endl;
    cout<<max_mem_print<<endl;
    end_scope();
    struct timeval end;gettimeofday(&end,0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;   
    printf("Time measured: %.3f seconds.\n", elapsed);
}