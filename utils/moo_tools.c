//
// Created by mooki on 10/27/21.
//

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "moo_utils.h"


/* 目录遍历 */
// 显示目录每个目录项
int showDirItems(FILE *file, const Inode *dirInode) {
    DirItem dirItem;
    uint16_t dirItemNum = dirInode->size / sizeof(DirItem);
    for(uint16_t i = 0; i < dirItemNum; ++i){
        uint16_t idxMaj = i / DIRITEMPERBLK;
        uint16_t idxMin = i % DIRITEMPERBLK;
        if(readDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj] + sizeof(DirItem) * idxMin, &dirItem) != 0){
            errToRet("showDirItems", ERR_READ);
            return -1;  // 读取目录项失败
        }
        printf("%d  %s\n", dirItem.inodeIdx, dirItem.name);
     }
    return 0;
}

// 检查目录中是否有对应项
int chkItemInDir(FILE *file, const Inode* dirInode, const char *name){
    DirItem dirItem;
    uint16_t dirItemNum = dirInode->size / sizeof(DirItem);
    for(uint16_t i = 0; i < dirItemNum; ++i){
        uint16_t idxMaj = i / DIRITEMPERBLK;
        uint16_t idxMin = i % DIRITEMPERBLK;
        if(readDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj] + sizeof(DirItem) * idxMin, &dirItem) != 0){
            errToRet("chkItemInDir", ERR_READ);
            return -1;  // 读取目录项失败
        }
        if(strcmp(dirItem.name, name) == 0){
            return 1;  // 找到对应项
        }
    }
    return 0;  // 不存在对应项
}

// 在目录中搜索指定项
int findItemInDir(FILE *file, const Inode *dirInode, const char *name, DirItem *dirItem) {
    uint16_t dirItemNum = dirInode->size / sizeof(DirItem);
    for(uint16_t i = 0; i < dirItemNum; ++i){
        uint16_t idxMaj = i / DIRITEMPERBLK;
        uint16_t idxMin = i % DIRITEMPERBLK;
        if(readDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj] + sizeof(DirItem) * idxMin, dirItem) != 0){
            errToRet("findItemInDir", ERR_READ);
            return -1;  // 读取目录项失败
        }
        if(strcmp(dirItem->name, name) == 0){
                return 1;  // 找到对应项
        }
    }
    return 0;  // 不存在对应项
}

/* 设置偏移量 */
int setOffset(FILE* file, uint32_t offset){
    if(fseek(file, offset, SEEK_SET) != 0){
        errToRet("setOffset", ERR_SETCURSOR);
        return -1;
    }
    return 0;
}

/* 更新超级块 */
int updateSB(FILE* file, const SuperBlk* superBlk){
    if(setOffset(file, SBBEG) != 0){
        return -1;
    }
    if(fwrite(superBlk, sizeof(SuperBlk), 1, file) != 1){
        errToRet("updateSB", ERR_UPDATESB);
        return -1;
    }
    return 0;
}

/* 更新位图 */
int updateBmp(FILE* file, uint32_t offset, const uint32_t* bmp,  uint16_t size){
    if(setOffset(file, offset) != 0){
        return -1;
    }
    if(fwrite(bmp, size, 1, file) != 1){
        return -1;
    }
    return 0;
}

/* i节点读写 */
int readInode(FILE* file, uint32_t offset, Inode* inode){
    if(setOffset(file, offset) != 0){
        return -1;
    }
    if(fread(inode, sizeof(Inode), 1, file) != 1){
        errToRet("readInode", ERR_READ);
        return -1;
    }
    return 0;
}

int writeInode(FILE* file, uint32_t offset, const Inode* inode){
    if(setOffset(file, offset) != 0){
        return -1;
    }
    if(fwrite(inode, sizeof(Inode), 1, file) != 1){
        errToRet("writeInode", ERR_WRITE);
        return -1;
    }
    return 0;
}

