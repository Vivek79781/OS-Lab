#include "ds.h"

/*
Wakes up when required(all rooms are occupied 2
times).Selects a random room which was occupied since the last time the hotel
was cleaned.The amount of time to clean a room is proportional to the time it
was occupied by previous guests.The cleaning thread sleeps for the duration of
cleaning a room and then marks the room as clean.This is done till all the rooms are cleaned.
*/


void *cleaner(void *arg) {

// cout<<"Cleaner opened\n";

int cle=*((int*)(arg));
// free(arg);
    while(1) {
        sem_wait(&sem_clean);

        sem_wait(&print_sem);
        cout<<"Cleaner "<<cle<<" gets opened\n";
        sem_post(&print_sem);

        sem_wait(&rset_sem);
        while(!rooms_set.empty())
        {
            int i = *rooms_set.begin();
            rooms_set.erase(rooms_set.find(i));
            sem_post(&rset_sem);
            sem_wait(&per_room[i]);
            int time_sleep=rooms[i].tot_time;
            if(rooms[i].state==3)
            {
                sem_wait(&per_guest[rooms[i].Guest.id]);
                guests[rooms[i].Guest.id].evict=1;
                sem_post(&per_guest[rooms[i].Guest.id]);
                sem_wait(&print_sem);
                cout<<"Guest "<<rooms[i].Guest.id<<" is evicted\n";
                sem_post(&print_sem);
            }
            rooms[i].state=0;
            rooms[i].tot_time=0;
            sem_wait(&print_sem);
            cout<<"Cleaning the room "<<i<<" by cleaner "<<cle<<"\n";
            sem_post(&print_sem);

            sleep(time_sleep * K);
            sem_post(&per_room[i]);
            sem_wait(&rset_sem);
        }
        sem_post(&rset_sem);
        if(cle == 0) {
            sem_wait(&sem_Rset);
            for(int i = 0; i<N; i++) {
                Rset.insert(i);
            }
            sem_post(&sem_Rset);
            for(int i = 0; i<2*N; i++) {
                sem_post(&sem_tot);
            }
            
        }
    }
    return NULL;

}