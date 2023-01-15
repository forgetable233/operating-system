#include "stdio.h"
#include "fs.h"
#include "vfs.h"
#include "time.h"

const char* file_path[4] = {"orange/test.txt", 
                            "orange/test2.txt", 
                            "orange/test3.txt", 
                            "orange/test4.txt"};


int main(int arg, char *argv[])  {
    printf("**********************************************\n");
    printf("begin to read and write using buffer\n");
    printf("[");
    int begin, end;
    begin = get_ticks();
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 1000; i++)
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
    end = get_ticks();
    printf("]");
    printf("The test has been finished!\n");
    printf("The time cost is %d \n", end - begin);
    printf("**********************************************\n\n");
    // close(fd);
    bh_refresh();
    reset_flag();
    printf("**********************************************\n");
    printf("not use buffer to read and write\n");
    printf("[");
    begin = get_ticks();
    for (int j = 0; j < 78; j ++ )
    {
        for (int i = 0; i < 1000; i++)
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
    end = get_ticks();
    printf("]");
    printf("The test has been finished!\n");
    printf("The time cost is %d \n", end - begin);
    printf("**********************************************\n\n");
    return 0;
}