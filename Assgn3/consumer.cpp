#include <iostream>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <bits/stdc++.h>

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
int dist[MAXSIZE];
int vis[MAXSIZE];
int par[MAXSIZE];
int ind;
int lc;

void dijkstra(shared_memory* shm,vector<int>&src) {
    //int dist[n], vis[n], par[n];
    ofstream myfile;
    char *op=(char *)malloc(4*sizeof(char));
                        op[0]='o';
                        op[1]='p';;
                        op[2]='0'+ind;
                        op[3]='\0';
    myfile.open(op);
            time_t my_time = time(NULL);  
            // ctime() used to give the present time            
            myfile<<ctime(&my_time);

    int n=shm->count;
    cout<<n<<"\n";
    for(int i = 0; i<n; i++) {
        dist[i] = INT16_MAX;
        vis[i] = 0;
        par[i] = -1;
    }
    priority_queue<pair<int, int>> pq;
    for(auto v:src) {
        dist[v] = 0;
        pq.push({0, v});
        par[v] = v;
    }
    while(!pq.empty()) {
        auto pt = pq.top();
        pq.pop();
        int curd = dist[pt.second];
        if(vis[pt.second]) continue;
        //cout<<pt.second<<"\n";
        vis[pt.second] = 1;
        for(int i = 0; i<shm->count; i++) {
            if(shm->graph[pt.second][i]) {
                if(dist[i] > curd + 1) {
                    dist[i] = curd + 1;
                    par[i] = pt.second;
                    pq.push({-dist[i], i});
                }
            }
        }
    }
    myfile<<"Path yaha se chalu haua\n";
    for(int i = 0; i<n; i++) {
        if(par[i] == -1) continue;
        int j = i;
        if(j!=par[j])
            myfile<<j<<" <- ";
            else
            myfile<<j;
        while(par[j] != j) {
            j = par[j];
            if(j!=par[j])
            myfile<<j<<" <- ";
            else
            myfile<<j;
            
        }
        myfile<<"\n";
    }
}

void optimize(vector<int> &new_nodes, shared_memory *shm,vector<int>&src)
{
    int n=shm->count;
    ofstream myfile;
    char *op=(char *)malloc(4*sizeof(char));
                        op[0]='o';
                        op[1]='p';;
                        op[2]='0'+ind;
                        op[3]='\0';
    myfile.open(op,ios::app);
            //sleep(30);
            time_t my_time = time(NULL);  
            // ctime() used to give the present time            
            myfile<<ctime(&my_time);
    int *distance=(int *)malloc(MAXSIZE*sizeof(int));


    for(int i=0;i<MAXSIZE;i++)
    {
        distance[i]=dist[i];
    }
    // for(int i = 0;i < n;i++){
    //     myfile<<i<<" "<<par[i]<<endl;
    // }
    // myfile<<"Distance of zero "<<distance[0]<<"\n";
    // myfile<<"New Parent"<<endl;

    for (int i = 0; i < new_nodes.size(); i++)
    {
        int u = new_nodes[i];
        // for(int j = 0;j < shm->count;j++){
        //     if(shm->graph)
        // }
        for (int v = 0; v < shm->count; v++)
        {
            if (shm->graph[u][v] == 1)
            {
                //myfile<<u<<" "<<v<<" new edge\n";
                int src, dest;
                if (distance[u] < distance[v] - 1)
                {
                    distance[v] = distance[u] + 1;
                    src = u;
                    par[v]=u;
                    dest = v;
                }
                else if (distance[v] < distance[u] - 1)
                {
                    distance[u] = distance[v] + 1;
                    src = v;
                    par[u]=v;
                    dest = u;
                }
                else
                {
                    continue;
                }
                priority_queue<pair<int, int>> pq;
                pq.push({-distance[dest], dest});
                while (!pq.empty())
                {
                    int top = pq.top().second;
                    pq.pop();
                    for (int next = 0; next < shm->count; next++)
                    {
                        if (shm->graph[top][next] == 1 && distance[next] -1 > distance[top])
                        {
                            distance[next] = distance[top] + 1;
                            par[next]=top;
                            pq.push({-distance[next], next});
                        }
                    }
                }
            }
        }
    }
    // myfile<<"Distance of zero "<<distance[0]<<"\n";

    // for(int i = 0;i < n;i++){
    //     myfile<<i<<" "<<par[i]<<endl;
    // }


    myfile<<"Optimise Path yaha se chalu haua\n";
    for(int i=lc;i<src.size();i++)
    myfile<<src[i]<<"\n";
    for(int i = 0; i<n; i++) {
        if(dist[i]==distance[i])continue;
        if(par[i] == -1) continue;
        int j = i;
        if(j!=par[j])
            myfile<<j<<" <- ";
            else
            myfile<<j;
        while(par[j] != j) {
            j = par[j];
            if(j!=par[j])
            myfile<<j<<" <- ";
            else
            myfile<<j;
            
        }
        myfile<<"\n";
    }

    for(int i=0;i<shm->count;i++)
    dist[i]=distance[i];

    free(distance);
}

