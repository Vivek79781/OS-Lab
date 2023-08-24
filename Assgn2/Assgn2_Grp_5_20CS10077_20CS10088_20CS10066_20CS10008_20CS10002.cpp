#include <limits.h>
#include <pwd.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
#include <termios.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <glob.h>
#include <signal.h>
#include <string.h>
#define MAX_STR_LEN 2000
using namespace std;
#define MAX_HISTORY 10000;



//global variables for finding the culprit process in the sb command
int maxPID = 0, prevPID;
pid_t child_pid;
double currTime, maxd = INT_MIN, prevTime = 0;
bool running = false,bg = false;

//The termio structure in the termio. h file defines the basic input, output, control, and line discipline modes
struct termios tr_old, tr_new;

// This function counts the number of child processes
int count_child_processes(pid_t parent_pid)
{
    int count = 0;
    DIR *dir = opendir("/proc");//the informsation directory of all the processes
    if (dir)
    {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL)// going to all the processes and then checking whether its parent is equal to parent_pid or not
        {
            if (ent->d_name[0] >= '0' && ent->d_name[0] <= '9')
            {
                pid_t pid = atoi(ent->d_name);
                char path[100];
                sprintf(path, "/proc/%d/stat", pid);
                FILE *file = fopen(path, "r");
                if (file)
                {
                    int ppid;
                    fscanf(file, "%*d %*s %*c %d", &ppid);
                    if (ppid == parent_pid)
                    {
                        ++count;
                    }
                    fclose(file);
                }
            }
        }
        closedir(dir);
    }
    return count;
}

// This function prints the time taken by a process
void print_time(pid_t pid)
{
    struct timespec start, end;
    int fd;
    char filename[1024];
    char buffer[1024];
    ssize_t n;

    snprintf(filename, sizeof(filename), "/proc/%d/stat", pid);// opening the infomation file of that process
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

    n = read(fd, buffer, sizeof(buffer));
    if (n == -1)
    {
        perror("read");
        exit(1);
    }
    close(fd);

    unsigned long utime;
    unsigned long stime;
    sscanf(buffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);// scanning the time 14th argument and 15th argument

    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // converting the time into seconds as the bove is in the form of number of clock ticks
    double elapsed_utime = utime / sysconf(_SC_CLK_TCK);
    double elapsed_stime = stime / sysconf(_SC_CLK_TCK);
    currTime = elapsed_utime + elapsed_stime;// total time of  process
    printf("User CPU time: %f seconds\n", elapsed_utime);
    printf("System CPU time: %f seconds\n", elapsed_stime);
}

// This function prinths the parent tree by going upto the pid 1
void print_tree(int pid)
{

    print_time(pid);// first printing the time
    int child_count = count_child_processes(pid);
    printf("The pid %d and its count of children is %d\n", pid, child_count);// printing the count of children

    if (prevTime - currTime > maxd)
    {
        maxd = prevTime - currTime;
        maxPID = pid;
    }// culprit finding check

    char filename[1024];
    char line[1024];
    int parent;
    prevPID = pid;
    prevTime = currTime;
    snprintf(filename, sizeof(filename), "/proc/%d/status", pid);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "PPid:", 5) == 0)
        {
            parent = atoi(line + 5);
            break;
        }
    }// finding its parent 
    fclose(file);

    if (parent)
    {
        printf("%d -> %d\n", parent, pid);
        print_tree(parent);
    }// going to its parent
}

// The class for storing the previous commands
class History
{
private:
    deque<string> Commands; // the deque of commands

public:
    void add_cmd(string s) // adding a command and then checking if it exceeds tha maximum size or not
    {
        if (Commands.size() == MAX_HISTORY + 0)
            Commands.pop_back();
        Commands.push_front(s);
        return;
    }
    string latest()// return the most recent command 
    {
        return Commands.front();
    }
    string extract(int ind) // extracting the command at a particular index
    {
        return Commands.at(ind);
    }
    string del()// delete a command from the front
    {
        if(Commands.size()>0)
        {
            string s=Commands.back();
            Commands.pop_back();
            return s;
        }
        else
        {
            return "EOF";
        }
    }
    int history_size()// returns the total size
    {
        return Commands.size();
    }
};

History history;


