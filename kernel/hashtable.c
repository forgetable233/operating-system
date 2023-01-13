/*************************************************************************//**
 *****************************************************************************
 * @file   hashtable.c
 * @brief  This hash table is provided for buffer cache.
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

#define M 1000

struct HashNode
{
    struct sect
    {
        int dev, block;
    }key;
    int value;
    int nxt;
}data[BUF_SIZE];

int head[M], size;

int f(struct sect key) { return (key.block % M); }

int get_value(struct sect key)
{
    for (int p = head[f(key)]; p; p = data[p].nxt)
        if (data[p].key.dev == key.dev && data[p].key.block == key.block) return data[p].value;
    return -1;
}

int modify(struct sect key, int value)
{
    for (int p = head[f(key)]; p; p = data[p].nxt)
        if (data[p].key.dev == key.dev && data[p].key.block == key.block) return data[p].value = value;
}

int add_key(struct sect key, int value)
{
    if (get_value(key) != -1) return -1;
    data[++ size] = (struct Node){key, value, head[f(key)]};
    head[f(key)] = size;
    return value;
}