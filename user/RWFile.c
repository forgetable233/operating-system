#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"
#include "hd.h"

const char* file_path[4] = {"dev_tty0/test.txt", 
                            "dev_tty0/test2.txt", 
                            "dev_tty0/test3.txt", 
                            "dev_tty0/test4.txt"};


const char* test = "aaabbbccc";
const char* test1 = "This is a test file for hd; This is first";

int main(int arg, char *argv[])  {
    printf("begin to read file\n");
    
    // for (int i = 0; i < 10; i++)
    // {
    //     int tar = i % 4;
    //     int fd = open(file_path[tar], O_RDWR);
    //     if (fd != -1)
    //     {
    //         printf("success\n");
    //     } else {
    //         printf("error \n");
    //     }      
    //     close(fd);  
    // }
    // int fd = 0;
    char readbuf[15];
    int fd = open(file_path[0], O_RDWR);
    read(fd, readbuf, 13);
    for (int i = 0; i < 13; i++)
    {
        printf("%c", readbuf[i]);
    }
    printf("\n");
    int fd1 = open(file_path[0], O_RDWR);
    write(fd1, test, 9);
    int fd2 = open(file_path[0], O_RDWR);
    read(fd2, readbuf, 9);
    for (int i = 0; i < 13; i++)
    {
        printf("%c", readbuf[i]);
    }
    // read();
    return 0;
}