struct inodeMeta {
    unsigned short name_start;
    unsigned short name_length;
    unsigned short record_num_start;
    unsigned short isdir_start;
    unsigned short start;
};

class Inode {
public:
    Inode(unsigned char*);
    void initialize();
    void setName(unsigned char*);
    void getName(unsigned char*);
    void setRecord(unsigned short);
    unsigned short getRecord();
    void setIsDir(bool);
    bool isDir();
    void appendAddress(unsigned char*);
    void getAddress(unsigned short, unsigned char*);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
    
private:
    unsigned char* block;
    inodeMeta meta;
};

// first 100 byte used to store inode attribute
// each block address 2 byte, 11 address, 22 byte

