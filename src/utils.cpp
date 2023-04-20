#include "utils.h"

// 这两个用于将inode number从directory entry 的 2 byte field中读写

void shortToByte(unsigned short num, unsigned char* byte) {
    byte[1] = num & 0xFF;
    byte[0] = (num >> 8) & 0xFF;
}

unsigned short byteToShort(unsigned char* byte) {
    return (byte[0] << 8) + byte[1];
}

// 这两个用于将14bit inode number从24 bit address中读写

void shortToAddress(unsigned short num, unsigned char* addr) {
    addr[0] = (num >> 6) & 0xFF;
    auto bit_mask = 0b11111100;
    addr[1] = (addr[1] & (~bit_mask)) | ((num & 0b111111) << 2);
}

unsigned short addressToShort(unsigned char* addr) {
    return (addr[0] << 6) + ((addr[1]>>2) & 0b111111);
}

// 这两个用于将10bit offset 从24bit address中读写

void offsetToAddress(unsigned short offset, unsigned char* addr) {
    addr[2] = offset & 0xFF;
    auto bit_mask = 0b00000011;
    addr[1] = (addr[1] & (~bit_mask)) | (offset >> 8);
}

unsigned short addressToOffset(unsigned char* addr) {
    return ((addr[1] & 0b11) << 8) + addr[2];
}
    // unsigned short a = 1000;
    // unsigned char b[2];

    // b[0] = a & 0xFF;
    // b[1] = (a >> 8) & 0xFF;

    // unsigned short c;
    // c = b[0] + (b[1] << 8);

    // std::cout<<(int) a<<std::endl;