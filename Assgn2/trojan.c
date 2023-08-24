#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

int main(){
    while(1){
        for(int i = 0;i < 5;i++){
            if(fork() == 0){
                for(int j = 0;j < 10;j++){
                    if(fork() == 0){
                        while(1);
                    }
                }
                while(1);
            }
        }
        sleep(120000);

    }

}