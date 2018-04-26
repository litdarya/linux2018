//
// Created by darya on 22.04.18.
//

#include "file.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

const int block_size = 16;
const int tree_size = 256;

int min(int a, int b) {
    if(a < b) {
        return a;
    }
    return b;
}

char* GetFromFile(int begin, int end) {
    char* res = (char*)malloc(end - begin + 1);
    FILE* save_file = fopen("save_file.txt", "r");
    fseek(save_file, begin, SEEK_SET);
    fread(res, sizeof(char), end - begin, save_file);
    fclose(save_file);
    res[end - begin] = '\0';
    return res;
}

char* ReadBlock(int* pos) {
    char* res = (char*)malloc(block_size);
    FILE* save_file = fopen("save_file.txt", "r");
    fseek(save_file, *pos, SEEK_SET);
    fread(res, sizeof(char), block_size, save_file);
    fclose(save_file);
    res[block_size] = '\0';
    return res;
}

char* Concatenate(char* a, char* b) {
    char* res = (char *)malloc(1 + strlen(a)+ strlen(b));
    strcpy(res, a);
    strcat(res, b);
    free(a);
    free(b);

    return res;
}

char* ReadTree(struct file** files, int size) {
    char* res = NULL;
    for(int i = 0; i < size; ++i) {
        if(res == NULL) {
            res = ReadBlock(&files[i]->start_pos_content);
        } else {
            res = Concatenate(res, ReadBlock(&files[i]->start_pos_content));
        }
    }
    return res;
}

char* ReadLevelTree(int level, struct block1024* parent) {
    char* res = NULL;
    for (int i = 0; i < parent->size; ++i) {
        if(level == 1) {
            parent->children[i]->children = NULL;
            if (res == NULL) {
                res = ReadTree(parent->children[i]->files, parent->children[i]->size);
            } else {
                res = Concatenate(res, ReadTree(parent->children[i]->files, parent->children[i]->size));
            }
        } else {
            if (res == NULL) {
                res = ReadLevelTree(level - 1, parent->children[i]);
            } else {
                res = Concatenate(res, ReadLevelTree(level - 1, parent->children[i]));
            }
        }
    }
    return res;
}

char* GetContentNew(struct fileTree* file) {
    int size = min(12, file->size);
    char* res = ReadTree(file->files, size);

    if (file->size > 12) {
        res = Concatenate(res, ReadTree(file->block_tree[0]->files, file->block_tree[0]->size));
        for (int i = 1; i < file->size - 12; ++i) {
            res = Concatenate(res, ReadLevelTree(i, file->block_tree[i]));
        }
    }

    return res;
}

char* GetName(struct file* file) {
    return ReadBlock(&file->start_pos_name);
}

void DestructorFile(struct file* sample, int* pos) {
    AppendFreeRanges(*pos);
    free(sample);
}

void DeleteTree(struct file** files, int size) {
    for(int i = 0; i < size; ++i) {
        DestructorFile(files[i], &files[i]->start_pos_content);
    }
}

void DeleteLevelTree(int level, struct block1024* parent) {
    for (int i = 0; i < parent->size; ++i) {
        if (level == 1) {
            DeleteTree(parent->children[i]->files, parent->children[i]->size);
            free(parent->children[i]);
        } else {
            DeleteLevelTree(level - 1, parent->children[i]);
        }
    }
    free(parent);
}

void DestructorFileTree(struct fileTree* sample) {
    int size = min(12, sample->size);

    DeleteTree(sample->files, size);
    free(sample->files);

    if (sample->size > 12) {
        DeleteTree(sample->block_tree[0]->files, sample->block_tree[0]->size);
        for (int i = 1; i < sample->size - 12; ++i) {
            DeleteLevelTree(i, sample->block_tree[i]);
        }
    }

    DestructorFile(sample->file_name, &sample->file_name->start_pos_name);
    free(sample);
}

struct file* WriteBlock(char* block, int* pos) {
    FILE* save_file;
    *pos = ClosestPos();
    if (*pos == -1) {
        save_file = fopen("save_file.txt", "a");
        *pos = ftell(save_file);
    } else {
        save_file = fopen("save_file.txt", "r+");
    }

