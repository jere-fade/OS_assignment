#include "indirect.h"
#include "utils.h"

// 4 byte 存储信息
// 1020 byte 用来存储 address, 最多510条

Indirect::Indirect(unsigned char* block) {
    meta.record_num_start = 0;
    meta.start = 4;
    this->block = block;
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

void Indirect::appendAddress(unsigned char* addr) {
    unsigned short curr = meta.start + getRecord() * 2;
    block[curr] = addr[0];
    block[curr+1] = addr[1]; 
    setRecord(getRecord()+1);
}

void Indirect::getAddress(unsigned short iter, unsigned char* addr) {
    addr[0] = block[iter];
    addr[1] = block[iter+1];
}

unsigned short Indirect::begin() {
    return meta.start;
}

unsigned short Indirect::end() {
    return meta.start + getRecord() * 2;
}

unsigned short Indirect::next(unsigned short iter) {
    return iter + 2;
}