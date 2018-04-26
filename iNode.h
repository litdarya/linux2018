//
// Created by darya on 22.04.18.
//

#ifndef FS_EXT2_LIKE_INODE_H
#define FS_EXT2_LIKE_INODE_H

#include "file.h"

struct iNode {
    int size;
    struct iNode* parent;
    struct fileTree* file;
    struct iNode** children;
};


void PrintChildren(struct iNode* parent);

int IsFolder(struct iNode* sample);

struct iNode* Find(struct iNode* parent, char *addr, int* param);

void ReallocChildrenArray(struct iNode* parent);

void AddExistedChild(struct iNode *parent, struct iNode* child);

struct iNode* AddNewChild(struct iNode *parent, struct fileTree *file);

void DestructoriNode(struct iNode* sample);

void DeleteRecursively(struct iNode* sample);

struct iNode* DeleteChild(struct iNode* child);

#endif //FS_EXT2_LIKE_INODE_H