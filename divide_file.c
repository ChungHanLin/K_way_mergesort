//
//  divide_file.c
//  K_way_mergesort
//
//  Created by 林重翰 on 2019/10/3.
//  Copyright © 2019 林重翰. All rights reserved.
//

#include "divide_file.h"

void send_mode_to_divider(boolean begin_pat, boolean size, boolean key, boolean time, boolean ignore, boolean reverse, boolean unique){
    BEGIN_PAT = begin_pat;
    SIZE_MODE = size;
    KEY_MODE = key;
    TIME_MODE = time;
    IGNORE_MODE = ignore;
    REVERSE_MODE = reverse;
    UNIQUE_MODE = unique;
}

void divide_file(char *begin_pattern, char *key_pattern, char *time_field, unsigned long total_data_num){
    // 讀原資料檔
    char *filePath = "ettoday.rec";
    char buffer[MAX_BUFFER_SIZE];

    FILE *fp = fopen(filePath, "r");
 
    // 計算需要多少 block 存儲所有 node
    unsigned long block_num = get_block_num(total_data_num);
    unsigned long block_size = get_block_size(total_data_num, block_num, 0);
    unsigned long block_num_cnt = 0, block_size_cnt = 0;
    // 宣告 memory，存放每格 block 資訊
    block cache;
    
    // 存放 block 中所有 data
    cache.list = (single_data *)malloc(sizeof(single_data) * block_size);
    cache.list_size = 0;
    
    //
    boolean begin_read_file = false;
    unsigned long write_buffer_size = 8192, write_buffer_cnt = 0;
    char *write_buffer = (char *)malloc(write_buffer_size);

    
    boolean key_pattern_is_body = false;
    unsigned long body_buffer_size = 8192, body_buffer_cnt = 0;
    char *body_buffer;
  
    while(fgets(buffer, MAX_BUFFER_SIZE, fp)){
        // 先將 buffer 尾端的 \n 去除
        rm_newline(buffer);
        if(buffer[0] == '\0'){
            continue;
        }
        
        // 沒有給 begin_pattern 時，則預設為 line base，逐行視為 data 讀取
        if(BEGIN_PAT == false){
            // total_data_num 爆炸多，完全 line_base
            // 基本上即便有 -k -t 也不會作用
            if(block_size_cnt == block_size){
                block_internal_sort(cache, block_size, block_num_cnt);
                if(block_num_cnt == (block_num - 1)){
                    break;
                }
                // 釋放 cache.list 該 block 中 content 的記憶體
                free_cache_list(cache, block_size);
                
                block_num_cnt++;
                block_size_cnt = 0;
                block_size = get_block_size(total_data_num, block_num, block_num_cnt);
                cache.list = (single_data *)malloc(sizeof(single_data) * block_size);
                cache.list_size = 0;
            }
            
            // insert data
            cache.list_size++;
            cache.list[block_size_cnt].size = strlen(buffer) + 1;
            cache.list[block_size_cnt].content = (char *)malloc(strlen(buffer) + 1);
            if(!cache.list[block_size_cnt].content){
                fprintf(stderr, "ERROR: Malloc failed\n");
                exit(EXIT_FAILURE);
            }
            sprintf(cache.list[block_size_cnt].content, "%s", buffer);
            block_size_cnt++;
        }
        
        else{
            // 讀取到 begin_pattern 計算 block_cnt，開啟新的讀取檔案行為
            if((strstr(buffer, begin_pattern) && strlen(begin_pattern) > 2) || (strcmp(buffer, "@") == 0 && strlen(begin_pattern) <= 2)){
                
                if(begin_read_file == true){
                    if(block_size_cnt == block_size){
                        // 讀取完畢後，該 block 進行 internal sort
                        if(block_num_cnt == (block_num - 1)){
                            break;
                        }
                        block_internal_sort(cache, block_size, block_num_cnt);
                        free_cache_list(cache, block_size);
                        
                        block_num_cnt++;
                        block_size_cnt = 0;
                        block_size = get_block_size(total_data_num, block_num, block_num_cnt);
                        cache.list = (single_data *)malloc(sizeof(single_data) * block_size);
                        if(!cache.list){
                            fprintf(stderr, "ERROR: Malloc failed\n");
                            exit(EXIT_FAILURE);
                        }
                        cache.list_size = 0;
                    }
                    
                    // insert data
                    cache.list_size++;
                    cache.list[block_size_cnt].size = strlen(write_buffer);
                    cache.list[block_size_cnt].content = (char *)malloc(strlen(write_buffer) + 1);
                    if(!cache.list[block_size_cnt].content){
                        fprintf(stderr, "ERROR: Malloc failed\n");
                        exit(EXIT_FAILURE);
                    }
                    sprintf(cache.list[block_size_cnt].content, "%s", write_buffer);
                    // 記錄 news 內容的動態記憶體重新宣告
                    // key_pattern_is_body = false;
                    memset(write_buffer, '\0', write_buffer_size);
                    
                    if(write_buffer_cnt != 0){
                        free(write_buffer);
                    }
                    write_buffer_cnt = 0;
                    write_buffer_size = 8192;
                    
                    block_size_cnt++;
                    write_buffer = (char *)malloc(write_buffer_size);
                    if(!write_buffer){
                        fprintf(stderr, "ERROR: Malloc failed\n");
                        exit(EXIT_FAILURE);
                    }
                    sprintf(write_buffer, "%s", buffer);
                }
                else{
                    // 第一次讀取到 begin pattern
                    // 將資料放進 memory[0] 中
                    begin_read_file = true;
                    write_buffer_cnt += (strlen(buffer) + 1);
                    sprintf(write_buffer, "%s", buffer);
                    continue;
                }
            }
            // 表示尚未找到相應的 begin_pattern，則繼續往下讀取
            // 可避免整篇文章無相應 begin_pattern 的問題
            else if(begin_read_file == false){
                continue;
            }
            // 該篇新聞文檔的 attribute
            else{
                // 若有 -k 出現
                if(KEY_MODE == true){
                    if(strstr(buffer, key_pattern)){
                        if(strcmp("@body", key_pattern) != 0){
                            cache.list[block_size_cnt].pattern = (char *)malloc(strlen(get_pattern(buffer, key_pattern)));
                            strcpy(cache.list[block_size_cnt].pattern, get_pattern(buffer, key_pattern));
                        }
                        else{
                            key_pattern_is_body = true;
                            body_buffer = (char *)malloc(body_buffer_size);
                        }
                    }
                }
                
                // 若有 -t 出現
                else if(TIME_MODE == true){
                    if(strstr(buffer, time_field)){
                        cache.list[block_size_cnt].pattern = (char *)malloc(strlen(get_pattern(buffer, time_field)));
                        strcpy(cache.list[block_size_cnt].pattern, get_pattern(buffer, time_field));
                    }
                }
                // 計算 write_buffer 已使用多少空間
                write_buffer_cnt += (strlen(buffer) + 1);
                while(write_buffer_size * 0.8 < write_buffer_cnt){
                    write_buffer_size *= 2;
                    write_buffer = (char *)realloc(write_buffer, write_buffer_size);
                    if(!write_buffer){
                        fprintf(stderr, "ERROR: Realloc failed.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                
                // 寫在原 buffer 後方
                sprintf(write_buffer, "%s\n%s", write_buffer, buffer);
 /*
                if(key_pattern_is_body == true){
                    body_buffer_cnt += strlen(buffer);
                    if(body_buffer_size * 0.8 < body_buffer_cnt){
                        body_buffer_size *= 2;
                        body_buffer = (char *)realloc(body_buffer, body_buffer_size);
                    }
                    
                    sprintf(body_buffer, "%s\n%s", body_buffer, buffer);
                }*/
            }
        }
    }
    
    // 處理最後一筆資料
    if(BEGIN_PAT == true){
        cache.list_size++;
        cache.list[block_size_cnt].size = strlen(write_buffer);
        cache.list[block_size_cnt].content = (char *)malloc(write_buffer_size);
        sprintf(cache.list[block_size_cnt].content, "%s", write_buffer);
        free(write_buffer);
    }
    block_internal_sort(cache, block_size, block_num_cnt);
    free_cache_list(cache, block_size);
    
    fclose(fp);

    
    // 測試用 - 印出所有內容
    // print_out_block(memory, block_num);
 
}

// 去除 buffer 尾端的 \n
void rm_newline(char *buffer){
    if(buffer[strlen(buffer) - 1] == '\n'){
        buffer[strlen(buffer) - 1] = '\0';
    }
}

// 計算總共需要多少個 block 存儲所有 data
unsigned long get_block_num(unsigned long total_data_num){
    unsigned long block_num = 2;
    
    for(block_num = 2; block_num < (total_data_num / block_num); block_num *= 2);
    
    return block_num / 8;
}

// 計算每一個 block 當中有多少 size，由於 data_num 不一定能整除
unsigned long get_block_size(unsigned long total_data_num, unsigned long block_num, unsigned long block_index){
    unsigned long block_size = 0;
    
    if(block_index + 1 <= (total_data_num % block_num)){
        block_size = (total_data_num / block_num) + 1;
    }
    else{
        block_size = total_data_num / block_num;
    }
    
    return block_size;
}

char *get_pattern(char *buffer, char *field){
    return buffer += strlen(field);
}


void free_cache_list(block cache, unsigned long block_size){
    unsigned long block_index;
    
    for(block_index = 0; block_index < block_size; block_index++){
        if(cache.list[block_index].content[0] != '\0'){
            free(cache.list[block_index].content);
        }
        if(KEY_MODE == true || TIME_MODE == true){
            free(cache.list[block_index].pattern);
        }
    }
    
    free(cache.list);
}

void block_internal_sort(block cache, unsigned long block_size, unsigned long block_num){
    // ignore 可以在 cmp function 判斷
    
    if(SIZE_MODE == true){
        // compare its size 也能在 cmp 中做
        qsort(cache.list, block_size, sizeof(single_data), cmp_size);
    }
    else if((KEY_MODE == true || TIME_MODE == true) && BEGIN_PAT == true){
        // compare
        qsort(cache.list, block_size, sizeof(single_data), cmp_pattern);
    }
    else{
        qsort(cache.list, block_size, sizeof(single_data), cmp_content);
    }
    
    // 可以測試 block_size 與 cach.list_size 的是否恆為相等？
    
    write_cache_to_file(cache, block_size, block_num);
 /*   unsigned long i;
    
    for(i = 0; i < block_size; i++){
        printf("%s\n", cache.list[i].content);
    }*/
    
}

int cmp_size(const void *a, const void *b){
    single_data *x = (single_data *)a;
    single_data *y = (single_data *)b;
    
    if(REVERSE_MODE == false){
        if(x->size > y->size){
            return 1;
        }
        else{
            return -1;
        }
    }
    else{
        if(x->size >= y->size){
            return -1;
        }
        else{
            return 1;
        }
    }
}

int cmp_pattern(const void *a, const void *b){
    single_data *x = (single_data *)a;
    single_data *y = (single_data *)b;
    
    char *x_pattern, *y_pattern;
    
    if(IGNORE_MODE == true){
        x_pattern = (char *)malloc(strlen(x->pattern));
        x_pattern = tolower_str(x->pattern);
        
        y_pattern = (char *)malloc(strlen(y->pattern));
        y_pattern = tolower_str(y->pattern);
        
        if(REVERSE_MODE == false){
            return strcmp(x_pattern, y_pattern);
        }
        else{
            return strcmp(y_pattern, x_pattern);
        }
    }
    else{
        if(REVERSE_MODE == false){
            return strcmp(x->pattern, y->pattern);
        }
        else{
            return strcmp(y->pattern, x->pattern);
        }
    }
}

int cmp_content(const void *a, const void *b){
    single_data *x = (single_data *)a;
    single_data *y = (single_data *)b;
    
    char *x_content, *y_content;
    
    if(IGNORE_MODE == true){
        x_content = (char *)malloc(strlen(x->content));
        x_content = tolower_str(x->content);
        
        y_content = (char *)malloc(strlen(y->content));
        y_content = tolower_str(y->content);
        
        if(REVERSE_MODE == false){
            return strcmp(x_content, y_content);
        }
        else{
            return strcmp(y_content, x_content);
        }
    }
    else{
        if(REVERSE_MODE == false){
            return strcmp(x->content, y->content);
        }
        else{
            return strcmp(y->content, x->content);
        }
    }
}

char *tolower_str(char *str){
    unsigned long index;
    char *result = (char *)malloc(strlen(str));
    char *ptr = result;
    for(index = 0; index < strlen(str); index++){
        *ptr = tolower(*str);
        ptr++;
        str++;
    }
    *ptr = '\0';
    
    return result;
}

void write_cache_to_file(block cache, unsigned long block_size, unsigned long block_num){
    // 由於取得 file 總數量的方式與計算 block 總數的方法一樣
    // 是否要額外寫一個 get_file_num( ) 做同意的事情？
    unsigned long file_num = get_block_num(block_size);
    unsigned long file_size = get_block_size(block_size, file_num, 0);
    unsigned long file_size_cnt = 0, file_num_cnt = 0;
    
    unsigned long cache_index = 0;
    
    FILE *fw;
     
    for(file_num_cnt = 0; file_num_cnt < file_num; file_num_cnt++){
        file_size = get_block_size(block_size, file_num, file_num_cnt);
        
        // 開檔 寫入
        char path[64];
        get_file_path(path, block_num, file_num_cnt);
        fw = fopen(path, "w");
        for(file_size_cnt = 0; file_size_cnt < file_size; file_size_cnt++){
            // content
            fprintf(fw, ">Size:%lu\n", cache.list[cache_index].size);
            fprintf(fw, ">Content:\n%s", cache.list[cache_index].content);
            if((KEY_MODE == true || TIME_MODE == true) && BEGIN_PAT == true){
                fprintf(fw, "@Pattern:%s\n", cache.list[cache_index].pattern);
            }
            fprintf(fw, "\n\n");
            cache_index++;
        }
        fclose(fw);
    }
    //printf("%lu : %lu\n", block_num, cache_index);
    // 理論上應該寫入檔案是對的
}

void get_file_path(char *path, unsigned long block_num, unsigned long file_num){
    sprintf(path, "tmp/%lu_%lu", block_num, file_num);
}
