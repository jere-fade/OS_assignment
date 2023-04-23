#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "BlockManager.h"
#include "inode.h"

struct dirMeta {
    unsigned short record_num_start;
    unsigned short start;
    unsigned short entry_length;
    unsigned short name_length;
};

class Directory {
public:
    Directory(unsigned char*, BlockManager);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    void appendEntry(unsigned char* entry);
    void deleteEntry(unsigned short iter);
    void removeEntry(unsigned short iter);
    void getEntry(unsigned short iter, unsigned char* entry);
    void getNum(unsigned short iter, unsigned char* num);
    void getName(unsigned short iter, unsigned char* name);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
    void free();
private:
    unsigned char* block;
    dirMeta meta;
    BlockManager manager;
};
#endif