#include "inode.h"
#include "utils.h"
#include <iostream>
#include <ctime>

Inode::Inode(unsigned char* block, BlockManager manager) {
    this->block = block;
    this->manager = manager;
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
    if(getRecord() >= 11) {
        unsigned char temp[2];
        unsigned short pos = meta.start + 10 * 2;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        Indirect indirect = Indirect(manager.disk[byteToShort(temp)], manager);
        return indirect.getRecord() + getRecord() - 1;
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
    if (getRecord() >= 11) {
        unsigned char temp[2];
        unsigned short pos = meta.start + 2 * 10;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        Indirect indirect = Indirect(manager.disk[byteToShort(temp)], manager);
        if(indirect.getRecord() == 510) {
            std::cout<<"[ERROR] Inode Reach Maximum Capacity"<<std::endl;
            exit(1);
        }
        indirect.appendAddress(addr);
    }
    else {
        if(getRecord() == 10) {
            unsigned short indir_num = manager.allocate();
            unsigned char temp[2];
            unsigned short pos = meta.start + 10*2;
            shortToByte(indir_num, temp);
            block[pos] = temp[0];
            block[pos+1] = temp[1];
            setRecord(getRecord() + 1);
            Indirect indirect = Indirect(manager.disk[indir_num], manager);
            indirect.initialize();
            indirect.setIsDir(isDir());
            indirect.appendAddress(addr);
        }
        else {
            unsigned short curr = meta.start + getRecord() * 2;
            block[curr] = addr[0];
            block[curr + 1] = addr[1];
            setRecord(getRecord() + 1);
        }
    }

}

void Inode::deleteAddress(unsigned short iter) {
    
    if (iter < meta.start + 10*2) {
        unsigned char num_temp[2];
        getAddress(iter, num_temp);
        if(isDir()) {
            unsigned short dir_num = byteToShort(num_temp);
            Directory directory = Directory(manager.disk[dir_num], manager);
            directory.free();
            manager.free(dir_num);
        }
        else {
            unsigned short node_num = byteToShort(num_temp);
            manager.free(node_num);
        }

        if(getRecord() >= 11) {
            unsigned char temp[2];
            unsigned char addr[2];
            unsigned short pos = meta.start + 10*2;
            temp[0] = block[pos];
            temp[1] = block[pos+1];
            unsigned short indir_num = byteToShort(temp);
            Indirect indirect = Indirect(manager.disk[indir_num], manager);           
            unsigned short indir_iter = indirect.begin();
            for(indir_iter; next(indir_iter) < indirect.end(); indir_iter = indirect.next(indir_iter)) {}
            indirect.getAddress(indir_iter, addr);
            block[iter] = addr[0];
            block[iter+1] = addr[1];
            indirect.removeAddress(indir_iter);
            if(indirect.getRecord() == 0) {
                setRecord(getRecord()-1);
                manager.free(indir_num);
            }
        }
        else {
            unsigned short curr = iter;
            unsigned short nextone = next(curr);
            for(nextone; nextone < end(); nextone = next(nextone)) {
                block[curr] = block[nextone];
                block[curr+1] = block[nextone+1];
                curr = next(curr);
            }
            setRecord(getRecord()-1);
        }
    }
    else {
        unsigned char temp[2];
        unsigned short pos = meta.start + 10*2;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        unsigned short indir_num = byteToShort(temp);
        Indirect indirect = Indirect(manager.disk[indir_num], manager);
        unsigned short converted = iter - (meta.start + 10*2) + indirect.begin();
        indirect.deleteAddress(converted);
        if(indirect.getRecord() == 0) {
            setRecord(getRecord()-1);
            manager.free(indir_num);
        }
    }
}

void Inode::getAddress(unsigned short iter, unsigned char* addr) {

    if(iter < meta.start + 10*2) {
        addr[0] = block[iter];
        addr[1] = block[iter+1];
    }
    else {
        unsigned char temp[2];
        unsigned short pos = meta.start + 10 * 2;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        Indirect indirect = Indirect(manager.disk[byteToShort(temp)], manager);
        unsigned short converted = iter - (meta.start + 10*2) + indirect.begin();
        indirect.getAddress(converted, addr);
    }

}

unsigned short Inode::begin() {
    return meta.start;
} 

unsigned short Inode::end() {
    if(getRecord() >= 11) {
        unsigned char temp[2];
        unsigned short pos = meta.start + 10 * 2;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        Indirect indirect = Indirect(manager.disk[byteToShort(temp)], manager);
        return meta.start + 10 * 2 + indirect.end() - indirect.begin();
    }
    else {
        return meta.start + getRecord() * 2;
    }
}

unsigned short Inode::next(unsigned short iter) {
    return iter + 2;
}

void Inode::free() {
    bool has_indirect = (getRecord() >= 11? true: false);
    unsigned char temp[2];
    if(isDir()) {
        for (auto iter = begin(); iter < end(); iter = next(iter)) {
            getAddress(iter, temp);
            unsigned short dir_num = byteToShort(temp);
            Directory directory = Directory(manager.disk[dir_num], manager);
            directory.free();
            manager.free(dir_num);
        }

    }
    else {
        for (auto iter = begin(); iter < end(); iter = next(iter)) {
            getAddress(iter, temp);
            manager.free(byteToShort(temp));
        }
    }

    if(has_indirect) {
        unsigned short pos = meta.start + 10*2;
        temp[0] = block[pos];
        temp[1] = block[pos+1];
        manager.free(byteToShort(temp));
    }
}