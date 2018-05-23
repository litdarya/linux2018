#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "main.h"
#include "iNode.h"
#include "file.h"


int* free_ranges;
int free_ranges_size = 0;
int sock;

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
        if (buff[i] == delimeter || buff[i] == '\n') {
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
    char* ans = (char*)malloc(sizeof(char)*(i + 1));
    strncpy(ans, buff, i);
    ans[i] = 0;
    return ans;
}

void Touch(struct iNode* folder, char* buff) {
    char* filename;
    char* file;

    filename = GetArg(buff, ' ');

    if (strlen(filename) == 0) {
        char msg[] = "please add content... \n";
        send(sock, msg, sizeof(msg), 0);
        return;
    }

    file = GetArg(buff + strlen(filename) + 1, '\n');

    if(strlen(file) == 0) {
        char msg[] = "please add content... \n";
        send(sock, msg, sizeof(msg), 0);
        return;
    }

    struct fileTree* res = WriteFile(filename, file);
    AddNewChild(folder, res);
    char msg[] = "ok \n";
    send(sock, msg, sizeof(msg), 0);
}

struct iNode* Cd(struct iNode* root, char* buff) {
    char* addr = GetArg(buff, '\n');

    if (strlen(addr) == 0) {
        char msg[] = "no such directory \n";
        send(sock, msg, sizeof(msg), 0);

        return root;
    }

    char* part = GetArg(addr, '/');
    int shift = 0;

    struct iNode* res = root;
    int param = 1;
    while(strlen(part) > 0) {
        shift += strlen(part);
        res = Find(res, part, &param);
        if (res == NULL) {
            char msg[] = "no such directory \n";
            send(sock, msg, sizeof(msg), 0);

            return root;
        }
        part = GetArg(addr + shift, '/');
    }

    char msg[] = "ok \n";
    send(sock, msg, sizeof(msg), 0);

    return res;
}

void Ls(struct iNode* folder, char* buff) {
    char* filename = GetArg(buff, '\n');
    if (strlen(filename) != 0) {
        int param = 0;
        struct iNode* res = Find(folder, filename, &param);
        if (res == NULL) {
            char msg[] = "File is not found \n";
            send(sock, msg, sizeof(msg), 0);
        } else {
            char* info = GetContentNew(res->file);
            send(sock, info, strlen(info), 0);
            free(info);
        }
    } else {
        PrintChildren(folder);
    }
}

struct iNode* Up(struct iNode* root) {
    if (root->parent == NULL) {
        char msg[] = "Impossible \n";
        send(sock, msg, sizeof(msg), 0);
//        printf("Impossible \n");
        return root;
    }
    char msg[] = "ok \n";
    send(sock, msg, sizeof(msg), 0);
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
        char msg[] = "Not found \n";
        send(sock, msg, sizeof(msg), 0);
//        printf("Not found \n");
        return;
    }
    DeleteChild(res);
    DeleteElem(folder, ind);
    if(folder->size == 0) {
        folder->children = NULL;
    }
    char msg[] = "ok \n";
    send(sock, msg, sizeof(msg), 0);
}

struct iNode* MakeDir(struct iNode* parent, char* buffer) {
    char* dir_name = GetArg(buffer, '\n');
    if (strlen(dir_name) == 0) {
        char msg[] = "not empty dir please! \n";
        send(sock, msg, sizeof(msg), 0);
        return parent;
    }
    struct fileTree* file = WriteDir(dir_name);
    struct iNode* res = AddNewChild(parent, file);
    char msg[] = "ok \n";
    send(sock, msg, sizeof(msg), 0);
    return res;
}

void daemonize() {
    pid_t pid;
    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/tmp");

    for (int i = sysconf(_SC_OPEN_MAX); i >= 0; --i) {
        close(i);
    }

    openlog("Daemon", LOG_PID, LOG_DAEMON);
}

void process(char* buff, int size, struct iNode** root) {
    char* command = GetCommand(buff, size);

    if (strcmp(command, "touch") == 0) {
        Touch(*root, buff + strlen(command) + 1);
    } else if (strcmp(command, "makedir") == 0) {
        MakeDir(*root, buff + strlen(command) + 1);
    } else if (strcmp(command, "cd") == 0) {
        *root = Cd(*root, buff + strlen(command) + 1);
    } else if (strcmp(command, "ls") == 0) {
        Ls(*root, buff + strlen(command) + 1);
    } else if (strcmp(command, "..") == 0) {
        *root = Up(*root);
    } else if (strcmp(command, "rm") == 0) {
        Rm(*root, buff + strlen(command) + 1);
    } else {
        char msg[] = "No such command... \n";
        send(sock, msg, sizeof(msg), 0);
    }

    free(command);
}

int main() {
    daemonize();
    int size = BUFFER_SIZE;
    char* buff = (char*)malloc(size);
    int listener, bytes_read;
    struct sockaddr_in addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 1);

    struct iNode* root = (struct iNode*)malloc(sizeof(struct iNode));
    root->children = NULL;
    root->parent = NULL;
    char* root_name = (char*)malloc(2*sizeof(char));
    root_name[0] = '/';
    root_name[1] = '\0';
    root->file = WriteDir(root_name);

    while(1) {
        sock = accept(listener, NULL, NULL);
        printf("Accepted... \n");
        while(1) {
            bytes_read = recv(sock, buff, size, 0);

            if(bytes_read <= 0) {
                break;
            }

            printf("%s", buff);
            process(buff, size, &root);
        }
        close(sock);
    }

    free(free_ranges);
    free(buff);

    syslog (LOG_NOTICE, "Daemon terminated.");
    closelog();

    return EXIT_SUCCESS;
}