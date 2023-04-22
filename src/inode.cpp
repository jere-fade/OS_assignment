#include "inode.h"
#include "utils.h"
#include <iostream>
#include <ctime>

Inode::Inode(unsigned char* block, BlockManager manager) {
    this->block = block;
    this->manager = manager;
    this->indir = nullptr;
    meta.name_start = 0;
    meta.name_length = 62;
    meta.record_num_start = 62;
    meta.isdir_start = 64;
    meta.ctime_start = 65;
    meta.start = 100;
}

void Inode::initialize() {
    setRecord(0);
    setCtime(std::time(nullptr));
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

unsigned short Inode::getTotalRecord() {
    if(indir != nullptr) {
        Indirect indirect = Indirect(indir);
        return indirect.getRecord() + getRecord()-1;
    }
    else {
        return getRecord();
    }
}

void Inode::setIsDir(bool dir) {
    block[meta.isdir_start] = dir;
}

bool Inode::isDir() {
    return (bool) block[meta.isdir_start];
}

void Inode::setCtime(std::time_t time) {
    for (int i = 0; i < 8; i++) {
        block[meta.ctime_start + i] = (time >> (7-i)*8 ) & 0xFF; 
    }
}

std::time_t Inode::getCtime() {
    std::time_t result = 0;
    for (int i = 0; i < 8; i++) {
        result += (block[meta.ctime_start + i] << ((7-i)*8));
    }
    return result;
}

void Inode::appendAddress(unsigned char* addr) {
    if (indir != nullptr) {
        Indirect indirect = Indirect(indir);
        if(indirect.getRecord() == 510) {
            std::cout<<"[ERROR] Inode Reach Maximum Capacity"<<std::endl;
            exit(1);
        }
        indirect.appendAddress(addr);
    }
    else {
        unsigned short curr = meta.start + getRecord() * 2;
        block[curr] = addr[0];
        block[curr + 1] = addr[1];
        setRecord(getRecord() + 1);
        if(getRecord() == 11) {
            unsigned char** disk = nullptr;
            indir = manager.disk[manager.allocate()];
            Indirect indirect = Indirect(indir);
            indirect.appendAddress(addr);
        }
    }

}

void Inode::getAddress(unsigned short iter, unsigned char* addr) {

    if(iter < meta.start + 10*2) {
        addr[0] = block[iter];
        addr[1] = block[iter+1];
    }
    else {
        Indirect indirect = Indirect(indir);
        unsigned short converted = iter - (meta.start + 10*2) + indirect.begin();
        indirect.getAddress(converted, addr);
    }

}

unsigned short Inode::begin() {
    return meta.start;
} 

unsigned short Inode::end() {
    if(indir != nullptr) {
        Indirect indirect = Indirect(indir);
        return meta.start + 10 * 2 + indirect.end() - indirect.begin();
    }
    else {
        return meta.start + getRecord() * 2;
    }
}

unsigned short Inode::next(unsigned short iter) {
    return iter + 2;
}