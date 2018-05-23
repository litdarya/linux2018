//
// Created by darya on 22.04.18.
//

#ifndef FS_EXT2_LIKE_FILE_H
#define FS_EXT2_LIKE_FILE_H

#define BUFFER_SIZE 10000

struct file {
    int start_pos_name;
    int start_pos_content;
};

struct block1024{
    int size;
    struct file** files;
    struct block1024** children;
};

struct fileTree {
    int size;
    struct file* file_name;
    struct file** files;
    struct block1024** block_tree;
};

char* GetContentNew(struct fileTree* file);

char* GetFromFile(int begin, int end);

char* GetName(struct file* file);

char* GetContent(struct file* file);

void DestructorFile(struct file* sample, int* pos);

struct fileTree* WriteDir(char* dir_name);

struct fileTree* WriteFile(char* file_name, char* file);

void DestructorFileTree(struct fileTree* sample);

#endif //FS_EXT2_LIKE_FILE_H
