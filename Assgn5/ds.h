#ifndef DS_H
#define DS_H
#include <bits/stdc++.h>
#include <algorithm>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

typedef struct g
{
    int id;
    int priority;
    int evict;
    int roomid;
}guest_ds;

typedef struct r
{
    int id;
    int tot_time;
    int state;// 0 for no one has used this room after it is cleaned
    //1 for someone has used this room after it is cleaned
    //2 for one has used it but not cleaned,
    // 3 for 2 guest is staying here
    // 4 second left and no one is here right now
    guest_ds Guest;
    // int g_priority;
    // int g_id;

}room;

extern int flag_cleaner;// cleaners doing their job if this is 1

extern int N,X,Y;
extern guest_ds * guests;
extern room * rooms;

extern unordered_set <int> rooms_set; // set of room indices to be cleaned
extern unordered_set <int> Rset; // set of rooms free

extern int guest_count;
extern int occupancy;
extern int K; // proportionality constant for cleaning time

extern sem_t sem_tot;// Initialise it to the total number of occupancies it can happen
extern sem_t sem_room; // Initialise it to the number of rooms
extern sem_t sem_clean;// Initialise it to the 0,but the last occupancy guest thread time make it signals Y times.
extern sem_t* per_room; // Array of semaphores, one for each room
extern sem_t* per_guest; // Array of semaphores, one for each guest
extern sem_t occupancy_sem; // Semaphore to protect the occupancy variable
extern sem_t guest_count_sem; // Semaphore to protect the guest_count variable
extern sem_t rset_sem; // Semaphore to protect the rooms_set variable
extern sem_t print_sem; //Semaphore to print 
extern sem_t sem_Rset; // Semaphore to protect the Rset variable
extern sem_t sem_pq; // Semaphore to protect the pq variable
// srand(time(NULL); in main

extern set <pair<int, int>> pq; // priority queue to get the available least priority guest filled room

void * guest(void *arg);
void * cleaner(void *arg);

#endif