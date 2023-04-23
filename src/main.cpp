#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <bitset>
#include <ctime>
#include "BlockManager.h"
#include "inode.h"
#include "directory.h"
#include "constant.h"
#include "utils.h"
#include "FileSystem.h"
#include "path.h"

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
    bool format_build = true;
    if (format_build) {
        manager.format();
        manager.buildVolume();
    }
    std::cout<<"list head: "<< manager.getHead() <<std::endl;
    std::cout<<"list offset: "<< manager.getOffset() <<std::endl;
    std::cout<<"list size: "<< manager.listSize() << std::endl;

    // unsigned short node_num = manager.allocate();
    // Inode node = Inode(disk[node_num], manager);
    // unsigned char temp[2];

    // for(unsigned short i = 0; i < 520; i++) {
    //     shortToByte(i, temp);
    //     node.appendAddress(temp);
    // }

    // for(int i = 0; i < 500; i++) {
    //     auto iter = node.begin();
    //     node.deleteAddress(iter);
    // } 


    // manager.printDisk(16383);

    // for(auto i = node.begin(); i < node.end(); i = node.next(i)) {
    //     node.getAddress(i, temp);
    //     std::cout<<byteToShort(temp)<<" ";
    // }
    // std::cout<<std::endl;


    unsigned char path[2048];
    FileSystem fs = FileSystem(disk);
    fs.initialize();
    std::cout<<"list size: "<<manager.listSize()<<std::endl;
    for(int i = 0; i < 200; i++) {
        sprintf((char*)path, "%d", i);
        fs.createDir(path);
    }
    fs.listDir();
    std::cout<<"list size: "<<manager.listSize()<<std::endl;
    std::cout<<"--------------"<<std::endl;
    for(int i = 0; i < 180; i++) {
        sprintf((char*)path, "%d", i);
        fs.deleteDir(path);
    }
    fs.listDir();
    std::cout<<"list size: "<<manager.listSize()<<std::endl;
    // fs.createDir((unsigned char*)"/etc");
    // fs.createDir((unsigned char*)"etc/nginx");
    // fs.createDir((unsigned char*)"etc/xray");
    // fs.createDir((unsigned char*)"etc/nginx/log");
    // fs.changeDir((unsigned char*)"./etc/nginx");
    // fs.createDir((unsigned char*)"./error");
    // fs.createDir((unsigned char*)"../x2ray");
    // fs.listDir();
    // BlockManager manager1(disk);
    // unsigned char path[2048];
    // fs.getPath(path);
    // std::cout<<path<<std::endl;
    // Path path((unsigned char*)"nginx/log/error.log");
    // unsigned char temp[2048];
    // unsigned char name[62];
    // path.separate(temp, name);
    // std::cout<<"path: "<<temp<<std::endl;
    // std::cout<<"name: "<<name<<std::endl;

    // std::time_t time = std::time(nullptr);
    // std::cout<<time<<std::endl;
    // std::cout<< std::asctime(std::localtime(&time))<<std::endl;

    // unsigned short node_num = manager.allocate();
    // printf("%s%d", "allocated node: ", node_num);
    // std::time_t time = std::time(nullptr);

    // Inode node = Inode(disk[16381]);
    // std::time_t time = node.getCtime();
    // std::cout<<time<<std::endl;
    // std::cout<< std::asctime(std::localtime(&time))<<std::endl;
    
    // Inode node = Inode(disk[16383]);
    // unsigned char temp[14];
    // node.getName(temp);
    // printf("%s\n", temp);

    // printf("%d\n", node.getRecord());

    // unsigned char addr[2];
    // for(unsigned short iter = node.begin(); iter < node.end(); iter=node.next(iter)) {
    //     node.getAddress(iter, addr);
    //     std::cout<<byteToShort(addr)<<" ";
    // }

    // unsigned short num = 64;
    // unsigned char temp[3];
    // unsigned short offset = 256;
    // shortToAddress(num, temp);
    // offsetToAddress(offset, temp);
    // unsigned short c = addressToShort(temp);
    // unsigned short o = addressToOffset(temp);
    // std::cout<<c<<std::endl;
    // std::cout<<o<<std::endl;

    // std::bitset<8> x(temp[0]);
    // std::bitset<8> y(temp[1]);
    // std::bitset<8> z(temp[2]);
    // std::cout<<x<<" "<<y<<" "<<z<<std::endl;
    
    // while(1) {

    //     std::string command;
    //     std::cout<<"enter command: allocate, free, print, head or exit"<<std::endl;
    //     std::cin>>command;
    //     if (command == "allocate") {
    //         unsigned short node = manager.allocate();
    //         std::cout<<"allocated node: "<<node<<std::endl;
    //         unsigned char temp[2];
    //         shortToByte(1, temp);
    //         for (int i = 0; i < BLOCK_SIZE; i+=2) {
    //             disk[node][i] = temp[0];
    //             disk[node][i+1] = temp[1];
    //         }
    //     }
    //     else if (command == "free") {
    //         std::cout<<"please enter the block number to free"<<std::endl;
    //         unsigned short node;
    //         std::cin>>node;
    //         for(int i=0; i<BLOCK_SIZE; i++) {
    //             disk[node][i] = '\0';
    //         }
    //         manager.free(node);
    //     }
    //     else if (command == "print") {
    //         std::cout<<"please enter the block number to print"<<std::endl;
    //         int index;
    //         std::cin >> index;
    //         manager.printDisk(index);
    //     }
    //     else if (command == "head") {
    //         std::cout<<"head: "<<manager.getHead()<<std::endl;
    //         std::cout<<"offset: "<<manager.getOffset()<<std::endl;
    //     }
    //     else if (command == "exit") {
    //         break;
    //     }
    //     else {
    //         std::cout<<"unknown command"<<std::endl;
    //     }
    // }

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

