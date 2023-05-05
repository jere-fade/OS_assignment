#include "indirect.h"
#include "utils.h"

// 4 byte 存储信息
// 1020 byte 用来存储 address, 最多510条

Indirect::Indirect(unsigned char* block, BlockManager manager) {
    meta.record_num_start = 0;
    meta.isdir_start = 2;
    meta.start = 4;
    this->block = block;
    this->manager = manager;
}

void Indirect::initialize() {
    setRecord(0);
}

void Indirect::setRecord(unsigned short record) {
    unsigned char temp[2];
    shortToByte(record, temp);
    block[meta.record_num_start] = temp[0];
    block[meta.record_num_start+1] = temp[1]; 
}

unsigned short Indirect::getRecord() {
    unsigned char temp[2];
    temp[0] = block[meta.record_num_start];
    temp[1] = block[meta.record_num_start+1];
    return byteToShort(temp);
}

void Indirect::setIsDir(bool dir) {
    block[meta.isdir_start] = dir;
}

bool Indirect::isDir() {
    return (bool) block[meta.isdir_start];
}

void Indirect::appendAddress(unsigned char* addr) {
    unsigned short curr = meta.start + getRecord() * 2;
    block[curr] = addr[0];
    block[curr+1] = addr[1]; 
    setRecord(getRecord()+1);
}

void Indirect::removeAddress(iterator iter) {
    iterator curr = iter;
    iterator nextone = next(curr);
    for(nextone; nextone != end(); nextone++) {
        block[*curr] = block[*nextone];
        block[*curr+1] = block[*nextone+1];
        curr++;
    }
    setRecord(getRecord()-1);
}

void Indirect::deleteAddress(iterator iter) {
    if(isDir()) {
        unsigned short dir_num = iter.value(this);
        Directory directory = Directory(manager.disk[dir_num], manager);
        directory.free();
        manager.free(dir_num);
    }
    else {
        unsigned short node_num = iter.value(this);
        manager.free(node_num);
    }
    removeAddress(iter);
}

void Indirect::getAddress(iterator iter, unsigned char* addr) {
    addr[0] = block[*iter];
    addr[1] = block[*iter+1];
}

Indirect::iterator Indirect::begin() {
    return iterator(meta.start);
}

Indirect::iterator Indirect::end() {
    return iterator(meta.start + getRecord() * 2);
}

Indirect::iterator Indirect::next(iterator iter) {
    return ++iter;
}

unsigned short Indirect::iterator::value(Indirect* p) {
    unsigned char temp[2];
    temp[0] = p -> block[it];
    temp[1] = p -> block[it+1];
    return byteToShort(temp);
}