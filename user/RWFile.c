#include "stdio.h"
#include "fs.h"
#include "vfs.h"

const char* file_path[4] = {"dev_tty0/test.txt", 
                            "dev_tty0/test2.txt", 
                            "dev_tty0/test3.txt", 
                            "dev_tty0/test4.txt"};


int main(int arg, char *argv[])  {
    printf("[");
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 400; i++)
        {
            int tar = i % 4;
            char read_buf[128];
            char write_buf[128] = "Never gonna give you up!";

            int fd = open(file_path[tar], O_RDWR);
            read(fd, read_buf, 128);
            // printf("%s\n", read_buf);
            close(fd);

            fd = open(file_path[tar], O_RDWR);
            write(fd, write_buf, 128);
            close(fd);

            fd = open(file_path[tar], O_RDWR);
            read(fd, read_buf, 128);
            close(fd);
            // printf("%s\n", read_buf); 
        }
        printf("*");
    }
    printf("]");
    printf("The test has been finished!\n");
    
    // close(fd);
    return 0;
}