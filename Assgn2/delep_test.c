#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd = open("test.txt",O_WRONLY|O_CREAT,0644);
    flock(fd, LOCK_EX);
    printf("Locked\n");
    while(1){
        write(fd, "Hello\n", 6);
    }
}
