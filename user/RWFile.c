#include "stdio.h"
#include "fs.h"
#include "vfs.h"

const char* file_path[4] = {"dev_tty0/test.txt", 
                            "dev_tty0/test2.txt", 
                            "dev_tty0/test3.txt", 
                            "dev_tty0/test4.txt"};


int main(int arg, char *argv[])  {
    // printf("begin to read file\n");
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
    
    for (int i = 0; i < 4; i++)
    {
        int tar = i % 4;
        char read_buf[128];
        char write_buf[128] = "Never gonna give you up!";

        int fd = open(file_path[tar], O_RDWR);
        read(fd, read_buf, 128);
        printf("%s\n", read_buf);

        fd = open(file_path[tar], O_RDWR);
        write(fd, write_buf, 128);

        fd = open(file_path[tar], O_RDWR);
        read(fd, read_buf, 128);
        printf("%s\n", read_buf); 
    }
    
    // close(fd);
    return 0;
}