/* 目录项读写 */
// 读指定目录项
int readDirItem(FILE* file, uint32_t offset, DirItem* dirItem){
    if(setOffset(file, offset) != 0){
        return -1;
    }
    if(fread(dirItem, sizeof(DirItem), 1, file) != 1){
        errToRet("readDirItem", ERR_READ);
        return -1;
    }
    return 0;
}

// 在指定位置写目录项
int writeDirItem(FILE* file, uint32_t offset, const DirItem* dirItem){
    if(setOffset(file, offset) != 0){
        return -1;
    }
    if(fwrite(dirItem, sizeof(DirItem), 1, file) != 1){
        errToRet("writeDirItem", ERR_WRITEDIRITEM);
        return -1;
    }
    return 0;
}

// 删除指定目录项
int delDirItem(FILE *file, Inode *dirInode, uint16_t dirInodeIdx, const char *name) {
    DirItem dirItem;
    uint16_t dirItemNum = dirInode->size / sizeof(DirItem);
    for (uint16_t i = 0; i < dirItemNum; ++i) {
        uint16_t idxMaj = i / DIRITEMPERBLK;
        uint16_t idxMin = i % DIRITEMPERBLK;
        if (readDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj] + sizeof(DirItem) * idxMin, &dirItem) != 0) {
            return -1;  // 读取目录项失败
        }
        if (strcmp(dirItem.name, name) == 0) {
            // 调整父目录目录项
            for (uint16_t j = i; j < dirItemNum - 1; ++j) {
                // 目标地址
                uint16_t idxMaj_a = j / DIRITEMPERBLK;
                uint16_t idxMin_a = j % DIRITEMPERBLK;
                // 源地址
                uint16_t idxMaj_b = (j + 1) / DIRITEMPERBLK;
                uint16_t idxMin_b = (j + 1) % DIRITEMPERBLK;
                if (readDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj_b] + sizeof(DirItem) * idxMin_b,
                                &dirItem) != 0) {
                    return -1;  // 读取目录项失败
                }
                if (writeDirItem(file, BLKBEG + BLKSIZE * dirInode->blkIdx[idxMaj_a] + sizeof(DirItem) * idxMin_a,
                                 &dirItem) != 0) {
                    return -1;  // 写入目录项失败
                }
            }
            break;
        }
    }
    // 更新父目录
    dirInode->size -= sizeof(DirItem);
    writeInode(file, INDBEG + sizeof(Inode) * dirInodeIdx, dirInode);

    return 0;
}

// 添加新的目录项
int addDirItem(FILE* file, Inode* dirInode, uint32_t dirInodeIdx, const DirItem* dirItem){
    uint32_t dirItemNum = dirInode->size / sizeof(DirItem);
    uint32_t blkIdxMaj = dirItemNum / DIRITEMPERBLK;
    uint32_t blkIdxMin = dirItemNum % DIRITEMPERBLK;

    // 写入目录项
    if(setOffset(file, BLKBEG + BLKSIZE * dirInode->blkIdx[blkIdxMaj] + sizeof(DirItem) * blkIdxMin) != 0){
        return -1;
    }
    if(fwrite(dirItem, sizeof(DirItem), 1, file) != 1){
        errToRet("addDirItem", ERR_ADDDIRITEM);
        return -1;
    }
    dirInode->size += sizeof(DirItem);

    // 更新父目录
    if(writeInode(file, INDBEG + sizeof(Inode) * dirInodeIdx, dirInode) != 0){
        return -1;
    }

    return 0;
}

/* 位图操作 */
// 遍历位图寻找空闲位
uint32_t findZeroInBmp(const uint32_t* beg, uint32_t range){
    for(uint32_t i = 0; i < range; ++i){
        uint32_t idxMaj = i / BITNUMOFBMP;
        uint32_t idxMin = i % BITNUMOFBMP;
        if(~beg[idxMaj] & (1 << idxMin)){
            return i;
        }
    }

    return 0;
}

