//Jai Shree Ram
#include <bits/stdc++.h>
#include <algorithm>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
// library to read and write files
#include <fstream>
int like=0,comment=0,post=0;
using namespace std;
#define l1 long long
#define int long long
#define actions_count 100
#define tot_nodes 37700

typedef struct at
{
    int uid;
    int aid;
    string atype;// 0 : post, 1: comment, 2: like
    time_t t_stamp;
    int pv;// priority value or the number of mutual friends ?
}action;

typedef struct node
{
    int id;
    int like=0;
    int post=0;
    int comment=0;
    queue<action>wq;
    queue<action>fq;
    int ptype;  // 0: priority, 1: chronological
}node;

ofstream fout;

vector<vector<int>>g(tot_nodes);


vector<int>degree(tot_nodes + 1);
vector<vector<action>> nodes_action(tot_nodes + 1);
pthread_mutex_t nodes_action_lock[tot_nodes];

unordered_set<int> usq;
pthread_mutex_t usq_lock;

pthread_mutex_t nodes_lock[tot_nodes][2];
// Declare the condition variables here
pthread_cond_t usq_cond;
map<pair<int,int>,int>mp;
int atid = 0;

pthread_mutex_t print_lock;

unordered_set<int> uset;
pthread_mutex_t uset_lock;
pthread_cond_t uset_cond;

// declare the nodes vector
vector<node>nodes(tot_nodes);

