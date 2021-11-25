//
// Created by mooki on 10/22/21.
//

#ifndef VIRTUAL_FILE_SYSTEM_MOO_UTILS_H
#define VIRTUAL_FILE_SYSTEM_MOO_UTILS_H

#include <stdint.h>

#include "moo_fs.h"

/* Info */
// 不支持该类型
#define INFO_NOTDIR "Entity with this name is not a dir. "
#define INFO_NOTFILE "Entity with this name is not a file. "

// 指定名称项已存在或不存在
#define INFO_ALREADYEXITS "Entity with this name already exists. "
#define INFO_NOTEXIT "Entity with this name do not exist. "

// 资源不足
#define INFO_NOFREEINODE "No free inode. "
#define INFO_NOENOUGHFREEBLK "No enough free block. "
#define INFO_NOSPACEFORDIRITEM "No space to new dir item. "

/* Error */
// 一般性错误
#define ERR_SETCURSOR "Set cursor offset failed. "
#define ERR_READ "Read failed. "
#define ERR_WRITE "Write failed. "

// 更新控制信息错误
#define ERR_UPDATESB "Update SuperBlk failed. "
#define ERR_UPDATEFSINFO "Update FSInfo failed. "
#define ERR_UPDATEINDBMP "Update IndBitmp failed. "
#define ERR_UPDATEBLKBMP "Update BlkBitmp failed. "
#define ERR_ADDDIRITEM "Add DirItem failed. "
#define ERR_WRITEDIRITEM "Write DirItem failed. "

// 严重错误
#define ERR_INDBMPCONSISTENCY "SuperBlk and IndBitmap do not match, file system may be damaged. "
#define ERR_BLKBMPCONSISTENCY "SuperBlk and BlkBitmap do not match, file system may be damaged. "

/* Info and Error */
void infoToRet(const char* label, const char* info);
void errToRet(const char* label, const char* err);
void errToExit(const char* label, const char* err);

/* Tools */
int updateSB(FILE* file, const SuperBlk* superBlk);
int updateBmp(FILE* file, uint32_t offset, const uint32_t* bmp, uint16_t size);

int readInode(FILE* file, uint32_t offset, Inode* inode);
int writeInode(FILE* file, uint32_t offset, const Inode* inode);

int readDirItem(FILE *file, uint32_t offset, DirItem* dirItem);
int addDirItem(FILE* file, Inode* dirInode, uint32_t dirInodeIdx, const DirItem* dirItem);
int delDirItem(FILE *file, Inode *dirInode, uint16_t dirInodeIdx, const char *name);

uint32_t findZeroInBmp(const uint32_t* beg, uint32_t range);
int chkBit(void *beg, uint32_t idx);
void setBit(void *beg, uint32_t idx);
void clrBit(void *beg, uint32_t idx);

enum traverseMode{showItemName, findByName};  // 遍历模式，{输出目录项名字，按名字索引}
int showDirItems(FILE *file, const Inode *dirInode);
int chkItemInDir(FILE *file, const Inode *dirInode, const char* name);
int findItemInDir(FILE *file, const Inode *dirInode, const char *name, DirItem *dirItem);

void
newItem(FILE *file, Inode *dirInode, uint32_t dirInodeIdx, const char *name, enum ITEMTYPE type, SuperBlk *superBlk,
        uint32_t *indBitmap, uint32_t *blkBitmap);
void freeInode(FILE *file, uint16_t inodeIdx, SuperBlk *superBlk, uint32_t *indBitmap);
void freeBlkOfInode(FILE *file, Inode *inode, SuperBlk *superBlk, uint32_t *blkBitmap);

#endif //VIRTUAL_FILE_SYSTEM_MOO_UTILS_H