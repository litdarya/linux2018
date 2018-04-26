#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "iNode.h"
#include "file.h"

int* free_ranges;
int free_ranges_size = 0;

void AppendFreeRanges(int begin) { //todo x2
    free_ranges = (int *)realloc(free_ranges, (free_ranges_size + 1)* sizeof(int));
    free_ranges[free_ranges_size] = begin;
    ++free_ranges_size;
}

int ClosestPos() {
    if (free_ranges_size <= 0) {
        return -1;
    }
    int res = free_ranges[free_ranges_size - 1];
    free_ranges = (int *)realloc(free_ranges, (free_ranges_size - 1)* sizeof(int));
    --free_ranges_size;
    return res;
}

char* GetArg(char* buff, char delimeter) {
    char* res = NULL;

    for(int i = 0; i < strlen(buff); ++i) {
        if (buff[i] == delimeter) {
            res = (char*)malloc(i + 1);
            strncpy(res, buff, i);
            res[i] = '\0';
            break;
        }
    }

    if(res == NULL) {
        return buff;
    }
    return res;
}

char* GetCommand(char* buff, int size) {
    int i = 0;
    for(; i < strlen(buff); ++i) {
        if(buff[i] == ' ' || buff[i] == '\n') {
            break;
        }
    }
    char* ans = (char*)malloc(sizeof(char)*i);
    strncpy(ans, buff, i + 1);
    ans[i] = '\0';
    return ans;
}

void Touch(struct iNode* folder, char* buff) {
    char* filename;
    char* file;

    filename = GetArg(buff, ' ');
    file = GetArg(buff + strlen(filename) + 1, '\n');
    if(strlen(file) == 0) {
        return;
    }
    struct fileTree* res = WriteFile(filename, file);
    AddNewChild(folder, res);
}

struct iNode* Cd(struct iNode* root, char* buff) {
    char* addr = GetArg(buff, '\n');

    char* part = GetArg(addr, '/');
    int shift = 0;

    struct iNode* res = root;
    int param = 1;
    while(strlen(part) > 0) {
        shift += strlen(part);
        res = Find(res, part, &param);
        part = GetArg(addr + shift, '/');
    }

    return res;
}

void Ls(struct iNode* folder, char* buff) {
    char* filename = GetArg(buff, '\n');
    if(strlen(filename) != 0) {
        int param = 0;
        struct iNode* res = Find(folder, filename, &param);
        if (res == NULL) {
            printf("File is not found \n");
        } else {
            printf("%s \n", GetContentNew(res->file));
        }
    } else {
        PrintChildren(folder);
    }
}

struct iNode* Up(struct iNode* root) {
    if (root->parent == NULL) {
        printf("Impossible \n");
        return root;
    }
    return root->parent;
}

void DeleteElem(struct iNode* sample, int ind) {
    //todo x2
    struct iNode** new = (struct iNode**)malloc((sample->size)* sizeof(struct iNode*));

    int j = 0;
    for(int i = 0; i < sample->size; ++i) {
        if (i != ind) {
            new[i] = sample->children[j];
        } else {
            ind = -1;
            --i;
        }
        ++j;
    }
    free(sample->children);
    sample->children = new;
}

void Rm(struct iNode* folder, char* buff) {
    char* filename = GetArg(buff, '\n');
    int ind = -1;
    struct iNode* res = Find(folder, filename, &ind);
    if(res == NULL) {
        printf("Not found \n");
        return;
    }
    DeleteChild(res);
    DeleteElem(folder, ind);
    if(folder->size == 0) {
        folder->children = NULL;
    }
}

struct iNode* MakeDir(struct iNode* parent, char* buffer) {
    char* dir_name = GetArg(buffer, '\n');
    struct fileTree* file = WriteDir(dir_name);
    struct iNode* res = AddNewChild(parent, file);
    return res;
}

int main() {
    int size = 10000;
    char* buff = (char*)malloc(size);

    struct iNode* root = (struct iNode*)malloc(sizeof(struct iNode));
    root->children = NULL;
    char* root_name = (char*)malloc(2*sizeof(char));
    root_name[0] = '/';
    root_name[1] = '\0';
    root->file = WriteDir(root_name);
    while(fgets (buff, size, stdin) != NULL) {
        char* command = GetCommand(buff, size);
        if (strcmp(command, "touch") == 0) {
            Touch(root, buff + strlen(command) + 1);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "makedir") == 0) {
            MakeDir(root, buff + strlen(command) + 1);
        } else if (strcmp(command, "cd") == 0) {
            root = Cd(root, buff + strlen(command) + 1);
        } else if (strcmp(command, "ls") == 0) {
            Ls(root, buff + strlen(command) + 1);
        } else if (strcmp(command, "..") == 0) {
            root = Up(root);
        } else if (strcmp(command, "rm") == 0) {
            Rm(root, buff + strlen(command) + 1);
        }
    }
    free(free_ranges);
    free(buff);
    return 0;
}