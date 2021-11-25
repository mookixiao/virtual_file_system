//
// Created by mooki on 10/18/21.
//

#ifndef VIRTUAL_FILE_SYSTEM_MOO_FS_H
#define VIRTUAL_FILE_SYSTEM_MOO_FS_H

#include <stdint.h>

// 文件系统基本信息
#define VDISKFILE "vdisk"  // 用于模拟文件系统的文件名称
#define BLKNUM 1024       // 数据块数量
#define BLKSIZE 1024      // 数据块大小
#define INDNUM 128        // i节点数量

// 超级块
typedef struct {  // 16字节
    // 数据块
    uint32_t blkNum;
    uint32_t blkNumFree;
    uint16_t blkSize;

    // i节点
    uint16_t indNum;
    uint16_t indNumFree;
    uint16_t indSize;
} SuperBlk;
#define SBBEG 0
#define SBEND sizeof(SuperBlk)

// 文件系统描述
typedef struct {  // 64字节
    char label[48];
    // i节点区
    uint32_t indBeg;
    uint32_t indEnd;

    // 数据块区
    uint32_t blkBeg;
    uint32_t blkEnd;
} FSInfo;
#define FSINFOBEG SBEND
#define FSINFOEND (FSINFOBEG + sizeof(FSInfo))

/* 位图区，0表示空闲，1表示占用 */
#define BITNUMOFBMP 32
// i节点位图
uint32_t IndBitmap[INDNUM / 32];  // 16字节
#define INDBMPBEG FSINFOEND
#define INDBMPEND (INDBMPBEG + sizeof(IndBitmap))

// 数据块位图
uint32_t BlkBitmap[BLKNUM / 32];  // 128字节
#define BLKBMPBEG INDBMPEND
#define BLKBMPEND (BLKBMPBEG + sizeof(BlkBitmap))

// i节点区
#define IDXBLKNUMININODE 14
enum ITEMTYPE{iSFile, iSDir};
typedef struct {  // 64字节
    uint32_t size;
    enum ITEMTYPE type;  // 4字节
    uint32_t blkIdx[IDXBLKNUMININODE];  // 数据块直接索引
} Inode;
#define INDBEG BLKBMPEND
#define INDEND (INDBEG + sizeof(Inode) * INDNUM)

// 数据块区
#define BLKBEG INDEND
#define BLKEND (BLKBEG + BLKNUM * BLKSIZE)

// 目录项
typedef struct {  // 32字节
    char name[28];
    uint32_t inodeIdx;
} DirItem;
#define DIRITEMPERBLK (BLKSIZE / sizeof(DirItem))

#endif //VIRTUAL_FILE_SYSTEM_MOO_FS_H
