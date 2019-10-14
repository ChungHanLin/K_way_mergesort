//
//  main.c
//  K_way_mergesort
//
//  Created by 林重翰 on 2019/10/3.
//  Copyright © 2019 林重翰. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "divide_file.h"
#include "selection_tree.h"

#define true 1
#define false 0

// 用用看
typedef unsigned int boolean;

// global mode
static boolean BEGIN_PATTERN = false;
static boolean SIZE_ORDER = false;
static boolean KEY_ORDER = false;
static boolean TIME_ORDER = false;
static boolean IGNORE_CASE = false;
static boolean REVERSE_ORDER = false;
static boolean UNIQUE_ORDER = false;

// function
void argument_detector(int argc, char **argv, char *begin_pattern, char *key_pattern, char* time_field, unsigned long pattern_size);
void create_target_dir();
unsigned long get_data_num(char *begin_pattern);

int main(int argc, char *argv[]) {
    unsigned long pattern_size = 64;
    
    char *begin_pattern = (char *)malloc(pattern_size);
    char *key_pattern = (char *)malloc(pattern_size);
    char *time_field = (char *)malloc(pattern_size);
    
    // 判斷 command line 啟用哪些 argument
    argument_detector(argc, argv, begin_pattern, key_pattern, time_field, pattern_size);
    
    create_target_dir();
    
    // 為求分割檔案保持效率，犧牲一次計算 begin_pattern 數量
    unsigned long total_data_num = get_data_num(begin_pattern);
    
    struct timeval start_t_divider, end_t_divider;
    gettimeofday(&start_t_divider, NULL);
    // 將 mode flag 傳至 divide file 函式中
    send_mode_to_divider(BEGIN_PATTERN, SIZE_ORDER, KEY_ORDER, TIME_ORDER, IGNORE_CASE, REVERSE_ORDER, UNIQUE_ORDER);
    
    // 將原測資檔切成數個 file
    divide_file(begin_pattern, key_pattern, time_field, total_data_num);
    
    gettimeofday(&end_t_divider, NULL);
    fprintf(stderr, "Divide file : %lf sec\n", (double)(1000000 * (end_t_divider.tv_sec - start_t_divider.tv_sec) + end_t_divider.tv_usec - start_t_divider.tv_usec) / 1000000);
    
    struct timeval start_t_merge, end_t_merge;
    // 開始進行 merge sort
    // 判斷 command line 啟用哪些 argument
    send_mode_to_heap(BEGIN_PATTERN, SIZE_ORDER, KEY_ORDER, TIME_ORDER, IGNORE_CASE, REVERSE, UNIQUE);
    gettimeofday(&start_t_merge, NULL);
    fill_heap(get_block_num(total_data_num));
    gettimeofday(&end_t_merge, NULL);
    
    fprintf(stderr, "Merge file : %lf sec\n", (double)(1000000 * (end_t_merge.tv_sec - start_t_merge.tv_sec)+ end_t_merge.tv_usec - start_t_merge.tv_usec) / 1000000);
    
    // 釋放動態記憶體
    free(begin_pattern);
    free(key_pattern);
    free(time_field);
    
    // 刪除 tmp 資料夾
    system("rmdir tmp");
    
    return 0;
}

// 判斷 command line 啟用哪些 argument
void argument_detector(int argc, char **argv, char *begin_pattern, char *key_pattern, char *time_field, unsigned long pattern_size){
    int arg_index = 0;
    
    for(arg_index = 0; arg_index < argc; arg_index++){
        if(argv[arg_index][0] == '-'){
            switch (argv[arg_index][1]){
                case 'b':   // record begin pattern
                    // move to the next argument to get [pattern], check its boundary at the same time.
                    if(++arg_index < argc){
                        if(strlen(argv[arg_index]) > pattern_size){
                            fprintf(stderr, "WARNING: Please decrease your key pattern size.\n");
                            exit(EXIT_FAILURE);
                        }
                        if(argv[arg_index][0] != '@'){
                            fprintf(stderr, "WARNING: '＠' is needed before the pattern or field.\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(begin_pattern, argv[arg_index]);
                        BEGIN_PATTERN = true;
                    }
                    else{
                        fprintf(stderr, "ERROR: [pattern] is needed when using [-bk] mode\n");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'k':   // key begin pattern
                    // move to the next argument to get [pattern], check its boundary at the same time.
                    if(++arg_index < argc){
                        if(strlen(argv[arg_index]) > pattern_size){
                            fprintf(stderr, "WARNING: Please decrease your key pattern size.\n");
                            exit(EXIT_FAILURE);
                        }
                        if(argv[arg_index][0] != '@'){
                            fprintf(stderr, "WARNING: '＠' is needed before the pattern or field.\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(key_pattern, argv[arg_index]);
                        KEY_ORDER = true;
                    }
                    else{
                        fprintf(stderr, "ERROR: It should add [pattern] when using [-bk] mode\n");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 's':   // order with size
                    SIZE_ORDER = true;
                    break;
                case 'i':   // ignore big or small case
                    IGNORE_CASE = true;
                    break;
                case 't':   // order with time tag
                    if(++arg_index < argc){
                        if(strlen(argv[arg_index]) > pattern_size){
                            fprintf(stderr, "WARNING: Please decrease your key pattern size.\n");
                            exit(EXIT_FAILURE);
                        }
                        if(argv[arg_index][0] != '@'){
                            fprintf(stderr, "WARNING: '＠' is needed before the pattern or field.\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(time_field, argv[arg_index]);
                    }
                    else{
                        fprintf(stderr, "ERROR: It should add [time_field] when using [-t] mode\n");
                        exit(EXIT_FAILURE);
                    }
                    TIME_ORDER = true;
                    break;
                case 'r':   // reverse output order
                    REVERSE_ORDER = true;
                    break;
                case 'u':   // only output unique data
                    UNIQUE_ORDER = true;
                    break;
                default:    // unknown argument
                    fprintf(stderr, "ERROR: illegal option %s\nUSAGE: external_sort [-irsu] | [-bk] + [pattern] | [-t] + [time_filed] [file ...]\n", argv[arg_index]);
                    exit(EXIT_FAILURE);
            }
        }
    }
    
    // 若沒有指定 begin_pattern 則預設 @ 為分段點
    if(strlen(begin_pattern) == 0){
        strcpy(begin_pattern, "");
    }
}

void create_target_dir(){
    if(access("tmp", 0) < 0){
        system("mkdir tmp");
    }
    else{
        system("rm -r tmp");
        system("mkdir tmp");
    }
}

// 先執行一次，計算總共會生成多少資料量
unsigned long get_data_num(char *begin_pattern){
    unsigned long data_count = 0;
    boolean valid_pattern = true;
    // 讀原資料檔
    char *filePath = "ettoday.rec";
    char buffer[MAX_BUFFER_SIZE];
    FILE *fp = fopen(filePath, "r");
    if(strcmp(begin_pattern, "") == 0){
        valid_pattern = false;
    }
    
    while(fgets(buffer, MAX_BUFFER_SIZE, fp)){
        rm_newline(buffer);
        if(buffer[0] != '\0' && valid_pattern == false){
            data_count++;
        }
        else if(valid_pattern == true){
            if(strlen(begin_pattern) > 2){
                if(strstr(buffer, begin_pattern)){
                    data_count++;
                }
            }
            else{
                if(strcmp("@", buffer) == 0){
                    data_count++;
                }
            }
        }
    }
    fclose(fp);
    
    return data_count;
}
