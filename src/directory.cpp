#include "directory.h"
#include "string.h"
#include "utils.h"

Directory::Directory(unsigned char* block, BlockManager manager) {
    this->block = block;
    this->manager = manager;
}

void Directory::initialize() {
    setRecord(0);
}

void Directory::setRecord(unsigned short record) {
    unsigned char temp[2];
    shortToByte(record, temp);
    block[meta.record_num_start] = temp[0];
    block[meta.record_num_start + 1] = temp[1];
}

unsigned short Directory::getRecord() {
    unsigned char temp[2];
    temp[0] = block[meta.record_num_start];
    temp[1] = block[meta.record_num_start + 1];
    return byteToShort(temp);
}

void Directory::appendEntry(unsigned char* entry) {
    unsigned short curr = meta.start + getRecord() * meta.entry_length;
    for (int i = 0; i < meta.entry_length; i++) {
        block[curr + i] = entry[i];
    }
    setRecord(getRecord() + 1);
}

void Directory::deleteEntry(iterator iter) {
    unsigned short node_num = iter.num(this);
    Inode node = Inode(manager.disk[node_num], manager);
    node.free();
    manager.free(node_num);
    removeEntry(iter);
}

void Directory::removeEntry(iterator iter) {
    iterator curr = iter;
    iterator nextone = next(curr);
    for (nextone; nextone != end(); nextone++) {
        for (int i = 0; i < meta.entry_length; i++) {
            block[*curr + i] = block[*nextone + i];
        }
        curr++;
    }
    setRecord(getRecord()-1);
}

void Directory::getEntry(iterator iter, unsigned char* entry) {
    for (int i = 0; i < meta.entry_length; i++) {
        entry[i] = block[*iter + i];
    }
}

void Directory::getNum(iterator iter, unsigned char* num) {
    num[0] = block[*iter];
    num[1] = block[*iter+1];
}

void Directory::getName(iterator iter, unsigned char* name) {
    for (int i = 0; i < meta.name_length; i++) {
        name[i] = block[*iter + 2 + i];
    }
}

Directory::iterator Directory::begin() {
    return iterator(meta.start);
}

Directory::iterator Directory::end() {
    return iterator(meta.start + getRecord() * meta.entry_length);
}

Directory::iterator Directory::next(iterator iter) {
    return ++iter;
}

void Directory::free() {
    unsigned char temp[2];
    unsigned char name[meta.name_length];
    unsigned char entry_name[meta.name_length] = "..";
    unsigned short node_num;
    for(auto iter = next(begin()); iter != end(); iter++) {
        if(iter == next(begin())) {
            getName(iter, name);
            if(strcmp((char*)name, (char*)entry_name) == 0) {
                continue;
            }
        }
        node_num = iter.num(this);
        Inode node = Inode(manager.disk[node_num], manager);
        node.free();
        manager.free(node_num);
    }
}

unsigned short Directory::iterator::num(Directory* p) {
    unsigned char temp[2];
    temp[0] = p -> block[it];
    temp[1] = p -> block[it+1];
    return byteToShort(temp);
}