void *readPost(void *arg)
{
    while(1) {

        pthread_mutex_lock(&uset_lock);
        while(uset.empty())
            pthread_cond_wait(&uset_cond,&uset_lock);
        int a = *uset.begin();
        uset.erase(uset.find(a));
        pthread_mutex_unlock(&uset_lock);
        
        int type;
        vector<action>tp;
        
        type = nodes[a].ptype;
        // vector<action>tp;
        pthread_mutex_lock(&nodes_lock[a][1]);
        while(!nodes[a].fq.empty()) {
            
            action b = nodes[a].fq.front();
            nodes[a].fq.pop();
            if(mp.find({a,b.uid}) != mp.end())
                b.pv = mp[{a,b.uid}];
            else
                b.pv=mp[{b.uid,a}];
            tp.push_back(b);
            // action i = b;
            // pthread_mutex_lock(&print_lock);
            // cout<< a << " read action number "<<i.aid<<" of type "<<i.atype<<" posted by user "<<i.uid<<" at time "<<i.t_stamp<<"\n";
            // fout<< a << " read action number "<<i.aid<<" of type "<<i.atype<<" posted by user "<<i.uid<<" at time "<<i.t_stamp<<"\n";
            // pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_unlock(&nodes_lock[a][1]);

        if(type == 0) {
            sort(tp.begin(),tp.end(),[](action a,action b) {
                return a.pv > b.pv;
            });
        }
        else {
            sort(tp.begin(),tp.end(),[](action a,action b) {
                return a.t_stamp < b.t_stamp;
            });
        }

        for(auto i:tp) {
            pthread_mutex_lock(&print_lock);
            fout<<a<<" read action number "<<i.aid<<" of type "<<i.atype<<" posted by user "<<i.uid<<" at time "<<i.t_stamp<< " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
            cout<<a<<" read action number "<<i.aid<<" of type "<<i.atype<<" posted by user "<<i.uid<<" at time "<<i.t_stamp<< " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
            // fout<<a<<" read action number "<<i.aid<<" of type "<<i.atype<<", posted by user "<<i.uid<<" at time "<<i.t_stamp<< " pv: " << i.pv << " type: " << type <<" , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
            // cout<<a<<" read action number "<<i.aid<<" of type "<<i.atype<<", posted by user "<<i.uid<<" at time "<<i.t_stamp<< " pv: " << i.pv << " type: " << type <<" , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
            pthread_mutex_unlock(&print_lock);
        }
    }
}

void *pushUpdate(void *arg)
{
    // int id = (int)arg;
    while(1)
    {
        int neighbour;
        pthread_mutex_lock(&usq_lock);
        while(usq.empty())
        {
            pthread_cond_wait(&usq_cond,&usq_lock);
        }
        neighbour = *usq.begin();
        usq.erase(usq.find(neighbour));
        pthread_mutex_unlock(&usq_lock);
        // cout<<"in push Update thread\n";

        
        pthread_mutex_lock(&nodes_lock[neighbour][1]);// lock for neighbour ka fq
        pthread_mutex_lock(&nodes_action_lock[neighbour]);// lock for neighbour ka vector
        // uset ko lock kar
        pthread_mutex_lock(&uset_lock);
        uset.insert(neighbour);
        pthread_mutex_unlock(&uset_lock);
        // copy(nodes_action[neighbour].begin(), nodes_action[neighbour].end(), back_inserter(nodes[neighbour].fq));
        for(auto i:nodes_action[neighbour])
        {
            nodes[neighbour].fq.push(i);

            // pthread_mutex_lock(&print_lock);
            // fout<< i.atype << i.aid << " from user "<< i.uid << " destined to user " << neighbour << " pushed to its feed queue " << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";;
            // cout<< i.atype << i.aid << " from user "<< i.uid << " destined to user " << neighbour << " pushed to its feed queue " << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";;
            // pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_lock(&print_lock);
        fout<<"Actions destined to user " << neighbour << " pushed to its feed queue " << ", time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
        cout<<"Actions destined to user " << neighbour << " pushed to its feed queue " << ", time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
        pthread_mutex_unlock(&print_lock);
        pthread_mutex_unlock(&nodes_lock[neighbour][1]);
        nodes_action[neighbour].clear();
        pthread_mutex_unlock(&nodes_action_lock[neighbour]);
        pthread_cond_broadcast(&uset_cond);

        // pthread_mutex_lock(&print_lock);
        // fout<<"Actions destined to user " << neighbour << " pushed to its feed queue\n";
        // cout<<"Actions destined to user " << neighbour << " pushed to its feed queue\n";
        // pthread_mutex_unlock(&print_lock);
    }
}

void printAction(action a)
{
    cout<<"User "<<a.uid<<" did action "<<a.atype<<" at time "<<a.t_stamp<<endl;
}

int *picknum()
{
    int *nums = (int *)malloc(actions_count * sizeof(int));
    if(nums == NULL) {
        cout<<"Memory allocation failed" << endl;
        exit(1);
    }

    int i, j = 0, new_num;

    srand(time(NULL));
    nums[0] = rand() % tot_nodes;

    for (i = 1; i < actions_count; i++)
    {
        do
        {
            new_num = rand() % tot_nodes;
            for (j = 0; j < i; j++)
                if (nums[j] == new_num)
                    break;
        } while (j < i);   // repeat if the new number is already in the array
        nums[i] = new_num; // add the new number to the array
    }
    return nums;
}

int noOfActions(int n)
{
    int log_degree = (int)(1+log(n) / log(2));
    // return 1;
    return 11*log_degree;
    // return 11*log_degree;
}
// Jai Shr
action makeAction(int user_id, int action_id)
{
    action myaction;
    int action_type = (rand() % 3);

    // pthread_mutex_lock(&nodes_lock[user_id][0]);
    if(action_type == 0)
    {
        myaction.atype = "post";
        nodes[user_id].post++;
        myaction.aid = nodes[user_id].post;
    }
    else if(action_type == 1)
    {
        myaction.atype = "comment";
        nodes[user_id].comment++;
        myaction.aid = nodes[user_id].comment;
    }
    else
    {
        myaction.atype = "like";
        nodes[user_id].like++;
        myaction.aid = nodes[user_id].like;
    }
    // pthread_mutex_unlock(&nodes_lock[user_id][0]);
    myaction.uid = user_id;
    time_t ctime;
    ctime = time(NULL);
    myaction.t_stamp = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
    return myaction;
}

void *userSimulator(void *arg)
{
    // print timestamp upto microsecond
    cout << "userSimulator thread started at " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() << endl;
    int action_id = 1;
    while(1)
    {
        int *nodenums = picknum(); // pick 100 random node indices
        int noofactions[actions_count];
        for (int i = 0; i < actions_count; i++)
            noofactions[i] = noOfActions(degree[nodenums[i]]);  // deciding no of actions corresponding to each indices picked above
        for (int i = 0; i < actions_count; i++) // creating and pushing action to action queue usq
            for (int j = 0; j < noofactions[i]; j++)
            {
                action myaction = makeAction(nodenums[i], action_id++); // need to check this up
                // pthread_mutex_lock(&print_lock);
                // fout << "Action id: " << myaction.aid << " action type: " << myaction.atype << ", action generated for user id: " << myaction.uid << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                // cout << "Action id: " << myaction.aid << " action type: " << myaction.atype << ", action generated for user id: " << myaction.uid << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                // pthread_mutex_unlock(&print_lock);
                nodes[myaction.uid].wq.push(myaction);
                pthread_mutex_lock(&print_lock);
                fout << "Action id: " << myaction.aid << ", action type: " << myaction.atype << ", pushed in the wall queue of node " << myaction.uid << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                cout << "Action id: " << myaction.aid << ", action type: " << myaction.atype << ", pushed in the wall queue of node " << myaction.uid << " , time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                pthread_mutex_unlock(&print_lock);

                // printAction(myaction);
                for(auto v:g[myaction.uid])
                {
                    pthread_mutex_lock(&nodes_action_lock[v]);
                    pthread_mutex_lock(&usq_lock);
                    usq.insert(v);                
                    nodes_action[v].push_back(myaction);
                    pthread_mutex_lock(&print_lock);
                    fout << "Neighbour " << v << " of user " << myaction.uid << " is added in the usq set " <<", time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                    cout << "Neighbour " << v << " of user " << myaction.uid << " is added in the usq set " <<", time : " << chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count() <<"\n";
                    pthread_mutex_unlock(&print_lock);
                    pthread_mutex_unlock(&usq_lock);
                    pthread_mutex_unlock(&nodes_action_lock[v]);                
                    pthread_cond_broadcast(&usq_cond);
                }
                //pthread_mutex_lock(&print_lock);
                // fout << "Neighbours of user " << myaction.uid << " are added in the usq set" <<"\n";
                // cout << "Neighbours of user " << myaction.uid << " are added in the usq set" <<"\n";
                //pthread_mutex_unlock(&print_lock);               
            }
        free(nodenums);
        sleep(120);
        // Jay Shree Ram
    }
}

void* mainThread(void *arg)
{
    // load the graph
    ifstream fin("musae_git_edges.csv");
    string line;
    getline(fin,line);
    while(getline(fin,line))
    {
        stringstream ss(line);
        string s;
        vector<int>temp;
        while(getline(ss,s,','))
        {
            temp.push_back(stoi(s));
        }
        g[temp[0]].push_back(temp[1]);
        g[temp[1]].push_back(temp[0]);
    }

    // print graph g
    // for(int i=0;i<g.size();i++)
    // {
    //     cout<<i<<"->";
    //     for(int j=0;j<g[i].size();j++)
    //     {
    //         cout<<g[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }
//  cout << "Hello World!\n";
 
int f = 0;
    for(int i=0;i<g.size();i++)
    {
        unordered_set<int>st(g[i].begin(),g[i].end());
       
        for(int j=0;j<g[i].size();j++)
        {
            
            int cnt=0;
            for(int k=0;k<g[g[i][j]].size();k++)
            {
                if(st.find(g[g[i][j]][k])!=st.end())
                {
                    cnt++;
                    // cout << "Hello World!\n" << f++;
                }
            }
            mp[{i,g[i][j]}]=cnt;
        }
    }

//  cout << "Hello World!\n";
    for(int i = 0; i<g.size(); i++) {
        degree[i] = g[i].size();
        nodes[i].ptype = rand()%2;
    }

   

    // create 1 thread
    pthread_t userSimulator_thread;
    pthread_create(&userSimulator_thread,NULL,userSimulator,NULL);
    // create 25 threads
    pthread_t pushUpdate_threads[25];
    for(int i=0;i<25;i++)
    {
        pthread_create(&pushUpdate_threads[i],NULL,pushUpdate,NULL);
    }
    // create 10 threads
    pthread_t readPost_threads[10];
    for(int i=0;i<10;i++)
    {
        pthread_create(&readPost_threads[i],NULL,readPost,NULL);
    }
    
    // wait for all threads
    pthread_join(userSimulator_thread,NULL);
    for(int i=0;i<10;i++)
    {
        pthread_join(readPost_threads[i],NULL);
    }
    for(int i=0;i<25;i++)
    {
        pthread_join(pushUpdate_threads[i],NULL);
    }
    return NULL;
}



signed main()
{
    
    fout.open("sns.log");
    //saare locks and conditions initialize karna hai
    pthread_mutex_init(&usq_lock,NULL);
    pthread_cond_init(&usq_cond,NULL);
    for(int i=0;i<tot_nodes;i++)
    {
        pthread_mutex_init(&nodes_lock[i][0],NULL);
        pthread_mutex_init(&nodes_lock[i][1],NULL);
        // pthread_cond_init(&nodes_action_cond[i],NULL);
    }
    pthread_mutex_init(&print_lock,NULL);

    // create the main thread
    pthread_t main_thread;
    pthread_create(&main_thread,NULL,mainThread,NULL);
    pthread_join(main_thread,NULL);

    // saare locks and conditions destroy karna hai
    pthread_mutex_destroy(&usq_lock);
    pthread_cond_destroy(&usq_cond);
    for(int i=0;i<tot_nodes;i++)
    {
        pthread_mutex_destroy(&nodes_lock[i][0]);
        pthread_mutex_destroy(&nodes_lock[i][1]);
        // pthread_cond_destroy(&nodes_action_cond[i]);
    }
    return 0;
}