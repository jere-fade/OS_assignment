#ifndef INDIRECT_H
#define INDIRECT_H
#include "BlockManager.h"
#include "inode.h"
#include "directory.h"

struct indirectMeta {
    unsigned short record_num_start;
    unsigned short isdir_start;
    unsigned short start;
};

class Indirect {
public:
    Indirect(unsigned char*, BlockManager);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    void appendAddress(unsigned char*);
    void deleteAddress(unsigned short); // 移除并且free相关node
    void removeAddress(unsigned short); // 仅移除
    void getAddress(unsigned short, unsigned char*);
    bool isDir();
    void setIsDir(bool);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
private:
    unsigned char* block;
    indirectMeta meta;
    BlockManager manager;
};

#endif