// This function removes the spaces from both ends
string cut_off_spaces_from_end(string s)
{
    for (int i = 0; i < s.size(); i++)
        if (s[i] != ' ')
        {
            s = s.substr(i);
            break;
        }
    for (int i = 0; i < s.size(); i++)
        if (s[s.size() - i - 1] != ' ')
        {
            s = s.substr(0, s.size() - i);
            break;
        }
    return s;
}

// This function tokenizes the string with a given delimeter and sends a vector of strings
vector<string> tokenize(string s, char delim)
{
    vector<string> res;
    string elem = "";
    for (char c : s)
    {
        if (c == delim)
        {
            if (delim != ' ' || elem != "")
                res.push_back(elem);
            elem = "";
        }
        else
            elem.push_back(c);
    }
    res.push_back(elem);
    return res;
}

// function to check whether the command is valid or not
bool check_valid(string cmd)
{
    ifstream fin;
    string ostr;
    cmd = cut_off_spaces_from_end(cmd);
    cmd = tokenize(cmd, ' ')[0];

    //for the squashbug and delep commands
    if (cmd == "sb" || cmd=="delep")
        return true;
    // type command help us to check whether the command is valid or not and store the result in opt.txt
    string test_cmd = ("type " + cmd + " > opt.txt 2>&1").c_str();
    system(test_cmd.c_str());
    // read the output from opt.txt
    fin.open("opt.txt");
    getline(fin, ostr);
    // remove opt.txt after storing the output in a string
    system("rm opt.txt");
    // if the result is not found then return false else return true
    vector<string> vec=tokenize(ostr, ' ');
    if (*(vec.end()-2)+*(vec.end()-1) == "notfound")
        return false;
    return true; 
}

void cd_check(string &cmd)
{
    cmd = cut_off_spaces_from_end(cmd);// check if the command is cd or not
    if (tokenize(cmd, ' ')[0] != "cd")
    {
        return;
    }
    cmd.replace(0, 3, "");
    cmd.erase(remove(cmd.begin(), cmd.end(), '\"'), cmd.end());// remove the cd part from the command

    if (!cmd.size())// if there is nothing go to home
    {
        chdir(getenv("HOME"));
        return;
    }

    int status = chdir(cmd.c_str());
    if (status == -1)
    {
        cout << "cd: " + cmd + ": No such file or directory\n";
    }
    cmd = "";
}

void pwd_check(string &cmd)
{
    cmd = cut_off_spaces_from_end(cmd);
    if (tokenize(cmd, ' ')[0] != "pwd")// check if the command is pwd or not
    {
        return;
    }
    cmd.replace(0, 3, "");// remove the pwd part
    char cwd[PATH_MAX]; 
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        cout << cwd << endl;
    }
    else
    {
        cout << "Error while trying to get current working directory" << endl;
    }
    //cout<<cmd<<"\n";
}

