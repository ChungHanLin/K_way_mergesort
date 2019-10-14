//
//  selection_tree.h
//  K_way_mergesort
//
//  Created by 林重翰 on 2019/10/9.
//  Copyright © 2019 林重翰. All rights reserved.
//

#ifndef selection_tree_h
#define selection_tree_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "divide_file.h"

#define MAX_BUFFER_SIZE 131072
#define true 1
#define false 0

typedef struct file_offset{
    char filename[64];
    long offset;
}file_offset;

typedef unsigned int boolean;

static boolean BEGIN = false;
static boolean SIZE = false;
static boolean KEY = false;
static boolean TIME = false;
static boolean IGNORE = false;
static boolean REVERSE = false;
static boolean UNIQUE = false;

void send_mode_to_heap(boolean begin, boolean size , boolean key, boolean time, boolean ignore, boolean reverse, boolean unique);

void fill_heap(unsigned long block_num);

void read_file_to_block(FILE *fp, char *path, unsigned long block_index);

void select_winner_tree(unsigned long block_num);

void push_new_block();

unsigned long toNumber(char *pattern);

char *get_next_file(char *path, unsigned long block_index);

#endif /* selection_tree_h */
