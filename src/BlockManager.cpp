#include"BlockManager.h"
#include"constant.h"
#include"utils.h"
#include<iostream>

BlockManager::BlockManager() {
    meta.rootDir = 0;
    meta.list_head[0] = 1;
    meta.list_head[1] = 0;
    meta.list_offset[0] = 1;
    meta.list_offset[1] = 2;
    disk = nullptr; 
}

// meta 是存 free list 头部信息的位置, 目前放在block 1
BlockManager::BlockManager(unsigned char** disk) {
    meta.rootDir = 0;
    meta.list_head[0] = 1;
    meta.list_head[1] = 0;
    meta.list_offset[0] = 1;
    meta.list_offset[1] = 2;
    this->disk = disk; 
}

void BlockManager::format() {
    for (int i = 0; i < BLOCK_NUM; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            disk[i][j] = '\0';
        }
    }
}

// use the first block as the root directory
// use the second block to store the meta info, like the head of the free list
// use the third block as the beginning of the free list
void BlockManager::buildVolume() {

    setHead(2);
    setOffset(0);

    for (int i = 3; i < BLOCK_NUM; i++) {
        free(i);
    }

}

unsigned short BlockManager::getRootBlock() {
    return meta.rootDir;
}

unsigned short BlockManager::allocate() {
    if(getOffset() == 0) {
        unsigned char temp[2];
        unsigned short head = getHead();
        temp[0] = disk[head][1022];
        temp[1] = disk[head][1023];
        unsigned short free_node = head;
        setHead(byteToShort(temp));
        
        head = getHead();
        setOffset(1022);
        unsigned short offset = getOffset();
        temp[0] = disk[head][offset-2];
        temp[1] = disk[head][offset-1];
        unsigned short result = byteToShort(temp);
        shortToByte(free_node, temp);
        disk[head][offset-2] = temp[0];
        disk[head][offset-1] = temp[1];
        return result;
    }
    else {
        unsigned char temp[2];
        unsigned short head = getHead();
        unsigned short offset = getOffset();
        temp[0] = disk[head][offset-2];
        temp[1] = disk[head][offset-1];
        setOffset(getOffset()-2);
        return byteToShort(temp);
    }
}

void BlockManager::free(unsigned short block_num) {

    if(getOffset() == 1022) {
        unsigned char temp[2];
        unsigned short head = getHead();
        temp[0] = disk[head][1020];
        temp[1] = disk[head][1021];
        unsigned short next_node = head;
        setHead(byteToShort(temp));
        setOffset(0);

        shortToByte(block_num, temp);
        disk[next_node][1020] = temp[0];
        disk[next_node][1021] = temp[1];

        head = getHead();
        shortToByte(next_node, temp);
        disk[head][1022] = temp[0];
        disk[head][1023] = temp[1];
    }
    else {
        unsigned char temp[2];
        unsigned short head = getHead();
        unsigned short offset = getOffset();
        shortToByte(block_num, temp);
        disk[head][offset] = temp[0];
        disk[head][offset+1] = temp[1];
        setOffset(getOffset()+2);
    }
}

void BlockManager::printDisk(int index) {

    std::cout<<"######## block: "<<index <<std::endl;
    for (int i = 0; i < BLOCK_SIZE; i+=2) {
        unsigned char temp[2];
        if (index == getHead() && i == getOffset()) {
            std::cout<<" || ";
        }
        temp[0] = disk[index][i];
        temp[1] = disk[index][i+1];
        unsigned short symbol = byteToShort(temp);
        std::cout<<symbol<<" ";
    }
    std::cout<<std::endl;
    std::cout<<"#########"<<std::endl;
}

unsigned short BlockManager::getHead() {
    unsigned char temp[2];
    temp[0] = disk[meta.list_head[0]][meta.list_head[1]];
    temp[1] = disk[meta.list_head[0]][meta.list_head[1]+1];
    return byteToShort(temp);
}

void BlockManager::setHead(unsigned short num) {
    unsigned char temp[2];
    shortToByte(num, temp);
    disk[meta.list_head[0]][meta.list_head[1]] = temp[0];
    disk[meta.list_head[0]][meta.list_head[1]+1] = temp[1];
}

unsigned short BlockManager::getOffset() {
    unsigned char temp[2];
    temp[0] = disk[meta.list_offset[0]][meta.list_offset[1]];
    temp[1] = disk[meta.list_offset[0]][meta.list_offset[1]+1];
    return byteToShort(temp);
}

void BlockManager::setOffset(unsigned short num) {
    unsigned char temp[2];
    shortToByte(num, temp);
    disk[meta.list_offset[0]][meta.list_offset[1]] = temp[0];
    disk[meta.list_offset[0]][meta.list_offset[1]+1] = temp[1];
}

unsigned short BlockManager::listSize() {
    unsigned short currHead = getHead();
    unsigned short sum = 0;
    unsigned char temp[2];
    sum += getOffset() / 2;
    while (true) {
        temp[0] = disk[currHead][1022];
        temp[1] = disk[currHead][1023];
        currHead = byteToShort(temp);
        if(currHead == 0) {
            break;
        }
        sum += 511;
    }
    return sum;
}