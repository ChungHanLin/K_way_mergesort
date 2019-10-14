//
//  divide_file.h
//  K_way_mergesort
//
//  Created by 林重翰 on 2019/10/3.
//  Copyright © 2019 林重翰. All rights reserved.
//

#ifndef divide_file_h
#define divide_file_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 131072
#define true 1
#define false 0

// 單一資料的 struct
typedef struct single_data{
    char *content;          // 存放主要內容
    char *pattern;  // 比較欄位 (可能從主要內容中擷取)
    unsigned long size;
}single_data;

// 堆檔案資料的 struct
typedef struct block{
    single_data *list;
    unsigned long list_size;
}block;


// 用用看
typedef unsigned int boolean;

// global mode
static boolean BEGIN_PAT = false;
static boolean SIZE_MODE = false;
static boolean KEY_MODE = false;
static boolean TIME_MODE = false;
static boolean IGNORE_MODE = false;
static boolean REVERSE_MODE = false;
static boolean UNIQUE_MODE = false;

void send_mode_to_divider(boolean begin, boolean size , boolean key, boolean time, boolean ignore, boolean reverse, boolean unique);

void divide_file(char *begin_pattern, char *key_pattern, char *time_field, unsigned long total_data_num);

void rm_newline(char *buffer);

unsigned long get_block_num(unsigned long total_data_num);

unsigned long get_block_size(unsigned long total_data_unm, unsigned long block_num, unsigned long block_index);

char *get_pattern(char *buffer, char *field);

void block_internal_sort(block cache, unsigned long block_size, unsigned long block_num);

void write_cache_to_file(block cache, unsigned long block_size, unsigned long block_num);

void free_cache_list(block cache, unsigned long block_size);

int cmp_size(const void *a, const void *b);

int cmp_pattern(const void *a, const void *b);

int cmp_content(const void *a, const void *b);

char *tolower_str(char *str);

void get_file_path(char *path, unsigned long block_num, unsigned long file_num);
#endif /* divide_file_h */
