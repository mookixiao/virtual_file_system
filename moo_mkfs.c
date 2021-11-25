//
// Created by mooki on 10/16/21.
//

#include <stdio.h>
#include <string.h>

#include "utils/moo_fs.h"

void setBit(void *beg, uint32_t idx){
    uint32_t idxMaj = idx / BITNUMOFBMP;
    uint32_t idxMin = idx % BITNUMOFBMP;
    ((uint32_t*)beg)[idxMaj] |= 1 << idxMin;
}

void clrBit(void *beg, uint32_t idx){
    uint32_t idxMaj = idx / BITNUMOFBMP;
    uint32_t idxMin = idx % BITNUMOFBMP;
    ((uint32_t*)beg)[idxMaj] &= ~(1 << idxMin);
}

int main(int argc, char* argv[]){
    FILE *vdisk;
    if((vdisk = fopen(VDISKFILE, "w+")) == NULL) {
        printf("Create virtual disk file %s failed. \n", VDISKFILE);
        return 0;
    }

    // 写入超级块
    SuperBlk superBlk = {BLKNUM, BLKNUM, BLKSIZE, INDNUM, INDNUM, sizeof(Inode)};
    if(fwrite(&superBlk, sizeof(superBlk), 1, vdisk) != 1){
        printf("Write SuperBlk failed. \n");
        return 0;
    }

    // 写入文件系统描述
    FSInfo fsInfo = {"Moo File System, V0.1", INDBEG, INDEND, BLKBEG, BLKEND};
    if(fwrite(&fsInfo, sizeof(fsInfo), 1, vdisk) != 1){
        printf("Write FSInfo failed. \n");
        return 0;
    }

    // 写入i节点位图
    uint32_t indBitmap[INDNUM / 32];
    memset(indBitmap, 0, INDBMPEND - INDBMPBEG);
    if(fwrite(&indBitmap, sizeof(indBitmap), 1, vdisk) != 1){
        printf("Write indBitmap failed. \n");
        return 0;
    }

    // 写入数据块位图
    uint32_t blkBitmap[BLKNUM / 32];
    memset(blkBitmap, 0, BLKBMPEND - BLKBMPBEG);
    if(fwrite(&blkBitmap, sizeof(blkBitmap), 1, vdisk) != 1){
        printf("Write blkBitmap failed. \n");
        return 0;
    }

    // 写入数据块
    char blk[BLKSIZE];
    memset(blk, 0, sizeof(blk));
    if(fseek(vdisk, BLKBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    for(int i = 0; i < BLKNUM; ++i){
        if(fwrite(blk, sizeof(blk), 1, vdisk) != 1){
            printf("Write blkBitmap failed. \n");
            return 0;
        }
    }


    fflush(vdisk);  // 冲洗数据

    /* 创建根目录 */
    // 修改超级块
    if(fseek(vdisk, SBBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fread(&superBlk, sizeof(superBlk), 1, vdisk) != 1){
        printf("Read SuperBlk failed. \n");
        return 0;
    }
    superBlk.blkNumFree -= 1;
    superBlk.indNumFree -= 1;
    if(fseek(vdisk, SBBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fwrite(&superBlk, sizeof(superBlk), 1, vdisk) != 1){
        printf("Write SuperBlk failed. \n");
        return 0;
    }

    // 修改i节点位图
    if(fseek(vdisk, INDBMPBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fread(&indBitmap, sizeof(indBitmap), 1, vdisk) != 1){
        printf("Read indBitmap failed. \n");
        return 0;
    }
    setBit(indBitmap, 0);
    if(fseek(vdisk, INDBMPBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fwrite(&indBitmap, sizeof(indBitmap), 1, vdisk) != 1){
        printf("Write indBitmap failed. \n");
        return 0;
    }

    // 修改数据块位图
    if(fseek(vdisk, BLKBMPBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fread(&blkBitmap, sizeof(blkBitmap), 1, vdisk) != 1){
        printf("Read indBitmap failed. \n");
        return 0;
    }
    setBit(blkBitmap, 0);
    if(fseek(vdisk, BLKBMPBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fwrite(&blkBitmap, sizeof(blkBitmap), 1, vdisk) != 1){
        printf("Write indBitmap failed. \n");
        return 0;
    }

    // 修改i节点区
    Inode inode = {sizeof(DirItem) * 2, iSDir, {0}};
    if(fseek(vdisk, INDBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fwrite(&inode, sizeof(inode), 1, vdisk) != 1){
        printf("Write Inode failed. \n");
        return 0;
    }

    // 修改数据块区
    DirItem dirItem = {".", 0};
    if(fseek(vdisk, BLKBEG, SEEK_SET) != 0){
        printf("Set cursor offset failed. \n");
        return 0;
    }
    if(fwrite(&dirItem, sizeof(dirItem), 1, vdisk) != 1){  // 写入.的目录项
        printf("Write DirItem failed. \n");
        return 0;
    }
    strcpy(dirItem.name, "..");
    if(fwrite(&dirItem, sizeof(dirItem), 1, vdisk) != 1){  // 写入..的目录项
        printf("Write DirItem failed. \n");
        return 0;
    }

    // 关闭虚拟磁盘文件
    if(fclose(vdisk) != 0){
        printf("Close virtual disk file %s failed. ", VDISKFILE);
        return 0;
    }

    return 0;
}