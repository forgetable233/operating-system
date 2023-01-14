/*************************************************************************//**
 *****************************************************************************
 * @file   buffer.c
 * @brief  Hard disk buffer cache.
 * @author Lxc, Dcr
 * @date   2022
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "fs_const.h"
#include "hd.h"
#include "fs.h"
#include "fs_misc.h"
#include "x86.h"
#include "stdio.h"
#include "assert.h"


#define	DRV_OF_DEV(dev) (dev <= MAX_PRIM ? \
			 dev / NR_PRIM_PER_DRIVE : \
			 (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)
#define RD_SECT_BUF(dev,sect_nr,fsbuf) rw_sector(DEV_READ, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* read one sector */ \
				       proc2pid(p_proc_current),/*TASK_A*/			\
				       fsbuf);

#define WR_SECT_BUF(dev,sect_nr,fsbuf) rw_sector(DEV_WRITE, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* write one sector */ \
				       proc2pid(p_proc_current),				\
				       fsbuf);


static u8 buf_cache[BUF_SIZE][SECTOR_SIZE];  // 缓冲区定义
/* 缓冲块状态，
CLEAN表示缓冲块数据与磁盘数据同步，
UNUSED表示数据尚未写入，数据无效；
DIRTY表示缓冲块内为脏数据 */ 
enum buf_state{UNUSED, CLEAN, DIRTY}; 

struct buf_head
{
	int count;				// 为以后优化预留
	bool busy;            	// 该缓冲块是否被使用
	enum buf_state state; 	// 该缓冲块的状态
	int dev, block;     	// 设备号，扇区号(只有busy=true时才有效)
	void* pos;            	// 该缓冲块的起始地址, 为cache的地址
	// struct buf_head* nxt; 	// 指向下一个缓冲块头部
	// int nxt;
};

static struct buf_head bh[BUF_SIZE];
typedef struct buf_head_que
{
	struct buf_head* bh_list[BUF_SIZE + 1];
	int front, rear;
}buf_head_que;
static buf_head_que free_que;

static void push_to_free(struct buf_head* bhead)
{
	if ((free_que.rear + 1) % (BUF_SIZE + 1) == free_que.front)
	{
		panic("The free buffer cache queue is full!");
	}
	free_que.bh_list[free_que.rear] = bhead;
	free_que.rear = (free_que.rear + 1) % (BUF_SIZE + 1);
}

static struct buf_head* pop_from_free()
{
	if (free_que.front == free_que.rear)
	{
		panic("The free buffer cache queue is empty!");
	}
	struct buf_head* ret = free_que.bh_list[free_que.front];
	free_que.front = (free_que.front + 1) % (BUF_SIZE + 1);
	return ret;
}

static int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_nr, void* buf)
{
	MESSAGE driver_msg;
	
	driver_msg.type		= io_type;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= pos;
	driver_msg.CNT		= bytes;	/// hu is: 512
	driver_msg.PROC_NR	= proc_nr;
	driver_msg.BUF		= buf;

	hd_rdwt(&driver_msg);
	return 0;
}


/*****************************************************************************
 *                                init_buf
 *****************************************************************************/

void init_buf()
{
	free_que.front = free_que.rear = 0;
	for (int i = 0; i < BUF_SIZE; i ++ )
	{
		bh[i].count = 0;
		bh[i].busy = false;
		bh[i].state = UNUSED;
		bh[i].dev = 0, bh->block = 0;
		bh[i].pos = (void*)buf_cache[i];
		push_to_free(&bh[i]);
	}
}

static struct buf_head* get_free_buf(int dev, int block)
{
	if (free_que.front == free_que.rear)
		return NULL;

	struct buf_head *bhead = pop_from_free();
	bhead->dev = dev, bhead->block = block;
	bhead->busy = true;

	return bhead;
}

static void grow_buf(int dev, int block)
{
	// 如果当前没有空闲的缓冲块，则将其中一个缓冲块写入磁盘
	// bh表示即将分配给新的数据块的缓冲块，暂时先规定为第一个缓冲块
	struct buf_head* bhead;
	// bh = head->nxt;
	bhead = &bh[0];
	// 如果该缓冲块的状态为CLEAN或者UNUSED，那么没必要写入磁盘，直接分配即可
	if (bhead->state == CLEAN || bhead->state == UNUSED)
	{
		bhead->dev = dev, bhead->block = block;
		bhead->busy = true;
		bhead->state = UNUSED;
		return;
	}
	// 如果该缓冲块的状态为DIRTY，那么需要先将缓冲块中的数据写入磁盘，然后分配给新的数据块
	u8 hdbuf[512];
	memcpy(hdbuf, bhead->pos, SECTOR_SIZE);
	WR_SECT_BUF(bhead->dev, bhead->block, hdbuf);
	bhead->dev = dev, bhead->block = block;
	bhead->state = UNUSED;
	return;
}

static struct buf_head* get_buf(int dev, int block)
{
	for (int i = 0; i < BUF_SIZE; i ++ )
	{
		if (bh[i].busy == true && bh[i].dev == dev && bh[i].block == block)
		{
			return &bh[i];
		}
	}
	return NULL;
}

static struct buf_head* getblk(int dev, int block)
{
	for (;;)
	{
		struct buf_head* bh = get_buf(dev, block);
		if (bh)
			return bh;
		bh = get_free_buf(dev, block);
		if (bh)
			return bh;
		else
			grow_buf(dev, block);
	}
}

// 将(dev, block)这个数据块的数据读入到addr这个地址，读入数据的大小为size
void read_buf(void* addr, int dev, int block, int size)
{
	struct buf_head* bh;
	bh = getblk(dev, block);
	if (bh->state == CLEAN || bh->state == DIRTY)
	{
		memcpy(addr, bh->pos, size);
		return;
	}
	else if (bh->state == UNUSED)
	{
		// 先将磁盘中的数据读入到缓冲块中
		// int orange_dev = get_fs_dev(PRIMARY_MASTER, ORANGE_TYPE);
		u8 hdbuf[512];
		RD_SECT_BUF(dev, block, hdbuf);
		memcpy(bh->pos, hdbuf, SECTOR_SIZE);
		// 该缓冲块的状态更新为CLEAN
		bh->state = CLEAN;
		// 接下来从缓冲块中读取数据
		memcpy(addr, bh->pos, size);
	}
	return;
}

void write_buf(void* addr, int dev, int block, int size)
{
	struct buf_head* bh;
	bh = getblk(dev, block);
	memcpy(bh->pos, addr, size);
	bh->state = DIRTY;
	return;
}