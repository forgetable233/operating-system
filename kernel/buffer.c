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
#include "time.h"

bool use_buf = true;


#define	DRV_OF_DEV(dev) (dev <= MAX_PRIM ? \
			 dev / NR_PRIM_PER_DRIVE : \
			 (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)


// #define BUF_SIZE 64
static u8 buf_cache[BUF_SIZE][SECTOR_SIZE];  // 缓冲区数据块
/* 缓冲块状态，
CLEAN表示缓冲块数据与磁盘数据同步，
UNUSED表示数据尚未写入，数据无效；
DIRTY表示缓冲块内为脏数据 */ 
enum buf_state{UNUSED, CLEAN, DIRTY}; 

// 缓冲块头部，记录了缓冲块的信息
struct buf_head
{
	int count;				// 为以后优化预留
	bool busy;            	// 该缓冲块是否被使用
	enum buf_state state; 	// 该缓冲块的状态
	int dev, block;     	// 设备号，扇区号(只有busy=true时才有效)
	void* pos;            	// 该缓冲块的起始地址, 为cache的地址
	struct buf_head* pre;   // LRU链表指针，指向LRU链表的前一个元素
	struct buf_head* nxt; 	// 指向下一个缓冲块头部
};

static struct buf_head bh[BUF_SIZE];
static struct buf_head head;            // 链表的头结点

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

void free_buf(struct buf_head* bh);


/*****************************************************************************
 *                                init_buf
 *****************************************************************************/

void init_buf()
{
	head.pre = &head, head.nxt = &head;
	free_que.front = free_que.rear = 0;
	for (int i = 0; i < BUF_SIZE; i ++ )
	{
		bh[i].count = 0;
		bh[i].busy = false;
		bh[i].state = UNUSED;
		bh[i].dev = 0, bh->block = 0;
		bh[i].pos = (void*)buf_cache[i];
		bh[i].nxt = head.nxt;
		bh[i].pre = &head;
		head.nxt->pre = &bh[i];
		head.nxt = &bh[i];
		push_to_free(&bh[i]);
	}
}

static struct buf_head* get_free_buf(int dev, int block)
{
	if (free_que.front == free_que.rear)
		return NULL;

	struct buf_head *bhead = pop_from_free();
	bhead->dev = dev, bhead->block = block;
	bhead->state = UNUSED;
	bhead->busy = true;

	return bhead;
}

static void grow_buf(int dev, int block)
{
	// 首先判断当前缓冲区中是否存在空闲缓冲块，如果存在，将该缓冲块分配给当前数据块
	struct buf_head* bhead;
	bhead = get_free_buf(dev, block);
	if (bhead) return;
	// 如果当前没有空闲的缓冲块，则将距离上次使用间隔最长的缓冲块写入磁盘
	// bh表示即将分配给新的数据块的缓冲块，从LRU链表中搜索
	// for (bhead = head.pre; bhead != &head; bhead = bhead->pre)
	// 	if (bhead->count == 0) break;
	bhead = head.pre;
	// bhead = &bh[0];
	assert(bhead != &head);
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

struct buf_head* getblk(int dev, int block)
{
	for (;;)
	{
		// 先判断缓冲区是否命中
		struct buf_head* bh = get_buf(dev, block);
		// 命中后直接返回
		if (bh) { bh->count ++; return bh; }
		// 没有命中，那么重新分配一个缓冲块
		else
			grow_buf(dev, block);
	}
}

// 释放缓冲块的使用权
static void brelse(struct buf_head* b)
{
	b->count --;
	// if (!b->count)
	{
		// 将该缓冲块从LRU链表中删除
		b->nxt->pre = b->pre;
		b->pre->nxt = b->nxt;
		// 将该缓冲块添加到链表头
		b->nxt = head.nxt;
		b->pre = &head;
		head.nxt->pre = b;
		head.nxt = b;

		rw_sector(DEV_WRITE, 
		b->dev, 
		b->block * SECTOR_SIZE, 
		SECTOR_SIZE, 
		proc2pid(p_proc_current),
		b->pos);
	}
}

// 将(dev, block)这个数据块的数据读入到addr这个地址，读入数据的大小为size
void read_buf(void* addr, int dev, int block, int size)
{
	assert(size <= 512);
	if (!use_buf) {
		// u8 hdbuf[512];
		rw_sector(	DEV_READ, 
					dev, 
					block * SECTOR_SIZE, 
					size, 
					proc2pid(p_proc_current), 
					addr);
		return;
	}
	struct buf_head* bh;
	bh = getblk(dev, block);
	if (bh->state == CLEAN || bh->state == DIRTY)
	{
		memcpy(addr, bh->pos, size);
		// free_buf(bh);
		return;
	} else if (bh->state == UNUSED) {
		// 先将磁盘中的数据读入到缓冲块中
		u8 hdbuf[512];
		RD_SECT_BUF(dev, block, hdbuf);
		memcpy(bh->pos, hdbuf, SECTOR_SIZE);
		// 该缓冲块的状态更新为CLEAN
		bh->state = CLEAN;
		// 接下来从缓冲块中读取数据
		memcpy(addr, bh->pos, size);
	}
	// brelse(bh);
	return;
}

void write_buf(void* addr, int dev, int block, int size)
{
	assert(size <= 512);
	if (!use_buf) {
		rw_sector(	DEV_WRITE, 
					dev, 
					block * SECTOR_SIZE, 
					size, 
					proc2pid(p_proc_current), 
					addr);
		return;
	}
	struct buf_head* bh;
	bh = getblk(dev, block);
	
	memcpy(bh->pos, addr, size);
	bh->state = DIRTY;
	// brelse(bh);
	return;
}

void free_buf(struct buf_head* bh) {
	assert(bh->busy == true && bh->state != UNUSED);

	bh->busy = false;
	bh->state = UNUSED;
	push_to_free(bh);
	WR_SECT_BUF(bh->dev, bh->block, bh->pos);
}

void Free_buf(int dev, int block) {
	struct buf_head* bh = getblk(dev, block);
	if (bh->state == UNUSED) {
		push_to_free(bh);
		return;
	} else {
		free_buf(bh);
		return;
	}
}

// 清空缓冲区并写入硬盘
void refresh_buf() {
	kprintf("enter refresh buf\n");
	for (int i = 0; i < BUF_SIZE; i++) {
		struct buf_head* temp = bh + i;
		free_buf(temp);
		// if (bh[i].busy){
		// 	bh[i].busy = false;
		// 	bh[i].state = UNUSED;

		// 	push_to_free(bh + i);
		// 	WR_SECT_BUF(bh[i].dev, bh[i].block, bh[i].pos);
		// 	kprintf("\nwrite hd\n");
		// }
	}
}

int do_refresh() {
	for (int i = 0; i < BUF_SIZE; i++) {
		if (bh[i].busy) {
			brelse(&bh[i]);
		}
	}
	// kprintf("\n*************finish refresh*************\n");
	return 0;
}

int sys_bh_refresh() {
	
	return do_refresh();
}
int do_reset_flag() {
	use_buf = false;
	// kprintf("\n*************the flag is false*************\n");
	return 0;
}

int sys_reset_flag(){
	return do_reset_flag();
}
