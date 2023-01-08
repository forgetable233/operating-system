#include "stdio.h"
#include "fs.h"
#include "vfs.h"

const char* file_path = "dev_tty1/test.txt";


int main(int arg, char *argv[])  {
    printf("begin to read file\n");
    int fd = open(file_path, O_RDWR);
    if (fd == -1){
        printf("can not find the target file\n");
        return 0;
    } else {
        printf("successfully open the target file\n");
    }
    close(fd);
    return 0;
}