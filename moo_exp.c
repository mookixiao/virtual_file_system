//
// Created by mooki on 10/22/21.
//

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/moo_fs.h"
#include "utils/moo_utils.h"

FILE *vdisk;
SuperBlk superBlk;
FSInfo fsInfo;
uint32_t indBitmap[INDNUM / BITNUMOFBMP];
uint32_t blkBitmap[BLKNUM / BITNUMOFBMP];

char* cmdArr[] = {"mkfs", "ls", "cd", "touch", "rm", "mkdir", "rmdir", "quit"};
int cmdNum = 8;

char curPath[40] = "~";
uint32_t curDirInodeIdx;
Inode curDirInode;

// cmd prompt
void showCMDPrompt(){
    printf("MOO_VDFS:%s $ ", curPath);
}

// ls
void showDir(){
    showDirItems(vdisk, &curDirInode);
}

// cd
void changeDir(char* dir){
    // 寻找目录项
    DirItem dirItem;
    if(findItemInDir(vdisk, &curDirInode, dir, &dirItem) == 0){
        infoToRet("changeDir", INFO_NOTEXIT);
        return;
    }

    // 读取目录项i节点
    Inode inode;
    if(readInode(vdisk, INDBEG + sizeof(Inode) * dirItem.inodeIdx, &inode) != 0){
        return;
    }

    // 检验目录项是不是一个目录
    if (inode.type != iSDir) {
        infoToRet("changeDir", INFO_NOTDIR);
        return;
    }

    // 更新当前目录节点、当前目录节点索引、当前路径
    curDirInode = inode;
    curDirInodeIdx = dirItem.inodeIdx;
    strcpy(curPath, dir);
}

// touch
void newFile(const char* file){
    newItem(vdisk, &curDirInode, curDirInodeIdx, file, iSFile, &superBlk, indBitmap, blkBitmap);

    fflush(vdisk);
}

// rm
void rmFile(const char* file){
    // 遍历目录
    DirItem dirItem;
    if(findItemInDir(vdisk, &curDirInode, file, &dirItem) != 1){
        infoToRet("rmFile", INFO_NOTEXIT);
        return;
    }

    // 读取目录项i节点
    Inode inode;
    if(readInode(vdisk, INDBEG + sizeof(Inode) * dirItem.inodeIdx, &inode) != 0){
        return;
    }

    // 检验目录项是不是一个文件
    if (inode.type != iSFile) {
        infoToRet("rmFile", INFO_NOTFILE);
        return;
    }

    // 更新超级块
    ++superBlk.indNumFree;
    if(updateSB(vdisk, &superBlk) != 0){
        return;
    }

    // 更新i节点位图
    clrBit(indBitmap, dirItem.inodeIdx);
    if(updateBmp(vdisk, INDBMPBEG, indBitmap, sizeof(indBitmap)) != 0){
        return;
    }

    // 删除目录项
    if(delDirItem(vdisk, &curDirInode, curDirInodeIdx, file) != 0){
        return;
    }

    fflush(vdisk);
}

// newDir
void newDir(const char* dir){
    newItem(vdisk, &curDirInode, curDirInodeIdx, dir, iSDir, &superBlk, indBitmap, blkBitmap);
}

