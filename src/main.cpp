#include <iostream>
#include <fstream>
#include <string>
#include "BlockManager.h"
#include "constant.h"
#include "utils.h"

int main(int, char**) {

    unsigned char** disk;

    std::ifstream input_file ("../data/btrfs.bin", std::ios::in | std::ios::binary);
    if (input_file.is_open()) {
        disk = new unsigned char*[BLOCK_NUM];
        for (int i = 0; i < BLOCK_NUM; i++) {
            disk[i] = new unsigned char[BLOCK_SIZE];
        }
        for (int i = 0; i < BLOCK_NUM; i++) {
            input_file.read((char*)disk[i], BLOCK_SIZE);
        }
        input_file.close();
    }
    else {
        std::cout << "unable to open file to read" << std::endl;
    }

    BlockManager manager(disk);
    bool format_build = false;
    if (format_build) {
        manager.format();
        manager.buildVolume();
    }
    std::cout<<"list head: "<< manager.getHead() <<std::endl;
    std::cout<<"list offset: "<< manager.getOffset() <<std::endl;
    while(1) {

        std::string command;
        std::cout<<"enter command: allocate, free, print, head or exit"<<std::endl;
        std::cin>>command;
        if (command == "allocate") {
            unsigned short node = manager.allocate();
            std::cout<<"allocated node: "<<node<<std::endl;
            unsigned char temp[2];
            shortToByte(1, temp);
            for (int i = 0; i < BLOCK_SIZE; i+=2) {
                disk[node][i] = temp[0];
                disk[node][i+1] = temp[1];
            }
        }
        else if (command == "free") {
            std::cout<<"please enter the block number to free"<<std::endl;
            unsigned short node;
            std::cin>>node;
            for(int i=0; i<BLOCK_SIZE; i++) {
                disk[node][i] = '\0';
            }
            manager.free(node);
        }
        else if (command == "print") {
            std::cout<<"please enter the block number to print"<<std::endl;
            int index;
            std::cin >> index;
            manager.printDisk(index);
        }
        else if (command == "head") {
            std::cout<<"head: "<<manager.getHead()<<std::endl;
            std::cout<<"offset: "<<manager.getOffset()<<std::endl;
        }
        else if (command == "exit") {
            break;
        }
        else {
            std::cout<<"unknown command"<<std::endl;
        }
    }

    std::ofstream output_file;
    output_file.open("../data/btrfs.bin", std::ios::out | std::ios::binary );
    if (output_file.is_open()) {
        for (int i = 0; i < BLOCK_NUM; i++) {
            output_file.write((char*)disk[i], BLOCK_SIZE);
        }
        output_file.close();
    }
    else {
        std::cout << "unable to open file to write" << std::endl;
    }
    


}

