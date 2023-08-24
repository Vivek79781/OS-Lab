#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/wait.h>

#define MAXSIZE 10000

using namespace std;

typedef struct
{
    bool graph[MAXSIZE][MAXSIZE];

    int list0[MAXSIZE / 10];
    int list1[MAXSIZE / 10];
    int list2[MAXSIZE / 10];
    int list3[MAXSIZE / 10];
    int list4[MAXSIZE / 10];
    int list5[MAXSIZE / 10];
    int list6[MAXSIZE / 10];
    int list7[MAXSIZE / 10];
    int list8[MAXSIZE / 10];
    int list9[MAXSIZE / 10];

    int countset[10]; // array containing no. of nodes in ith list

    int count; // index of highest node number

} shared_memory;

void Print_2d_vector(shared_memory *shm)
{
    for (int i = 0; i < MAXSIZE; i++)
    {
        for (int j = 0; j < MAXSIZE; j++)
        {
            if (shm->graph[i][j])
                cout << 1 << " ";
            else
                cout << 0 << " ";
        }
        cout << endl;
    }
    return;
}

shared_memory *create_shared_memory(int &shmid)
{
    key_t key;
    const char *path = "/home";
    int proj_id = 1; // can be any non-zero integer
    key = ftok(path, proj_id);
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    shmid = shmget(key, 200000000, IPC_CREAT | 0666);

    shared_memory *shm = (shared_memory *)shmat(shmid, (void *)0, 0);

    shm->count = 0;
    for (int i = 0; i < 10; i++)
        shm->countset[i] = 0;

    for (int i = 0; i < MAXSIZE / 10; i++)
    {
        shm->list0[i] = 0;
        shm->list1[i] = 0;
        shm->list2[i] = 0;
        shm->list3[i] = 0;
        shm->list4[i] = 0;
        shm->list5[i] = 0;
        shm->list6[i] = 0;
        shm->list7[i] = 0;
        shm->list8[i] = 0;
        shm->list9[i] = 0;
    }

    for (int i = 0; i < MAXSIZE; i++)
        for (int j = 0; j < MAXSIZE; j++)
            shm->graph[i][j] = false;

    return shm;
}

void fill_list(shared_memory *shm)
{
    int n=shm->count;
    n=n/10;
    for(int i=0;i<n;i++)
    {
        shm->list0[shm->countset[0]++]=i;
    }
    for(int i=n;i<2*n;i++)
    {
        shm->list1[shm->countset[1]++]=i;
    }
    for(int i=2*n;i<3*n;i++)
    {
        shm->list2[shm->countset[2]++]=i;
    }
    for(int i=3*n;i<4*n;i++)
    {
        shm->list3[shm->countset[3]++]=i;
    }
    for(int i=4*n;i<5*n;i++)
    {
        shm->list4[shm->countset[4]++]=i;
    }
    for(int i=5*n;i<6*n;i++)
    {
        shm->list5[shm->countset[5]++]=i;
    }
    for(int i=6*n;i<7*n;i++)
    {
        shm->list6[shm->countset[6]++]=i;
    }
    for(int i=7*n;i<8*n;i++)
    {
        shm->list7[shm->countset[7]++]=i;
    }
    for(int i=8*n;i<9*n;i++)
    {
        shm->list8[shm->countset[8]++]=i;
    }
    for(int i=8*n;i<9*n;i++)
    {
        shm->list8[shm->countset[8]++]=i;
    }
    for(int i=9*n;i<shm->count;i++)
    {
        shm->list9[shm->countset[9]++]=i;
    }
}

void get_data(shared_memory *shm)
{
    // ifstream infile("facebook_combined.txt");
    ifstream infile("facebook_combined.txt");
    if (!infile)
    {
        cerr << "Error: could not open input file" << endl;
        exit(1);
    }

    // Determine number of nodes
    int max_node = -1;
    int src, dst;
    while (infile >> src >> dst)
    {
        max_node = max(max_node, max(src, dst));
    }
    shm->count = max_node + 1;

    // Reset file pointer to beginning of file
    infile.clear();
    infile.seekg(0, ios::beg);

    // Populate adjacency matrix
    while (infile >> src >> dst)
    {
        shm->graph[src][dst] = true;
        shm->graph[dst][src] = true; // assuming undirected graph
    }

    fill_list(shm);

    return;
}

int main(int argc, char** argv)
{
    int shmid;
    shared_memory *shm = create_shared_memory(shmid);
    // Print_2d_vector(shm);

    get_data(shm);

    // Print_2d_vector(shm);
    cout << "The highest index of a node is:" << endl;

        //sleep(5);

        if (fork() == 0) 
        {
            execl("./producer", "./producer", NULL);
        }

        else 
        {
            for(int i=0;i<10;i++)
            {
                if (fork() == 0)
                    {   
                        char *op=(char *)malloc(4*sizeof(char));
                        op[0]='o';
                        op[1]='p';;
                        op[2]='0'+i;
                        op[3]='\0';

                        char *in=(char *)malloc(2*sizeof(char));
                        in[0]='0'+i;
                        in[1]='\0';

                        if (argc>=2 && strcmp(argv[1], "-optimize") == 0)
                        {
                            execl("./consumer", "./consumer",in,op, "-optimize", NULL);

                        }
                        execl("./consumer", "./consumer",in,op, NULL);
                    }
            }
            
        }
    for(int i=0;i<11;i++)
    wait(NULL);

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
