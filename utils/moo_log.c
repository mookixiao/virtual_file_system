//
// Created by mooki on 10/27/21.
//

#include <stdio.h>
#include <stdlib.h>

#include "moo_utils.h"

// 显示提示信息并返回
void infoToRet(const char* label, const char* info){
    printf("INFO: %s, %s\n", label, info);
}

// 显示错误信息并返回
void errToRet(const char* label, const char* err){
    printf("ERR: %s, %s\n", label, err);
}

// 显示错误信息并退出
void errToExit(const char* label, const char* err){
    printf("ERR: %s, %s\n", label, err);
    exit(0);
}