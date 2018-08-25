/**
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 **/
 
//这个文件属于LWIP协议栈的一部分
#include "lwip/opt.h"
#include "lwip/def.h"
#include "fs.h"
#include "fsdata.h"
#include <string.h>

//设置HTTPD_USE_SUSTUM_FSDATA为1时,使用fsdata_custom.c代替fsdata.c
#ifndef HTTPD_USE_CUSTUM_FSDATA
#define HTTPD_USE_CUSTUM_FSDATA 0  //使用fsdata.c
#endif

#if HTTPD_USE_CUSTUM_FSDATA
#include "fsdata_custom.c"
#else /* HTTPD_USE_CUSTUM_FSDATA */
#include "fsdata.c"
#endif /* HTTPD_USE_CUSTUM_FSDATA */


//定义最多能打开的文件数目
#ifndef LWIP_MAX_OPEN_FILES
#define LWIP_MAX_OPEN_FILES     10
#endif

//定义文件系统内存分配结构体
struct fs_table {
  struct fs_file file;  //
  u8_t inuse;   //0表示未使用,1表示使用
};

//定义一个fs_tabble数组,其中包括LWIP_MAX_OPEN_FILES个元素
struct fs_table fs_memory[LWIP_MAX_OPEN_FILES];

#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
#endif /* LWIP_HTTPD_CUSTOM_FILES */

//文件内存申请函数 给LWIP_MAX_OPEN_FILES个文件同时分配内存
static struct fs_file *
fs_malloc(void)
{
  int i;
  for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
    if(fs_memory[i].inuse == 0) {
      fs_memory[i].inuse = 1;
      return(&fs_memory[i].file);
    }
  }
  return(NULL);
}

//释放内存,一次释放掉LWIP_MAX_OPEN_FILES个变量的内存
static void
fs_free(struct fs_file *file)
{
  int i;
  for(i = 0; i < LWIP_MAX_OPEN_FILES; i++) {
    if(&fs_memory[i].file == file) {
      fs_memory[i].inuse = 0;
      break;
    }
  }
  return;
}

//打开一个文件
//name:要打开的文件名
struct fs_file *
fs_open(const char *name)
{
  struct fs_file *file;
  const struct fsdata_file *f;

  file = fs_malloc();  //申请内存
  if(file == NULL) {
    return NULL;
  }

#if LWIP_HTTPD_CUSTOM_FILES
  if(fs_open_custom(file, name)) {
    file->is_custom_file = 1;
    return file;
  }
  file->is_custom_file = 0;
#endif /* LWIP_HTTPD_CUSTOM_FILES */

  for(f = FS_ROOT; f != NULL; f = f->next) {
    if (!strcmp(name, (char *)f->name)) {  //根据函数的name参数查找
      file->data = (const char *)f->data;
      file->len = f->len;
      file->index = f->len;
      file->pextension = NULL;
      file->http_header_included = f->http_header_included;
#if HTTPD_PRECALCULATED_CHECKSUM  //如果使用HTTPD_PRECALCULATED的话
      file->chksum_count = f->chksum_count;
      file->chksum = f->chksum;
#endif /* HTTPD_PRECALCULATED_CHECKSUM */
#if LWIP_HTTPD_FILE_STATE   //如果使用LEIP_HTTPD_FILE_STATE
      file->state = fs_state_init(file, name);
#endif /* #if LWIP_HTTPD_FILE_STATE */
      return file;
    }
  }
  fs_free(file);  //释放file内存
  return NULL;
}

//关闭文件
//参数:要关闭的文件
void
fs_close(struct fs_file *file)
{
#if LWIP_HTTPD_CUSTOM_FILES
  if (file->is_custom_file) {
    fs_close_custom(file);
  }
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#if LWIP_HTTPD_FILE_STATE
  fs_state_free(file, file->state);
#endif /* #if LWIP_HTTPD_FILE_STATE */
  fs_free(file);  //释放file的内存
}

//读取文件
//参数:file 要读取的文件
//参数:buffer 读取到后存放的缓冲区
//参数:count  要读取的个数
int
fs_read(struct fs_file *file, char *buffer, int count)
{
  int read; //实际读取的数据个数

  if(file->index == file->len) {
    return -1;
  }

  read = file->len - file->index;
  if(read > count) {
    read = count;
  }

  MEMCPY(buffer, (file->data + file->index), read);
  file->index += read;

  return(read);  //返回读取的数据个数
}

//文件左对齐
int fs_bytes_left(struct fs_file *file)
{
  return file->len - file->index;
}
