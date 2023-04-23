#include "directory.h"
#include "string.h"
#include "utils.h"

Directory::Directory(unsigned char* block, BlockManager manager) {
    this->block = block;
    this->manager = manager;
    meta.record_num_start = 0;
    meta.start = 64;
    meta.entry_length = 64;
    meta.name_length = 62;
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

void Directory::deleteEntry(unsigned short iter) {
    unsigned char temp[2];
    getNum(iter, temp);
    unsigned short node_num = byteToShort(temp);
    Inode node = Inode(manager.disk[node_num], manager);
    node.free();
    manager.free(node_num);
    removeEntry(iter);
}

void Directory::removeEntry(unsigned short iter) {
    unsigned short curr = iter;
    unsigned short nextone = next(curr);
    for (nextone; nextone < end(); nextone = next(nextone)) {
        for (int i = 0; i < meta.entry_length; i++) {
            block[curr + i] = block[nextone + i];
        }
        curr = next(curr);
    }
    setRecord(getRecord()-1);
}

void Directory::getEntry(unsigned short iter, unsigned char* entry) {
    for (int i = 0; i < meta.entry_length; i++) {
        entry[i] = block[iter + i];
    }
}

void Directory::getNum(unsigned short iter, unsigned char* num) {
    num[0] = block[iter];
    num[1] = block[iter+1];
}

void Directory::getName(unsigned short iter, unsigned char* name) {
    for (int i = 0; i < meta.name_length; i++) {
        name[i] = block[iter + 2 + i];
    }
}

unsigned short Directory::begin() {
    return meta.start;
}

unsigned short Directory::end() {
    return meta.start + getRecord() * meta.entry_length;
}

unsigned short Directory::next(unsigned short iter) {
    return iter + meta.entry_length;
}

void Directory::free() {
    unsigned char temp[2];
    unsigned char name[62];
    unsigned char entry_name[62] = "..";
    unsigned short node_num;
    for(auto iter = next(begin()); iter < end(); iter = next(iter)) {
        if(iter == next(begin())) {
            getName(iter, name);
            if(strcmp((char*)name, (char*)entry_name) == 0) {
                continue;
            }
        }
        getNum(iter, temp);
        node_num = byteToShort(temp);
        Inode node = Inode(manager.disk[node_num], manager);
        node.free();
        manager.free(node_num);
    }
}