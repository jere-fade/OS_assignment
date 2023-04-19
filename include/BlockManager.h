struct Meta{
    unsigned short list_head[2];
    unsigned short list_offset[2];
};

class BlockManager {
public:
    BlockManager(unsigned char**);

    void format();

    void buildVolume();

    unsigned short allocate();

    void free(unsigned short);

    void printDisk(int);

    unsigned short getHead();
    void setHead(unsigned short);
    unsigned short getOffset();
    void setOffset(unsigned short);

private:
    unsigned char** disk;
    struct Meta meta;

};