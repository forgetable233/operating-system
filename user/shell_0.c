#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"
#include "hd.h"


int main(int arg, char *argv[])
{
	int stdin = open("dev_tty0", O_RDWR);
	int stdout = open("dev_tty0", O_RDWR);
	int stderr = open("dev_tty0", O_RDWR);

	char buf[1024] = "orange/RWFile.bin";
	int pid;
	int times = 0;
	bh_refresh();
	while (1)
	{
		struct buf_head* test;
		printf("\nminiOS:/ $ ");
		if (gets(buf) && strlen(buf) != 0)
		{
			if (exec(buf) != 0)
			{
				printf("exec failed: file not found!\n");
				continue;
			}
		}
	}
}