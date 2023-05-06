#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

struct Meta{
    unsigned short rootDir;
    unsigned short list_head[2];
    unsigned short list_offset[2];
    unsigned short has_initialized[2];
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

    unsigned short getHead();
    void setHead(unsigned short);
    unsigned short getOffset();
    void setOffset(unsigned short);
    bool getIni();
    void setIni(bool);

    unsigned short listSize();

    unsigned char** disk;

    static constexpr struct Meta meta {
        .rootDir = 0,
        .list_head = {1, 0},
        .list_offset = {1, 2},
        .has_initialized = {1, 4}
    };

};

#endif