// 位图检查位
int chkBit(void *beg, uint32_t idx){
    uint32_t idxMaj = idx / BITNUMOFBMP;
    uint32_t idxMin = idx % BITNUMOFBMP;
    if((((uint32_t*)beg)[idxMaj] & (1 << idxMin)) != 0){
        return 1;
    }
    return 0;
}

// 位图置位
void setBit(void *beg, uint32_t idx){
    uint32_t idxMaj = idx / BITNUMOFBMP;
    uint32_t idxMin = idx % BITNUMOFBMP;
    ((uint32_t*)beg)[idxMaj] |= 1 << idxMin;
}

// 位图清除位
void clrBit(void *beg, uint32_t idx){
    uint32_t idxMaj = idx / BITNUMOFBMP;
    uint32_t idxMin = idx % BITNUMOFBMP;
    ((uint32_t*)beg)[idxMaj] &= ~(1 << idxMin);
}

/* 新建文件或目录 */
void
newItem(FILE *file, Inode *dirInode, uint32_t dirInodeIdx, const char *name, enum ITEMTYPE type, SuperBlk *superBlk,
        uint32_t *indBitmap, uint32_t *blkBitmap) {
    // 判断是否重名
    if(chkItemInDir(file, dirInode, name) == 1) {
        infoToRet("newFile", INFO_ALREADYEXITS);
        return;
    }

    // 判断当前目录是否有空闲空间保存目录项
    if(dirInode->size >= BLKSIZE * IDXBLKNUMININODE){
        infoToRet("newFile", INFO_NOSPACEFORDIRITEM);
        return;
    }

    // 申请i节点
    if(superBlk->indNumFree == 0){
        infoToRet("newFile", INFO_NOFREEINODE);
        return;
    }
    uint16_t newInodeIdx;
    if((newInodeIdx = (uint16_t)findZeroInBmp(indBitmap, superBlk->indNum)) == 0){
        errToRet("newFile", ERR_INDBMPCONSISTENCY);
        return;
    }
    --superBlk->indNumFree;  // 更新超级块
    setBit(indBitmap, newInodeIdx);  // 更新i节点位图

    /* 申请数据块 */
    uint32_t newBlkIdxForFatherDir;  // 存放父目录目录项
    uint32_t newBlkIdxForNewDir;  // 存放新目录目录项

    // 判断父目录是否需要新申请一个数据块用于新增目录项的存放
    int newBlkForParentDirFlag = 0;
    uint32_t oldBlkNum = dirInode->size / sizeof(DirItem) / DIRITEMPERBLK;  // 原来存放目录项所用的数据块数量
    uint32_t newBlkNum = (dirInode->size + sizeof(DirItem)) / sizeof(DirItem) / DIRITEMPERBLK;  // 新增目录项后
    if(oldBlkNum != newBlkNum){
        newBlkForParentDirFlag = 1;
    }

    if(type == iSFile){  // newFile
        if(newBlkForParentDirFlag == 1){
            if(superBlk->blkNumFree < 1){
                    infoToRet("newItem", INFO_NOENOUGHFREEBLK);
                    return;
            }

            if((newBlkIdxForFatherDir = findZeroInBmp(blkBitmap, superBlk->blkNum)) == 0){
                errToRet("newIterm", ERR_BLKBMPCONSISTENCY);
                return;
            }
            --superBlk->blkNumFree;
            setBit(blkBitmap, superBlk->blkNum);
        }
    }
    else {  // newDir
        if (newBlkForParentDirFlag == 0) {
            if (superBlk->blkNumFree < 1) {
                infoToRet("newFile", INFO_NOENOUGHFREEBLK);
                return;
            }

            if((newBlkIdxForNewDir = findZeroInBmp(blkBitmap, superBlk->blkNum)) == 0){
                errToRet("newFile", ERR_BLKBMPCONSISTENCY);
                return;
            }
            --superBlk->blkNumFree;
            setBit(blkBitmap, newBlkIdxForNewDir);
        }
        else {
            if(superBlk->blkNumFree < 2){
                infoToRet("newFile", INFO_NOENOUGHFREEBLK);
                return;
            }

            if((newBlkIdxForFatherDir = findZeroInBmp(blkBitmap, superBlk->blkNum)) == 0){
                errToRet("newFile", ERR_BLKBMPCONSISTENCY);
                return;
            }
            --superBlk->blkNumFree;
            setBit(blkBitmap, newBlkIdxForFatherDir);

            if((newBlkIdxForNewDir = findZeroInBmp(blkBitmap, superBlk->blkNum)) == 0){
                errToRet("newFile", ERR_BLKBMPCONSISTENCY);
                return;
            }
            --superBlk->blkNumFree;
            setBit(blkBitmap, newBlkIdxForNewDir);
        }
    }


    /* 写到磁盘上 */
    // 更新超级块
    if(updateSB(file, superBlk) != 0){
        return;
    }

    // 更新i节点位图
    if(updateBmp(file, INDBMPBEG, indBitmap, superBlk->indNum / 8) != 0){
        return;
    }

    // 更新数据块位图
    if(type == iSFile){
        if(newBlkForParentDirFlag == 1){
            if(updateBmp(file, BLKBMPBEG, blkBitmap, superBlk->blkNum / 8) != 0){
                return;
            }
        }
    }
    else {
        if(updateBmp(file, BLKBMPBEG, blkBitmap, superBlk->blkNum / 8) != 0) {
            return;
        }
    }

    // 写入新i节点
    Inode newInode;
    newInode.size = 0;
    newInode.type = type;
    if(type == iSDir) {
        newInode.blkIdx[0] = newBlkIdxForNewDir;
    }
    if(writeInode(file, INDBEG + sizeof(Inode) * newInodeIdx, &newInode) != 0){
        return;
    }

    // 在父目录写入新的目录项
    DirItem newDirItem;
    strcpy(newDirItem.name, name);
    newDirItem.inodeIdx = newInodeIdx;
    if(addDirItem(file, dirInode, dirInodeIdx, &newDirItem) != 0){
        return;
    }

    // 在新目录写入新的目录项
    if(type == iSDir){
        DirItem tmpDirItem;
        // 写入"."
        strcpy(tmpDirItem.name, ".");
        tmpDirItem.inodeIdx = newInodeIdx;
        addDirItem(file, &newInode, newInodeIdx, &tmpDirItem);

        // 写入".."
        strcpy(tmpDirItem.name, "..");
        tmpDirItem.inodeIdx = dirInodeIdx;
        addDirItem(file, &newInode, newInodeIdx, &tmpDirItem);
    }

    // 更新父i节点
    if(newBlkForParentDirFlag == 1){
        dirInode->blkIdx[newBlkNum] = newBlkIdxForFatherDir;
    }
    if(writeInode(file, INDBEG + sizeof(Inode) * dirInodeIdx, dirInode) != 0){
        return;
    }

    fflush(file);
}

/* 释放ind */
void freeInode(FILE *file, uint16_t inodeIdx, SuperBlk *superBlk, uint32_t *indBitmap) {
    // 更新超级块
    superBlk->blkNumFree += 1;
    updateSB(file, superBlk);

    // 更新inode位图
    clrBit(indBitmap, inodeIdx);
    updateBmp(file, INDBMPBEG, indBitmap, superBlk->indNum / 8);
}

/* 释放ind所持有的blk */
void freeBlkOfInode(FILE *file, Inode *inode, SuperBlk *superBlk, uint32_t *blkBitmap) {
    // 更新超级块
    uint32_t blkNum = ceil((double)inode->size / BLKSIZE);
    superBlk->blkNumFree += blkNum;
    updateSB(file, superBlk);

    // 更新blk位图
    uint32_t blkIdx;
    for(uint32_t i = 0; i < blkNum; ++i){
        blkIdx = inode->blkIdx[i];
        clrBit(blkBitmap, blkIdx);
    }
    updateBmp(file, BLKBMPBEG, blkBitmap, superBlk->blkNum / 8);
}