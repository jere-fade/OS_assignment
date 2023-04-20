struct dirMeta {
    unsigned short record_num_start;
    unsigned short start;
    unsigned short entry_length;
    unsigned short name_length;
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
