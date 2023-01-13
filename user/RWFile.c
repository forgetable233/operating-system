#include "stdio.h"
#include "fs.h"
#include "vfs.h"

const char* file_path[4] = {"dev_tty0/test.txt", 
                            "dev_tty0/test2.txt", 
                            "dev_tty0/test3.txt", 
                            "dev_tty0/test4.txt"};


int main(int arg, char *argv[])  {
    printf("begin to read file\n");
    // int fd = open(file_path, O_RDWR);

    // __clock_t begin = 
    // if (fd == -1){
    //     printf("error occurred can not find the target file\n");
    //     return 0;
    // } else {
    //     printf("successfully open the target file\n");
    // }

    // int fd_2 = open(file_path, O_RDWR);
    // if (fd_2 == -1) {
    //     printf("error occurred\n")        ;
    // } else {
    //     printf("second open the file found the target\n");
    // }
    
    for (int i = 0; i < 10; i++)
    {
        int tar = i % 4;
        int fd = open(file_path[tar], O_RDWR);
        if (fd != -1)
        {
            printf("success\n");
        } else {
            printf("error \n");
        }        
    }
    
    // close(fd);
    return 0;
}