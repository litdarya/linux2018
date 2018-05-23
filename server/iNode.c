//
// Created by darya on 22.04.18.
//
#include "iNode.h"
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <ares_build.h>

extern int sock;

void PrintChildren(struct iNode* parent) {
    if (parent->children != NULL) {
        char res[BUFFER_SIZE];
        res[0] = 0;
        for (int i = 0; i < parent->size; ++i) {
            char* name = GetName(parent->children[i]->file->file_name);
            strcat(res, name);
            strcat(res, " \n");
//            printf("%s \n", GetName(parent->children[i]->file->file_name));
        }
//        printf("\n");
        strcat(res, "\n");
        send(sock, res, sizeof(res), 0);
    } else {
        char msg[] = "Empty \n";
        send(sock, msg, sizeof(msg), 0);
//        printf("Empty \n");
    }
}

int IsFolder(struct iNode* sample) {
//    return sample->file->start_pos_content == '\0';
    return sample->file->size == 0;
}

struct iNode* Find(struct iNode* parent, char *addr, int* param) {
    //-1 - all, 0 - file, 1 - dir
    if (parent->children != NULL) {
        for (int i = 0; i < parent->size; ++i) {
            int predicate = IsFolder(parent->children[i]) == *param || *param == -1;
            if (predicate && strcmp(GetName(parent->children[i]->file->file_name), addr) == 0) {
                *param = i;
                return parent->children[i];
            }
        }
    }
    return NULL;
}

void ReallocChildrenArray(struct iNode* parent) {
    if (parent->children == NULL) {
        parent->size = 0;
    }
    parent->children = (struct iNode **) realloc(parent->children, (parent->size + 1)*sizeof(struct iNode*));
    ++parent->size;
}

void AddExistedChild(struct iNode *parent, struct iNode* child) {
    ReallocChildrenArray(parent);
    parent->children[parent->size - 1] = child;
}

struct iNode* AddNewChild(struct iNode *parent, struct fileTree *file) {
    ReallocChildrenArray(parent);
    parent->children[parent->size - 1] = (struct iNode *) malloc(sizeof(struct iNode));
    parent->children[parent->size - 1]->file = file;
    parent->children[parent->size - 1]->parent = parent;
    parent->children[parent->size - 1]->children = NULL;
    parent->children[parent->size - 1]->size = 0;
    return parent->children[parent->size - 1];
}

void DestructoriNode(struct iNode* sample) {
//    DestructorFile(sample->file);
    DestructorFileTree(sample->file);
    if(sample->parent != NULL) {
        --sample->parent->size;
        if(sample->parent->size == 0) {
            sample->parent->children = NULL;
        }
    }
    free(sample);
}

void DeleteRecursively(struct iNode* sample) {
    if (sample->children == NULL || sample->size == 0) {
        DestructoriNode(sample);
        return;
    }

    for(int i = 0; i < sample->size; ++i) {
        DeleteRecursively(sample->children[i]);
    }
    DestructoriNode(sample);
}

void DeleteChild(struct iNode* child) {
    if (child->parent == NULL) {
        char msg[] = "Unable to delete root... \n";
        send(sock, msg, sizeof(msg), 0);
//        printf("Unable to delete root... \n");
        exit(1);
    }

    if(IsFolder(child)) {
        DeleteRecursively(child);
    } else {
        DestructoriNode(child);
    }

}
