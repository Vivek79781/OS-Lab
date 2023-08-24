#include "ds.h"

//  int flag_cleaner=0;// cleaners doing their job if this is 1

 int N,X,Y;
 guest_ds * guests;
 room * rooms;

 unordered_set <int> rooms_set; // set of room indices to be cleaned
 unordered_set <int> Rset; // set of rooms free
 int guest_count=0;
 int occupancy=0;
 int K = 1; // proportionality constant for cleaning time

 sem_t sem_tot;// Initialise it to the total number of occupancies it can happen
 sem_t sem_room; // Initialise it to the number of rooms
 sem_t sem_clean;// Initialise it to the 0,but the last occupancy guest thread time make it signals Y times.
 sem_t* per_room; // Array of semaphores, one for each room
 sem_t* per_guest; // Array of semaphores, one for each guest
 sem_t occupancy_sem; // Semaphore to protect the occupancy variable
 sem_t guest_count_sem; // Semaphore to protect the guest_count variable
 sem_t rset_sem; // Semaphore to protect the rooms_set variable
 sem_t print_sem;
 set <pair<int, int>> pq; // priority queue to get the available least priority guest filled room
 sem_t sem_Rset; // Semaphore to protect the Rset variable
 sem_t sem_pq; // Semaphore to protect the pq variable


int main(){
    // Creates the guest and cleaning staff threads. Initialize semaphores
    // which will control access to rooms for guests and cleaning threads.
    cin>>Y>>N>>X;
    pthread_t guest_th[Y];
    pthread_t clean_th[X];

    guests=(guest_ds *)malloc(Y*sizeof(guest_ds));
    rooms=(room *)malloc(N*sizeof(room));
    per_room=(sem_t *)malloc(N*sizeof(sem_t));
    per_guest=(sem_t *)malloc(Y*sizeof(sem_t));

    // initializing the semaphores
    sem_init(&sem_tot,0,2*N);
    sem_init(&sem_room,0,N);
    sem_init(&sem_clean,0,0);
    sem_init(&occupancy_sem,0,1);
    sem_init(&guest_count_sem,0,1);
    sem_init(&print_sem,0,1);
    sem_init(&rset_sem,0,1);
    sem_init(&sem_Rset,0,1);
    sem_init(&sem_pq,0,1);
    
    for(int i=0;i<N;i++)
    {
        sem_init(&per_room[i],0,1);
    }
    for(int i=0;i<Y;i++)
    {
        sem_init(&per_guest[i],0,1);
    }

    // initailizing the rooms
    for(int i=0;i<N;i++)
    {
        rooms[i].id=i;
        rooms[i].tot_time=0;
        rooms[i].state=0;
        rooms[i].Guest.id=-1;
        rooms[i].Guest.priority=-1;
        rooms[i].Guest.evict=0;
        rooms[i].Guest.roomid=-1;
    }
    // creating the guest threads
    for(int i=0;i<N;i++)
    {
        Rset.insert(i);
    }
    for(int i=0;i<Y;i++)
    {
        // take a random priority and id and send it to the guest function
        guests[i].priority = rand()%Y;
        guests[i].id = i;
        guests[i].roomid = -1;
        guests[i].evict = 0;
        // cout<<"Here going to guests\n";
        pthread_create(&guest_th[i],NULL,guest,(void *)&(guests[i]));
        // cout<<guests[i].id<<" "<<guests[i].priority<<endl;

    }
    
    // creating the cleaning threads
    for(int i=0;i<X;i++)
    {
        int* ty=(int *)malloc(sizeof(int));
        *ty=i;
        pthread_create(&clean_th[i],NULL,cleaner,(void *)ty);
    }

    // joining the guest threads
    for(int i=0;i<Y;i++)
    {
        pthread_join(guest_th[i],NULL);
    }

    // joining the cleaning threads
    for(int i=0;i<X;i++)
    {
        pthread_join(clean_th[i],NULL);
    }


    
}