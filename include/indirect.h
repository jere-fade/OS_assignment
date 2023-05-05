#ifndef INDIRECT_H
#define INDIRECT_H
#include "BlockManager.h"
#include "inode.h"
#include "directory.h"
#include <iterator>

struct indirectMeta {
    unsigned short record_num_start;
    unsigned short isdir_start;
    unsigned short start;
};

class Indirect {
public:
    Indirect(unsigned char*, BlockManager);
    void initialize();
    void setRecord(unsigned short);
    unsigned short getRecord();
    bool isDir();
    void setIsDir(bool);
    class iterator: public std::iterator<
        std::input_iterator_tag,
        unsigned short,
        unsigned short,
        const unsigned short*,
        unsigned short> {
            unsigned short it;
        public:
            iterator(unsigned short _it = 4) :it(_it) {}
            iterator& operator++ () {
                it+=2;
                return *this;
            }
            iterator operator++(int) {
                iterator retval = *this;
                ++(*this);
                return retval;
            }
            bool operator==(iterator other) { return it == other.it;}
            bool operator!=(iterator other) { return it != other.it;}
            reference operator*() const {return it;}
            unsigned short value(Indirect* p);
        };
    iterator begin();
    iterator end();
    iterator next(iterator);
    void appendAddress(unsigned char*);
    void deleteAddress(iterator); // 移除并且free相关node
    void removeAddress(iterator); // 仅移除
    void getAddress(iterator, unsigned char*);
private:
    unsigned char* block;
    indirectMeta meta;
    BlockManager manager;
};

#endif