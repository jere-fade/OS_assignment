#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include"BlockManager.h"
#include"inode.h"

class FileSystem {
public: 
    FileSystem(unsigned char**);
    void initialize();
    bool changeDir(char* des);
    void getPath(char* path);
    void createDir(char* des);
    void deleteDir(char* des);
    void createFile(char* des, unsigned short size);
    void deleteFile(char* des);
    void listDir(char* parameter);
    void printTree();
    void concatenate(char* des);
    void copyFile(char* src, char* des);
    void usage();
private:
    void printTreeHelper(unsigned short, int);
    void fillFile(unsigned short block_num);
    unsigned char** disk;
    BlockManager manager;
    unsigned short dir;
};
#endif