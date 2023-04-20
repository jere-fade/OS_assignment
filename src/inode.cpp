#include "inode.h"
#include "utils.h"

Inode::Inode(unsigned char* block) {
    this->block = block;
    meta.name_start = 0;
    meta.name_length = 14;
    meta.record_num_start = 14;
    meta.isdir_start = 16;
    meta.start = 100;
}

void Inode::initialize() {
    setRecord(0);
}

void Inode::setName(unsigned char* name) {
    for (int i = 0; i < meta.name_length; i++) {
        block[meta.name_start + i] = name[i];
    }
}
void Inode::getName(unsigned char* name) {
    for (int i = 0; i < meta.name_length; i++) {
        name[i] = block[meta.name_start + i];
    }
}

void Inode::setRecord(unsigned short record) {
    unsigned char temp[2];
    shortToByte(record, temp);
    block[meta.record_num_start] = temp[0];
    block[meta.record_num_start + 1] = temp[1];
}

unsigned short Inode::getRecord() {
    unsigned char temp[2];
    temp[0] = block[meta.record_num_start];
    temp[1] = block[meta.record_num_start + 1];
    return byteToShort(temp);
}

void Inode::setIsDir(bool dir) {
    block[meta.isdir_start] = dir;
}

bool Inode::isDir() {
    return (bool) block[meta.isdir_start];
}

void Inode::appendAddress(unsigned char* addr) {
    unsigned short curr = meta.start + getRecord() * 2;
    block[curr] = addr[0];
    block[curr + 1] = addr[1];
    setRecord(getRecord() + 1);
}

void Inode::getAddress(unsigned short iter, unsigned char* addr) {
    addr[0] = block[iter];
    addr[1] = block[iter+1];
}

unsigned short Inode::begin() {
    return meta.start;
} 

unsigned short Inode::end() {
    return meta.start + getRecord() * 2;
}

unsigned short Inode::next(unsigned short iter) {
    return iter+2;
}