//This function splits the command from the file redirection parts
vector<string> split_io(string cmd)
{
    vector<string> parsed;
    string output, input, command;
    bool lsign = true, rsign = true;

    if (cmd.find('>') == string::npos) 
    {
        output = "";
        rsign = false;
    }
    else
        output = tokenize(cmd, '>').back(); // setting the output file

    output = cut_off_spaces_from_end(output);
    output = tokenize(output, '<')[0];
    if (output[0] == '\"')
        output = "\"" + tokenize(output, '\"')[1] + "\"";
    else
        output = tokenize(output, ' ')[0];
    output = cut_off_spaces_from_end(output);

    if (cmd.find('<') == string::npos) 
    {
        input = "";
        lsign = false;
    }
    else
        input = tokenize(cmd, '<').back(); // setting the input

    input = cut_off_spaces_from_end(input);
    input = tokenize(input, '>')[0];
    if (input[0] == '\"')
        input = "\"" + tokenize(input, '\"')[1] + "\"";
    else
        input = tokenize(input, ' ')[0];
    input = cut_off_spaces_from_end(input);

    
    if (input == output && (lsign || rsign))
    {
        cout << "syntax error"
             << "\n";
        return {};
    }
    // syntax errors for the case when there is redirection but no file name.
    if (lsign)
    {
        if (input == "")
        {
            cout << "syntax error"
                 << "\n";
            return {};
        }
    }
    if (rsign)
    {
        if (output == "")
        {
            cout << "syntax error"
                 << "\n";
            return {};
        }
    }
    for (int i = 0; i < cmd.size(); i++)
    {
        if (cmd[i] == '<' || cmd[i] == '>')
        {
            while (i < cmd.size())
                i++;
        }
        else
            command += cmd[i];
    }
    command = cut_off_spaces_from_end(command);
    // removing quotes from the file destination
    input.erase(remove(input.begin(), input.end(), '\"'), input.end());
    output.erase(remove(output.begin(), output.end(), '\"'), output.end());
    parsed.push_back(command); // parsed[0] = command
    parsed.push_back(input);   // parsed[1] = input file
    parsed.push_back(output);  // parsed[2] = output file

    return parsed;
}
void redirect_io(string inp, string out)
{
    // Open input redirecting file
    if (inp.size())
    {
        int inp_fd = open(inp.c_str(), O_RDONLY); // Open in read only mode
        int status = dup2(inp_fd, 0);             // Redirect input
        if (inp_fd < 0)
        {
            cout << inp << ": No such file or directory" << endl;
            exit(EXIT_FAILURE);
        }
        if (status < 0)
        {
            cout << "Input redirecting error" << endl;
            exit(EXIT_FAILURE);
        }
    }
    if (out.size())
    {
        int out_fd = open(out.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // Open in create and truncate mode
        int status = dup2(out_fd, 1);                                          // Redirect output
        if (status < 0)
        {
            cout << "Output redirecting error" << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void get_pids_with_open_files(int *pid_list, const char *file_name ,int *num_pids){
    char *cmd = (char*)malloc(sizeof(char));
    cmd[0] = '\0';
    realloc(cmd, strlen(cmd) + strlen("lsof -t ") + strlen(file_name) + 1);
    strcat(cmd, "lsof -t ");
    strcat(cmd, file_name);
    FILE *fp = popen(cmd, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1) {
        realloc(pid_list, sizeof(int) * (*num_pids + 1));
        pid_list[*num_pids] = atoi(line);
        (*num_pids)++;
    }
    pclose(fp);
    if (line)
        free(line);
}

void exec_cmd(string cmd)
{
    
    cmd = cut_off_spaces_from_end(cmd);
    if (cmd[0] == 's' && cmd[1] == 'b')
    {
        int i = 2;
        while (cmd[i] == ' ')
            i++;

        string sn = "";
        while (cmd[i] >= '0' && cmd[i] <= '9')
        {
            sn += cmd[i];
            i++;
        }

        maxPID = stoi(sn);
        prevPID = 0;
        currTime = 0, maxd = INT_MIN, prevTime = 0;
        print_tree(stoi(sn));

        while (cmd[i] == ' ')
            i++;
        string r = cmd.substr(i, 8);
        if (r == "-suggest")
        {
            printf("%d is the Trojan\n", maxPID);
        }
        
        return;
    }
    if(tokenize(cmd, ' ')[0] == "delep")
    {
        string s = tokenize(cmd, ' ')[1];
        int *pid_list = (int*)malloc(sizeof(int));

        int num_pids = 0;
        get_pids_with_open_files(pid_list, s.c_str(), &num_pids);
        int flag = 0;
        if(num_pids == 0){
            printf("No processes found with open file test.txt");
        }else{
            char yes_no[10];
            printf("Do you like to kill these processes? (yes/no): ");
            scanf("%s", yes_no);
            if(strcmp(yes_no, "yes") == 0){
                flag = 1;
            }
        }

        if(flag == 1){
            for(int i = 0;i < num_pids;i++){
                int pid = pid_list[i];
                printf("%d ", pid_list[i]);
                kill(pid, SIGKILL);
            }
        }
        return;
    }
    vector<char *> args;
    int i = 0;
    for (string s : tokenize(cmd, ' '))
    {
        s.erase(remove(s.begin(), s.end(), '\"'), s.end());
        char *temp = new char[s.size() + 1];
        strcpy(temp, const_cast<char *>(s.c_str()));
        args.push_back(temp);
    }
    args.push_back(NULL);
    char **argv = &args[0];
    execvp(tokenize(cmd, ' ')[0].c_str(), argv);
}

string wildcard(string input){
    string output = "",temp = "";
    
    vector<string> parsed = tokenize(input, ' ');
    string final = "";
    int quote = 0;
    char last;
    int i = 0;
    while(i < parsed.size()){
        glob_t globbuf;
        if(parsed[i][0] == '"' || parsed[i][0] == '\''){
            quote = 1;
            last = parsed[i][0];
        }
        int wild = 0;
        for(int j = 0;j < parsed[i].size();j++){
            if(parsed[i][j] == '*'){
                if(j > 0 && parsed[i][j-1] == '\\')
                    continue;
                wild = 1;
                break;
            }
            if(parsed[i][j] == '?'){
                if(j > 0 && parsed[i][j-1] == '\\')
                    continue;
                wild = 1;
                break;
            }
        }
        if(quote)
            wild = 0;
        if(quote && parsed[i][parsed[i].size()-1] == last){
            quote = 0;
        }
        if(wild){
        glob(parsed[i].c_str(), 0, NULL, &globbuf);
        for(int i = 0; i < globbuf.gl_pathc; i++){
            final += globbuf.gl_pathv[i];
            final += " ";    
        }
        }else{
            final += parsed[i];
            final += " ";
        }
        i++;
    }
    
    return final;
}

void exec_pipe(string cmd, int status)
{
    string new_cmd;
    bool resume_pipe = false;

    if (cmd.rfind("|", 0) == 0)
    {
        cout << "syntax error\n";
        return;
    }
    vector<string> cmds = tokenize(cmd, '|');
    if (cmds.size() == 1)
    {
        vector<string> parsed = split_io(cmds[0]);
        parsed[0] = wildcard(parsed[0]);
        if (parsed.size() == 0) // returning in case of syntax error
            return;
        if (!check_valid(parsed[0]))
        {
            cout << tokenize(parsed[0], ' ')[0] << ": command not found\n";
            return;
        }
        pid_t pid = fork();
        child_pid = pid;
        if (pid == -1)
        {
            perror("Failure in creation of child process; fork() failed!");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            
            redirect_io(parsed[1], parsed[2]); // Redirect input and output if required
            exec_cmd(parsed[0]);               // Execute the command
            exit(0);                           // Exit the child process
        }
        if (!bg)
            wait(&status);
        
    }

    else 
    {
        int n = cmds.size(); // No. of pipe commands
        int newFD[2], oldFD[2];

        for (int i = 0; i < n; i++)
        {
            // For cases that erroneously end in pipe, e.g. "cat Ass2.cpp|"
            if (cut_off_spaces_from_end(cmds[i]) == "")
                continue;
            vector<string> parsed = split_io(cmds[i]);
            if (parsed.size() == 0) // returning in case of syntax error
                return;
            if (!check_valid(parsed[0]))
            {
                cout << tokenize(parsed[0], ' ')[0] << ": command not found\n";
                vector<string> new_cmds = vector<string>(cmds.begin() + i + 1, cmds.end());

                for (string s : new_cmds)
                    new_cmd += (cut_off_spaces_from_end(s) + " | ");
                new_cmd=new_cmd.substr(0, new_cmd.size() - 2);
                new_cmd = cut_off_spaces_from_end(new_cmd);
                resume_pipe = true;
                break;
            }
            // Create new pipe except for the last command
            if (i != n - 1)
                pipe(newFD);
            // Fork for every command
            pid_t pid = fork();
            // Error in forking the process
            if (pid == -1)
            {
                perror("Failure in creation of child process!");
                exit(EXIT_FAILURE);
            }
            // Inside the child process
            if (pid == 0)
            {
                // if( !i || i==n-1)
                // Read from previous command for everything except the first command
                if (i)
                    dup2(oldFD[0], 0), close(oldFD[0]), close(oldFD[1]);
                // Write into pipe for everything except last command
                if (i != n - 1)
                    close(newFD[0]), dup2(newFD[1], 1), close(newFD[1]);
                // Execute command
                redirect_io(parsed[1], parsed[2]); // For the first and last command redirect the input output files
                exec_cmd(parsed[0]);
            }
            // In parent process
            if (i)
                close(oldFD[0]), close(oldFD[1]);

            // Copy newFD into oldFD for everything except the last process
            if (i != n - 1)
                oldFD[0] = newFD[0], oldFD[1] = newFD[1];
        }
        // If no background, then wait for all child processes to return
        if (!bg)
            while (wait(&status) > 0)
                ;
    }

    if (resume_pipe) // THIS PART IS OPTIONAL IN MINE (CAN BE REMOVED)
    {
        exec_pipe(new_cmd, status);
        return;
    }
}

// Check for exit command
void check_exit(string &cmd)
{

    if (tokenize(cmd, ' ')[0] == "exit")
    {

        ofstream file;
        file.open("/home/shah/Desktop/OS/Assgn2/history.txt", ios::out);
        if (file.is_open())
        {   
            string s=history.del();
            while(s!="EOF")
            {
                file<<s<<"\n";
                //cout<<s<<"\n";
                s=history.del();
            }
            file.close(); //close the file object.
        }

        cout << "\033[1;31mExiting ...\033[0m\n";
        tcsetattr(STDIN_FILENO, TCSANOW, &tr_old);
        exit(EXIT_FAILURE);
    }
}

void signalhandler(int signum)
{
    if (signum == SIGINT)
    {
        printf("^C\n");
        return;
    }
    else if (signum == SIGTSTP)
    {
        printf("^Z\n");
        int status = 1;
        if(running && child_pid != 0){
            kill(child_pid,SIGKILL);
        }
        return;
    }
    string exit1 = "exit";
    check_exit(exit1);
}

void new_line()
{
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    printf("\033[1;32m%s\033[0m > ",cwd);
    // cout << cwd << " > ";
    return;
}

void clear_line()
{
    // ESCAPE SEQ + CTRL 2 + START OF LINE
    printf("\33[2K\r");
    return;
}

void cursorFixation(int cursor, int cursor_position)
{
    while (cursor_position > cursor)
    {
        printf("\033[D");
        cursor_position--;
    }
    while (cursor_position < cursor)
    {
        printf("\033[C");
        cursor_position++;
    }
}

int main()
{
    int status = 0;

    

    signal(SIGINT, signalhandler);  // ctrl + C
    signal(SIGTSTP, signalhandler); // ctrl + Z

    bg = false; // flag for background running

    unsigned char c;

    /* get the terminal settings for stdin */
    tcgetattr(STDIN_FILENO, &tr_old);

    /* we want to keep the old setting to restore them a the end */
    tr_new = tr_old;

    /* disable canonical mode (buffered i/o) and local echo */
    tr_new.c_lflag &= (~ICANON & ~ECHO);
    tr_new.c_cc[VMIN] = 1;
    tr_new.c_cc[VTIME] = 0;
    /* set the new settings immediately */
    tcsetattr(STDIN_FILENO, TCSANOW, &tr_new);
    
    fstream newfile;
    newfile.open("history.txt",ios::in);
    if (newfile.is_open())
    {   //checking whether the file is open
        string tp;
        while(getline(newfile, tp)){ //read data from file object and put it into string.
            history.add_cmd(tp); //print the data of the string
        }
        newfile.close(); //close the file object.
    }


    printf("\n");
    int curr = -1;
    string current_command = "";

    new_line();
    int cursor = 0, cursor_position = 0;
    while (1)
    {
        c = getchar();
        if (c == '\n')
        {
            // if ((history.history_size() > 0 && history.extract(0) == current_command))
            // {
            //     curr = -1;
            //     current_command = "";
            //     printf("\n");
            //     new_line();
            //     continue;
            // }
            if (curr == -1)
                history.add_cmd(current_command);
            else
            {
                current_command = history.extract(curr);
                history.add_cmd(current_command);
            }
            curr = -1;
            // if(current_command == "exit") {
            //     cout << "\n";
            //     tcsetattr(STDIN_FILENO, TCSANOW, &tr_old);

            //     return 0;
            // }
            printf("\n");

            // Execute(current_command);
            // Remove trailling and preceeding spaces
            current_command = cut_off_spaces_from_end(current_command);
            if (current_command.back() == '&')
            {
                bg = true;
                current_command.pop_back();
            }
            running = true;
            // Check for exit
            check_exit(current_command);
            // Change dir if needed
            cd_check(current_command);
            // Get current dir if needed
            pwd_check(current_command); // GIVING DOUBLE OUTPUT (IF USED)
            // Parse piped commands and set up child processes
            exec_pipe(current_command, status);
            running = false;
            // next_line();
            current_command = "";
            new_line();
            cursor = 0;
            cursor_position = 0;
            bg = false;
        }

        else if (c == 27)
        {
            char e1 = getchar();
            char e2 = getchar();
            if (e1 == 91)
            {
                if (e2 == 65)
                {
                    // cout << "UP\n";
                    if (curr < history.history_size() - 1)
                    {
                        clear_line();
                        curr++;
                        // cout << "\nin" << curr << "\n";
                        new_line();
                        // printf("\nincrease\n");
                        cout << history.extract(curr);
                        // cursor = history.extract(curr).size();
                    }
                }
                if (e2 == 66)
                {
                    if (curr > 0)
                    {
                        clear_line();
                        curr--;
                        // cout << "\ndec " << curr << "\n";
                        new_line();
                        cout << history.extract(curr);
                        // cursor = history.extract(curr).size();
                    }
                    else if (curr == 0)
                    {
                        clear_line();
                        curr--;
                        // cout << "\ndec2 " << curr << ":" << current_command << "\n";
                        new_line();
                        cout << current_command;
                        // cursor = current_command.size();
                    }
                }
                if (e2 == 67)
                {
                    // right arrow key
                    if (curr != -1)
                    {
                        current_command = history.extract(curr);
                        cursor = current_command.size();
                        cursor_position = current_command.size();
                        curr = -1;
                    }
                    if (cursor != current_command.size())
                    {
                        printf("\033[C");
                        cursor++;
                        cursor_position++;
                    }
                    continue;
                }
                if (e2 == 68)
                {
                    // left arrow key
                    if (curr != -1)
                    {
                        current_command = history.extract(curr);
                        cursor = current_command.size();
                        cursor_position = current_command.size();
                        curr = -1;
                    }
                    if (cursor != 0)
                    {
                        printf("\033[D");
                        cursor_position--;
                        cursor--;
                    }
                    continue;
                }
            }
        }
        else if (c == 127)
        {
            if (curr == -1)
            {
                if (cursor > 0)
                {
                    string str1 = "", str2 = "";
                    if (cursor > 1)
                        str1 = current_command.substr(0, cursor - 1);
                    if (current_command.size() != cursor)
                        str2 = current_command.substr(cursor, current_command.size() - cursor);
                    while (cursor_position != current_command.size())
                    {
                        printf("\033[C");
                        cursor_position++;
                    }
                    for (int i = 0; i < current_command.size(); i++)
                        printf("\b \b");
                    current_command = str1 + str2;

                    printf("%s", current_command.c_str());
                    cursor--;
                    // size--;
                    cursor_position = current_command.size();
                }
            }
            else
            {
                if (history.extract(curr).size() > 0)
                {
                    current_command = history.extract(curr);
                    cursor = current_command.size();
                    cursor_position = cursor;
                    curr = -1;
                    string str1 = "", str2 = "";
                    if (cursor > 1)
                        str1 = current_command.substr(0, cursor - 1);
                    if (current_command.size() != cursor)
                        str2 = current_command.substr(cursor, current_command.size() - cursor);
                    while (cursor_position != current_command.size())
                    {
                        printf("\033[C");
                        cursor_position++;
                    }
                    for (int i = 0; i < current_command.size(); i++)
                        printf("\b \b");
                    current_command = str1 + str2;

                    printf("%s", current_command.c_str());
                    cursor--;
                    // size--;
                    cursor_position = current_command.size();
                }
            }
        }
        else if (c == 1)
        {
            if (curr != -1)
            {
                current_command = history.extract(curr);
                cursor = current_command.size();
                cursor_position = current_command.size();
                curr = -1;
            }
            cursor = 0;
        }
        else if (c == 5)
        {
            if (curr != -1)
            {
                current_command = history.extract(curr);
                cursor = current_command.size();
                cursor_position = current_command.size();
                curr = -1;
            }
            cursor = current_command.size();
        }
        else
        {
            // printf("%c", c);

            if (curr != -1)
            {
                current_command = history.extract(curr);
                // for(int i = 0; i < current_command.size(); i++)
                //     printf("\b \b");
                cursor_position = current_command.size();
                cursor = current_command.size();
            }
            string str1 = "", str2 = "";
            if (cursor != 0)
                str1 = current_command.substr(0, cursor);
            if (cursor != current_command.size())
                str2 = current_command.substr(cursor, current_command.size() - cursor);
            while (cursor_position != current_command.size())
            {
                printf("\033[C");
                cursor_position++;
            }
            for (int i = 0; i < current_command.size(); i++)
                printf("\b \b");
            current_command = str1;
            current_command += c;
            current_command += str2;

            printf("%s", current_command.c_str());
            cursor++;
            // size++;
            cursor_position = current_command.size();
            // cursor++;
            curr = -1;
        }
        // cout<<current_command<<endl;
        cursorFixation(cursor, cursor_position);
        cursor_position = cursor;
    }
}