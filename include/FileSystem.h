#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include"BlockManager.h"

class FileSystem {
public: 
    FileSystem(unsigned char**);
    void initialize();
    bool changeDir(unsigned char* des);
    void getPath(unsigned char* path);
    void createDir(unsigned char* des);
    void deleteDir(unsigned char* des);
    void listDir();
private:
    unsigned char** disk;
    BlockManager manager;
    unsigned short dir;
};
#endif