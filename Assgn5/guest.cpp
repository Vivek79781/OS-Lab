#include "ds.h"

/*
Sleeps for a random time between 10-20 seconds . After waking
up, requests for a room in the hotel for a time to stay between 10-30 seconds. A
guest thread G is allotted a room if either of the following conditions are met.
a) The number of guests currently in the hotel < N. No guest is removed in
this case and G will get an empty room.
b) The number of guests currently in the hotel equals N. However, there
exists at least 1 guest (H) with lower priority than G. In this case H is
removed from the hotel and G takes the place of H.
Naturally in either of the cases, the room allotted to G must have had at most 1
previous occupant.
After a guest is allotted a room, then it sleeps for the duration of stay. A guest
who is removed from the hotel first sleeps and then after waking up again tries
to get a room.. The guest threads repeat the above steps indefinitely.
Eventually all the rooms will have at least had 2 occupants and no room can be
allotted further. This means that the hotel needs to be cleaned.

*/

// use wait and signal methods for semaphores

void * guest(void * arg)
{
    guest_ds* g = (guest_ds *)(arg);// getting the guest id
    int g_id = g->id;
    int g_priority = g->priority; // getting the guest priority
    while(1) 
    {
        // sem_wait(&print_sem);
        // cout << "Guest " << g_id << " with priority " << g_priority << " is looking for a room" << endl;
        // sem_post(&print_sem);

        sem_wait(&sem_tot);
        sem_wait(&sem_Rset);
        sem_wait(&sem_pq);
        int time_sleep= rand()%11 + 10;
        if(Rset.size() > 0) 
        {
            int r = *Rset.begin();
            Rset.erase(Rset.find(r));

            sem_wait(&print_sem);
            cout << "Guest " << g_id << " with priority " << g_priority << " is allotted room " << r << endl;
            sem_post(&print_sem);

            sem_wait(&per_guest[g_id]);
            guests[g_id].evict = 0;
            guests[g_id].roomid = r;
            sem_post(&per_guest[g_id]);

            sem_wait(&per_room[r]);
            rooms[r].state++;
            rooms[r].Guest = guests[g_id];
            rooms[r].tot_time +=time_sleep;

            if(rooms[r].state == 3 && Rset.empty() && pq.empty()) {
                sem_wait(&print_sem);
                cout << "Hotel is full and needs to be cleaned. Jay Shree Ram" << endl;
                sem_post(&print_sem);
                // call_cleaner();
                sem_wait(&rset_sem);
                for(int i = 0; i < N; i++) {
                    rooms_set.insert(i);
                }

                for(int i = 0; i < X; i++) {
                    sem_post(&sem_clean);
                }
                sem_post(&rset_sem);

                sem_post(&per_room[r]);
                sem_post(&sem_Rset);
                sem_post(&sem_pq);
                sleep(time_sleep);
                continue;
            }
            else if(rooms[r].state==1)
            {
                pq.insert({guests[g_id].priority,r});
            }

            sem_post(&per_room[r]);
            sem_post(&sem_Rset);
            sem_post(&sem_pq);

            sem_wait(&print_sem);
            cout << "Guest " << g_id << " with priority " << g_priority << " is staying in room " << r << " for " << time_sleep << " seconds" << endl;
            sem_post(&print_sem);
            sleep(time_sleep);

            sem_wait(&per_guest[g_id]);
            if(guests[g_id].evict==1)
            {
                sem_post(&per_guest[g_id]);
                continue;
            }
            else
            {
                sem_wait(&sem_pq);
                sem_wait(&print_sem);
                cout << "Guest " << g_id << " with priority " << g_priority << " is leaving room " << r << endl;
                sem_post(&print_sem);
                guests[g_id].roomid=-1;
                
                sem_wait(&per_room[r]);
                
                if(pq.find({g_priority,r})!=pq.end())
                pq.erase(pq.find({g_priority,r}));

                rooms[r].state++;
                if(rooms[r].state==2)
                {
                    Rset.insert(r);
                }
                sem_post(&per_room[r]);
                sem_post(&per_guest[g_id]);
                sem_post(&sem_pq);
                continue;
            }


        }
        else
        {
            if(pq.empty())
            {
                sem_wait(&print_sem);
                cout << "Hotel is full and needs to be cleaned 22" << endl;
                sem_post(&print_sem);
                sem_post(&sem_Rset);
                sem_post(&sem_pq);
                sem_post(&sem_tot);
                // call_cleaner()
                sem_wait(&rset_sem);
                for(int i = 0; i < N; i++) {
                    rooms_set.insert(i);
                }

                for(int i = 0; i < X; i++) {
                    sem_post(&sem_clean);
                }
                sem_post(&rset_sem);

                continue;
            }
            else
            {
                auto pt=*pq.begin();
                if(pt.first<g_priority)
                {
                    sem_wait(&print_sem);
                    cout << "Guest " << g_id << " with priority " << g_priority << " evicted guest " << rooms[pt.second].Guest.id << " with priority " << pt.first << " from room " << pt.second << endl;
                    sem_post(&print_sem);

                    int rid=pt.second;
                    pq.erase(pq.find(pt));
                    sem_wait(&per_room[rid]);

                    rooms[rid].state+=2;
                    int tid=rooms[rid].Guest.id;
                    
                    sem_wait(&per_guest[tid]);
                    guests[tid].evict=1;
                    guests[tid].roomid=-1;
                    sem_post(&per_guest[tid]);

                    sem_wait(&per_guest[g_id]);
                    guests[g_id].evict=0;
                    guests[g_id].roomid=rid;
                    sem_post(&per_guest[g_id]);

                    rooms[rid].Guest=guests[g_id];
                    rooms[rid].tot_time+=time_sleep;
                    if(rooms[rid].state==3 && Rset.empty() && pq.empty())
                    {
                        sem_wait(&print_sem);
                        cout << "Hotel is full and needs to be cleaned. Jay Shree Ram" << endl;
                        sem_post(&print_sem);

                        // call_cleaner();
                        sem_wait(&rset_sem);
                        cout<<"Calling the cleaners\n";
                        for(int i = 0; i < N; i++) {
                            rooms_set.insert(i);
                        }

                        for(int i = 0; i < X; i++) {
                            sem_post(&sem_clean);
                        }
                        sem_post(&rset_sem);

                        sem_post(&per_room[rid]);
                        sem_post(&sem_Rset);
                        sem_post(&sem_pq);
                        sleep(time_sleep);
                        continue;
                    }

                    sem_post(&sem_Rset);
                    sem_post(&sem_pq);
                    sem_post(&per_room[rid]);
                    
                    sem_wait(&print_sem);
                    cout << "Guest " << g_id << " with priority " << g_priority << " is staying in room " << rid << " for " << time_sleep << " seconds" << endl;
                    sem_post(&print_sem);
                    sleep(time_sleep);

                    sem_wait(&per_guest[g_id]);
                    if(guests[g_id].evict==1)
                    {
                        sem_post(&per_guest[g_id]);
                        continue;
                    }
                    else
                    {
                        sem_wait(&print_sem);
                        cout << "Guest " << g_id << " with priority " << g_priority << " is leaving room " << rid << endl;
                        sem_post(&print_sem);

                        guests[g_id].roomid=-1;
                        sem_post(&per_guest[g_id]);
                        sem_wait(&per_room[rid]);
                        rooms[rid].state++;
                        sem_post(&per_room[rid]);
                        continue;
                    }


                }
                else
                {
                    // sem_wait(&print_sem);
                    // cout << "Guest " << g_id << " with priority " << g_priority << " didn't find a room" << endl;
                    // sleep(5);
                    // sem_post(&print_sem);

                    sem_post(&sem_Rset);
                    sem_post(&sem_pq);
                    sem_post(&sem_tot);
                    continue;
                }
            }
        }
    }
}