    if (save_file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    struct file* res = (struct file*)malloc(sizeof(struct file));
    fseek(save_file, *pos, SEEK_SET);
    fwrite(block, block_size, 1, save_file);
    fclose(save_file);

    return res;
}

void FillTree(struct file*** files, int* blocks, char* buff) {
    *files = (struct file **) malloc(min(*blocks, tree_size) * sizeof(struct file *));

    for (int i = 0; i < min(*blocks, tree_size); ++i) {
        int tmp;
        (*files)[i] = WriteBlock(i*block_size + buff, &tmp);
        (*files)[i]->start_pos_content = tmp;
    }

    *blocks -= min(*blocks, tree_size);
}

int small_pow(int a, int b) {
    int res = a;
    for(int i = 1; i < b; ++i) {
        res *= a;
    }
    return res;
}

void FillLevelTree(int level, int* blocks, char* buff, struct block1024* parent) {
    int bucket_size = *blocks;
    bucket_size /= small_pow(tree_size, level);
    if (*blocks % small_pow(tree_size, level) != 0) {
        ++bucket_size;
    }
    parent->size = min(tree_size, bucket_size);
    parent->children = (struct block1024**)malloc(parent->size*sizeof(struct block1024*));
    for (int i = 0; i < parent->size; ++i) {
        parent->children[i] = (struct block1024*)malloc(sizeof(struct block1024));
        if(level == 1) {
            parent->children[i]->children = NULL;
            parent->children[i]->size = min(*blocks, tree_size);
            FillTree(&parent->children[i]->files, blocks, buff + i * tree_size * block_size);
        } else {
            FillLevelTree(level - 1, blocks, buff + tree_size*tree_size * i * block_size,
                          parent->children[i]);
        }
    }
}

struct fileTree* Write(char* content) {
    struct fileTree* res = (struct fileTree*)malloc(sizeof(struct fileTree));

    int blocks = strlen(content)/block_size;
    if(strlen(content)%block_size != 0) {
        ++blocks;
    }
    res->files = (struct file**)malloc(min(12, blocks)*sizeof(struct file*));
    for(int i = 0; i < min(12, blocks); ++i) {
        int tmp;
        res->files[i] = WriteBlock(content + i*block_size, &tmp);
        res->files[i]->start_pos_content = tmp;
    }

    res->size = min(12, blocks);

    blocks -=12;

    if(blocks <= 0) {
        return res;
    }

    int size_tree = 1;
    if (blocks > small_pow(tree_size, 1)) {
        ++size_tree;
    }
    if (blocks > small_pow(tree_size, 2)) {
        ++size_tree;
    }
    res->size += size_tree;
    res->block_tree = (struct block1024**)malloc(size_tree*sizeof(struct block1024*));
    res->block_tree[0] = (struct block1024*) malloc(sizeof(struct block1024));
    res->block_tree[0]->size = min(blocks, tree_size);
    FillTree(&res->block_tree[0]->files, &blocks, content + 12*block_size);
    if (blocks <= 0) {
        return res;
    }

    int summary_size = tree_size;
    for (int i = 1; i < 3; ++i) {
        res->block_tree[i] = (struct block1024*) malloc(sizeof(struct block1024));
        FillLevelTree(i, &blocks, content + (12 + summary_size)*block_size, res->block_tree[i]);
        if (blocks <= 0) {
            return res;
        }
        summary_size*=(tree_size + 1);
    }

    assert(0);
    return NULL;
}

struct fileTree* WriteDir(char* dir_name) {
//    return Write(dir_name);
    if(strlen(dir_name) > block_size) {
        printf("Error! File name should be less then %d", block_size);
        return NULL;
    }

    int tmp;
    struct file* name = WriteBlock(dir_name, &tmp);
    name->start_pos_name = tmp;

    struct fileTree* res = (struct fileTree*)malloc(sizeof(struct fileTree));
    res->file_name = name;
    res->size = 0;
}


struct fileTree* WriteFile(char* file_name, char* file) {
    if(strlen(file_name) > block_size) {
        printf("Error! File name should be less then %d", block_size);
        return NULL;
    }

    int tmp;
    struct file* name = WriteBlock(file_name, &tmp);
    name->start_pos_name = tmp;

    struct fileTree* res = Write(file);
    res->file_name = name;

    free(file_name);
    free(file);
    return res;
}
