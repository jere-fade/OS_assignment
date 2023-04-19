#include "utils.h"

void shortToByte(unsigned short num, unsigned char* byte) {
    byte[1] = num & 0xFF;
    byte[0] = (num >> 8) & 0xFF;
}

unsigned short byteToShort(unsigned char* byte) {
    return (byte[0] << 8) + byte[1];
}
    // unsigned short a = 1000;
    // unsigned char b[2];

    // b[0] = a & 0xFF;
    // b[1] = (a >> 8) & 0xFF;

    // unsigned short c;
    // c = b[0] + (b[1] << 8);

    // std::cout<<(int) a<<std::endl;