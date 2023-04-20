struct dirMeta {
    unsigned short record_num_start;
    unsigned short start;
};

class Directory {
public:
    Directory(unsigned char*);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    void appendEntry(unsigned char* entry);
    void getEntry(unsigned short iter, unsigned char* entry);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
private:
    unsigned char* block;
    dirMeta meta;
};

// 每个directory entry 16 byte, 2byte inode number + 14 byte name
// 头32 byte存储信息 (目前存2byte record num), 后面存directory entry (最多62个)