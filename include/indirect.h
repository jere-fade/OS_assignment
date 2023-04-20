struct indirectMeta {
    unsigned short record_num_start;
    unsigned short start;
};

class Indirect {
public:
    Indirect(unsigned char*);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    void appendAddress(unsigned char*);
    void getAddress(unsigned short, unsigned char*);
    unsigned short begin();
    unsigned short end();
    unsigned short next(unsigned short);
private:
    unsigned char* block;
    indirectMeta meta;
};