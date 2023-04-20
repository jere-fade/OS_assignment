#include "directory.h"
#include "utils.h"

Directory::Directory(unsigned char* block) {
    this->block = block;
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
    unsigned short curr = meta.start + getRecord() * 3;
    for (int i = 0; i < meta.entry_length; i++) {
        block[curr + i] = entry[i];
    }
    setRecord(getRecord() + 1);
}

void Directory::getEntry(unsigned short iter, unsigned char* entry) {
    for (int i = 0; i < meta.entry_length; i++) {
        entry[i] = block[iter + i];
    }
}

unsigned short Directory::begin() {
    return meta.start;
}

unsigned short Directory::end() {
    return meta.start + getRecord() * 16;
}

unsigned short Directory::next(unsigned short iter) {
    return iter + 16;
}