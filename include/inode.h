#ifndef INODE_H
#define INODE_H
#include "indirect.h"
#include "BlockManager.h"
#include "utils.h"
#include <ctime>
#include <iterator>

struct inodeMeta {
    unsigned short name_start;
    unsigned short name_length;
    unsigned short record_num_start;
    unsigned short isdir_start;
    unsigned short ctime_start;
    unsigned short start;
    unsigned short direct_max;
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
    // unsigned short next(unsigned short);
    void free();
    
    class iterator: public std::iterator<
        std::input_iterator_tag,
        unsigned short,
        unsigned short,
        const unsigned short*,
        unsigned short> {
            unsigned short it;
        public:
            iterator(unsigned short _it = Inode::meta.start) :it(_it) {}
            iterator& operator++() {
                it+=2;
                return *this;
            }
            iterator operator++(int) {
                iterator retval = *this;
                ++(*this);
                return retval;
            }
            bool operator==(iterator other) { return it == other.it; }
            bool operator!=(iterator other) { return it != other.it; }
            reference operator*() const {return it;}
            unsigned short value(Inode* p);
        };

    iterator begin();
    iterator end();
    iterator next(iterator);
    void deleteAddress(iterator);
    void getAddress(iterator, unsigned char*);
    static constexpr inodeMeta meta = {
        .name_start = 0,
        .name_length = 62,
        .record_num_start = 62,
        .isdir_start = 64,
        .ctime_start = 65,
        .start = 100,
        .direct_max = 10
    };
private:
    void setRecord(unsigned short);
    unsigned short getRecord();
    unsigned char* block;
    BlockManager manager;
};

#endif