int main(int argc, char** argv) {

    //cout<<"In Consumer\n";
    key_t key;
    const char* path = "/home";
    int proj_id = 1; // can be any non-zero integer
    key = ftok(path, proj_id);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Retrieve the shared memory segment created in the previous example
    //sleep(2);
    int shm_id;

    // printf("Hi\n");
    int size = 200000000;

    shm_id = shmget(key, size, 0666);
    vector<vector<int>> vec;

    // printf("Hi\n");

    // Attach the shared memory segment to the current process
    shared_memory* shm = (shared_memory *) shmat(shm_id, NULL, SHM_RDONLY);
    if ((void*)shm == (void*) -1) {
        perror("shmat");
        exit(1);
    }
    // printf("Hi\n");

    // Read the contents of the shared memory segment into a vector
    // std::vector<vector<int>> vec;
    // // std::memcpy(vec.data(), shared_mem, vec.size() * sizeof(int));
    // for (int i = 0; i < 5; i++) {
    //     std::vector<int> row;
    //     for (int j = 0; j < 5; j++) {
    //         row.push_back(shared_mem[i * 5 + j]);
    //     }
    //     vec.push_back(row);
    // }

    //cout<<argc<<" "<<atoi(argv[1])<<"\n";

    for(int i=0;i<MAXSIZE;i++)
    {
        dist[i]=INT_MAX;
        vis[i]=0;
        par[i]=-1;
    }
    cout<<argv[2]<<"\n";
    ind=atoi(argv[1]);
   
    if(argc==3)// no optmiser flag
    {
        while(1)
        {
            

            
            cout<<ind<<"\n";
            vector<int>src;
            for(int i=0;i<shm->countset[ind];i++)
            {
                if(ind==0)
                src.push_back(shm->list0[i]);
                else if(ind==1)
                src.push_back(shm->list1[i]);
                else if(ind==2)
                src.push_back(shm->list2[i]);
                else if(ind==3)
                src.push_back(shm->list3[i]);
                else if(ind==4)
                src.push_back(shm->list4[i]);
                else if(ind==5)
                src.push_back(shm->list5[i]);
                else if(ind==6)
                src.push_back(shm->list6[i]);
                else if(ind==7)
                src.push_back(shm->list7[i]);
                else if(ind==8)
                src.push_back(shm->list8[i]);
                else if(ind==9)
                src.push_back(shm->list9[i]);
            }
            // for(auto v:src)
            // cout<<v<<" ";
            // cout<<"\n";
            dijkstra(shm,src);
            //break;
            sleep(30);

        }
    }
    else
    {
        //shared_memory* shmc=NULL;
        int last_count=0;
        while(1)
        {
            
            if(last_count==0)
            {
                // first iteration here 
               
                ind=atoi(argv[1]);
                cout<<ind<<"\n";
                vector<int>src;
                for(int i=0;i<shm->countset[ind];i++)
                {
                    if(ind==0)
                    src.push_back(shm->list0[i]);
                    else if(ind==1)
                    src.push_back(shm->list1[i]);
                    else if(ind==2)
                    src.push_back(shm->list2[i]);
                    else if(ind==3)
                    src.push_back(shm->list3[i]);
                    else if(ind==4)
                    src.push_back(shm->list4[i]);
                    else if(ind==5)
                    src.push_back(shm->list5[i]);
                    else if(ind==6)
                    src.push_back(shm->list6[i]);
                    else if(ind==7)
                    src.push_back(shm->list7[i]);
                    else if(ind==8)
                    src.push_back(shm->list8[i]);
                    else if(ind==9)
                    src.push_back(shm->list9[i]);
                }
                // for(auto v:src)
                // cout<<v<<" ";
                // cout<<"\n";
                lc=src.size();
                dijkstra(shm,src);
                last_count=shm->count;

            }
            else
            {
                printf("Jay Charbhuja Ji Ki\n");
                vector<int>st;
                for(int nn=last_count;nn<shm->count;nn++)
                {
                    st.push_back(nn);
                }

                vector<int>src;
                for(int i=0;i<shm->countset[ind];i++)
                {
                    if(ind==0)
                    src.push_back(shm->list0[i]);
                    else if(ind==1)
                    src.push_back(shm->list1[i]);
                    else if(ind==2)
                    src.push_back(shm->list2[i]);
                    else if(ind==3)
                    src.push_back(shm->list3[i]);
                    else if(ind==4)
                    src.push_back(shm->list4[i]);
                    else if(ind==5)
                    src.push_back(shm->list5[i]);
                    else if(ind==6)
                    src.push_back(shm->list6[i]);
                    else if(ind==7)
                    src.push_back(shm->list7[i]);
                    else if(ind==8)
                    src.push_back(shm->list8[i]);
                    else if(ind==9)
                    src.push_back(shm->list9[i]);
                }

                for(auto v:src)
                {
                    dist[v]=0;
                    par[v]=v;
                }
                
                optimize(st,shm,src);
                last_count=shm->count;
                lc=src.size();
               
                
            }
            sleep(30);
            //break;
            
            
        }
    }
    
    

    
    

    // printf(" %p \n", shared_mem);

    // Print the contents of the vector
    // for (int i = 0; i < vec.size(); i++) {
    //     for (int j = 0; j < vec[0].size(); j++) {
    //         printf(" %d ", vec[i][j]);
    //     }
    //     cout << "\n";
    // }



    printf("\n");
    shmdt(shm);

    return 0;
}
