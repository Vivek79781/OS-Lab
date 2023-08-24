#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <fstream>

#define MAXSIZE 10000

using namespace std;

typedef struct shared_memory
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

    int countset[10]; // array containing number of nodes in ith list

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

void get_data(shared_memory *shm)   // NOT needed here
{
    ifstream infile("facebook_combined.txt");
    // ifstream infile("sample.txt");
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
    return;
}

void insert(shared_memory *shm, int *deg, int index)
{
    shm->graph[index][shm->count] = true;
    shm->graph[shm->count][index] = true;
    deg[shm->count]++; // increasing the degree of the 2 nodes connected by 1 each
    deg[index]++;
    return;
}

void reinitRange(shared_memory *shm, int *range, int *deg)
{
    range[0] = deg[0];
    for (int k = 1; k < shm->count; k++)
        range[k] = range[k - 1] + deg[k];
}

void Producer(shared_memory *shm, int *range, int *deg)
{

    int k = (rand() % 20 + 1); // generating a random number of edges for each node between 1 and 20

    // for (int i = 0; i < shm->count; i++)
    // {
    //     cout << range[i] << endl;
    // }

    for (int i = 0; i < k; i++)
    {
        int x = rand() % range[shm->count - 1] + 1;
        // deciding which nodes to join based on the probability as per given in the question
        if (x < range[0])
        {
            insert(shm, deg, 0);

            for (int a = 1; a < shm->count; a++)
                range[a] -= range[0];
        }
        else
            for (int p = 1; p < shm->count; p++)
                if (x > range[p - 1] && x <= range[p])
                {
                    insert(shm, deg, p);
                    for (p; p < shm->count; p++) // removing the probablility range of pth node for further calculation
                        range[p] -= deg[p];
                    break;
                }
        // reinitRange(shm, range, deg);
    }
    shm->count++;
    reinitRange(shm, range, deg);
    return;
}

int main()
{

    cout<<"In Producer\n";
    key_t key = ftok("/home", 1);

    // shmget returns an identifier in shmid
    int shmid = shmget(key, 200000000, IPC_CREAT | 0666);

    // shmat to attach to shared memory
    shared_memory *shm = (shared_memory *)shmat(shmid, (void *)0, 0);

    // get_data(shm);
    // Print_2d_vector(shm);

    // creating an array to maintain degree of ith node
    int *deg = (int *)malloc(MAXSIZE * sizeof(int));

    for (int i = 0; i < MAXSIZE; i++)
        deg[i] = 0;

    // initializing the array recording degree of all the nodes
    for (int i = 0; i < MAXSIZE; i++)
        for (int j = 0; j < MAXSIZE; j++)
            if (shm->graph[i][j])
                deg[i]++;

    // index for noting which list to chose from
    int index = 0;
    int *range = (int *)malloc(MAXSIZE * sizeof(int)); // An array corresponding to probability function

    while (1)
    {
        int m = (rand() % 21 + 10); // generating a random number of nodes between 10 and 30

        for (int i = 0; i < m; i++) // recursively making m nodes with k connections
        {
            switch (index%10) // chosing the consumer list to put the node into
            {
            case 0:
                shm->list0[shm->countset[0]] = shm->count;
                shm->countset[0]++;
                break;
            case 1:
                shm->list1[shm->countset[1]] = shm->count;
                shm->countset[1]++;
                break;
            case 2:
                shm->list2[shm->countset[2]] = shm->count;
                shm->countset[2]++;
                break;
            case 3:
                shm->list3[shm->countset[3]] = shm->count;
                shm->countset[3]++;
                break;
            case 4:
                shm->list4[shm->countset[4]] = shm->count;
                shm->countset[4]++;
                break;
            case 5:
                shm->list5[shm->countset[5]] = shm->count;
                shm->countset[5]++;
                break;
            case 6:
                shm->list6[shm->countset[6]] = shm->count;
                shm->countset[6]++;
                break;
            case 7:
                shm->list7[shm->countset[7]] = shm->count;
                shm->countset[7]++;
                break;
            case 8:
                shm->list8[shm->countset[8]] = shm->count;
                shm->countset[8]++;
                break;
            case 9:
                shm->list9[shm->countset[9]] = shm->count;
                shm->countset[9]++;
                break;
            }
            range[0] = deg[0];
            for (int k = 1; k < shm->count; k++)
                range[k] = range[k - 1] + deg[k]; // values between range[i - 1] and range[i] corresponds to the range of i

            Producer(shm, range, deg);
            index++;
            //index = index % 10;
            // shm->count++;
        }
        sleep(50); // change to 50 at the end
    }

    shmdt(shm);
    return 0;
}