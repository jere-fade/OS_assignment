#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "BlockManager.h"
#include "inode.h"
#include <iterator>

struct dirMeta {
    unsigned short record_num_start;
    unsigned short start;
    unsigned short entry_length;
    unsigned short name_length;
    unsigned short max;
};

class Directory {
public:
    Directory(unsigned char*, BlockManager);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    void appendEntry(unsigned char* entry);
    class iterator: public std::iterator<
        std::input_iterator_tag,
        unsigned short,
        unsigned short,
        const unsigned short*,
        unsigned short> {
            unsigned short it;
        public:
            iterator(unsigned short _it = Directory::meta.start) :it(_it) {}
            iterator& operator++() {
                it+=64;
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
            unsigned short num(Directory* p);
        };
    void deleteEntry(iterator iter);
    void removeEntry(iterator iter);
    void getEntry(iterator iter, unsigned char* entry);
    void getNum(iterator iter, unsigned char* num);
    void getName(iterator iter, unsigned char* name);
    iterator begin();
    iterator end();
    iterator next(iterator);
    void free();
    static constexpr dirMeta meta = {
        .record_num_start = 0,
        .start = 64,
        .entry_length = 64,
        .name_length = 62,
        .max = 15
    };
private:
    unsigned char* block;
    BlockManager manager;
};
#endif