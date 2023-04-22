#include "indirect.h"
#include "BlockManager.h"
#include <ctime>

struct inodeMeta {
    unsigned short name_start;
    unsigned short name_length;
    unsigned short record_num_start;
    unsigned short isdir_start;
    unsigned short ctime_start;
    unsigned short start;
};

class Inode {
public:
    Inode(unsigned char*, BlockManager);
    void initialize();
    void setName(unsigned char*);
    void getName(unsigned char*);
    unsigned short getTotalRecord();
    void setIsDir(bool);
    bool isDir();
    void setCtime(std::time_t);
    std::time_t getCtime();
    void appendAddress(unsigned char*);
    void getAddress(unsigned short, unsigned char*);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
    
private:
    void setRecord(unsigned short);
    unsigned short getRecord();
    unsigned char* block;
    inodeMeta meta;
    BlockManager manager;
    unsigned char* indir;
};