// rm
int rmItemRecur(Inode* dirInode, uint16_t dirInodeIdx, DirItem* dirItem){  // 父目录i节点，父目录i节点索引，要删除的目录项
    // 读取要删除目录项i节点
    Inode itemInode;
    if(readInode(vdisk, INDBEG + sizeof(Inode) * dirItem->inodeIdx, &itemInode) != 0){
        return -1;
    }

    if(itemInode.type == iSFile) {  // 如果目录项是一个文件
        // 释放blk
        freeBlkOfInode(vdisk, &itemInode, &superBlk, blkBitmap);
        // 释放inode
        freeInode(vdisk, dirItem->inodeIdx, &superBlk, indBitmap);
        return 0;
    }
    else{  // 如果目录项是一个目录
        for(uint32_t i = 0; i < ceil((double)itemInode.size / sizeof(DirItem)); ++i){  // 遍历新发现目录各项
            DirItem tmpDirItem;
            uint16_t idxMaj = i / DIRITEMPERBLK;
            uint16_t idxMin = i % DIRITEMPERBLK;
            if(readDirItem(vdisk, BLKBEG + BLKSIZE * itemInode.blkIdx[idxMaj] + sizeof(DirItem) * idxMin, &tmpDirItem) != 0){
                errToRet("rmItemRecur", ERR_READ);
                return -1;
            }
            if(strcmp(tmpDirItem.name, ".") == 0 || strcmp(tmpDirItem.name, "..") == 0){  // 跳过"."和".."
                continue;
            }
            rmItemRecur(&itemInode, dirItem->inodeIdx, &tmpDirItem);  // 进入下一次递归
        }
        // 释放blk
        freeBlkOfInode(vdisk, &itemInode, &superBlk, blkBitmap);
        // 释放inode
        freeInode(vdisk, dirItem->inodeIdx, &superBlk, indBitmap);
        // 删除目录项
        delDirItem(vdisk, dirInode, dirInodeIdx, dirItem->name);
        return 0;
    }
}

void rmDir(const char* dir){
    // 遍历目录
    DirItem dirItem;
    if(findItemInDir(vdisk, &curDirInode, dir, &dirItem) == 0){
        infoToRet("rmDir", INFO_NOTEXIT);
        return;
    }

    // 读取目录项i节点
    Inode inode;
    if(readInode(vdisk, INDBEG + sizeof(Inode) * dirItem.inodeIdx, &inode) != 0){
        return;
    }

    // 检验目录项是不是一个目录
    if(inode.type != iSDir) {
        errToRet("rmDir", INFO_NOTDIR);
        return;
    }

    // 递归删除
    rmItemRecur(&curDirInode, curDirInodeIdx, &dirItem);

    fflush(vdisk);
}


int main(int argc, char *argv[]){
    // 打开虚拟磁盘文件
    if((vdisk = fopen(VDISKFILE, "r+")) == NULL) {
        printf("Open virtual disk file %s failed. \n", VDISKFILE);
        return 0;
    }

    // 读超级块
    if(fread(&superBlk, sizeof(SuperBlk), 1, vdisk) != 1){
        printf("Read SuperBlock failed. \n");
        return 0;
    }

    // 读文件系统描述
    if(fread(&fsInfo, sizeof(FSInfo), 1, vdisk) != 1){
        printf("Read FSInfo failed. \n");
        return 0;
    }

    // 读I节点位图
    if(fread(indBitmap, sizeof(indBitmap), 1, vdisk) != 1){
        printf("Read indBitmap failed. \n");
    }

    // 读数据块位图
    if(fread(blkBitmap, sizeof(blkBitmap), 1, vdisk) != 1){
        printf("Read blkBitmap failed. \n");
    }

    // 更新当前路径信息
    curDirInodeIdx = 0;
    if(fseek(vdisk, INDBEG + sizeof(Inode) * curDirInodeIdx, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fread(&curDirInode, sizeof(Inode), 1, vdisk) != 1){
        printf("Read CurDirInode failed. \n");
    }

    // 进入读取命令循环
    char cmd[16];
    char subCMD[16];
    while(1){
        showCMDPrompt();
        scanf("%s", cmd);

        int idx = -1;
        for(int i = 0; i < cmdNum; ++i){
            if(strcmp(cmd, cmdArr[i]) == 0){
                idx = i;
                break;
            }
        }

        switch(idx){
            case 0:  // mkfs
                system("./moo_mkfs");
                break;
            case 1:  // ls
                showDir();
                break;
            case 2:  // cd
                scanf("%s", subCMD);
                changeDir(subCMD);
                break;
            case 3:  // touch
                scanf("%s", subCMD);
                newFile(subCMD);
                break;
            case 4:  // rm
                scanf("%s", subCMD);
                rmFile(subCMD);
                break;
            case 5:  // mkdir
                scanf("%s", subCMD);
                newDir(subCMD);
                break;
            case 6:  // rmdir
                scanf("%s", subCMD);
                rmDir(subCMD);
                break;
            case 7:
                printf("Waiting for your back. \n");
                return 0;
            default:
                printf("Unknown cmd. \n");
        }
    }
}