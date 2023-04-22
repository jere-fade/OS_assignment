#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

struct Meta{
    unsigned short rootDir;
    unsigned short list_head[2];
    unsigned short list_offset[2];
};

class BlockManager {
public:
    BlockManager();
    BlockManager(unsigned char**);

    void format();

    void buildVolume();

    unsigned short allocate();
    
    unsigned short getRootBlock();

    void free(unsigned short);

    void printDisk(int);

    unsigned short getHead();
    void setHead(unsigned short);
    unsigned short getOffset();
    void setOffset(unsigned short);

    unsigned short listSize();

    unsigned char** disk;

private:
    struct Meta meta;

};

#endif