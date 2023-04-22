#include "FileSystem.h"
#include "inode.h"
#include "directory.h"
#include "utils.h"
#include "path.h"
#include "string.h"
#include <iostream>

FileSystem::FileSystem(unsigned char** disk) {
    this->disk = disk;
    manager = BlockManager(disk);
    dir = manager.getRootBlock();
}

void FileSystem::initialize() {
    unsigned short root_num = manager.getRootBlock();
    Inode root_node = Inode(disk[root_num], manager);
    root_node.initialize();
    unsigned char name[62];
    name[0] = '\0';
    root_node.setName(name);
    root_node.setIsDir(true);

    unsigned short dir_node_num = manager.allocate();
    Directory dir_node = Directory(disk[dir_node_num]);
    dir_node.initialize();

    unsigned char entry_num[2];
    shortToByte(root_num, entry_num);
    unsigned char entry[64];
    entry[0] = entry_num[0];
    entry[1] = entry_num[1];
    entry[2] = '.';
    entry[3] = '\0';
    dir_node.appendEntry(entry);
    
    unsigned char addr[2];
    shortToByte(dir_node_num, addr);
    root_node.appendAddress(addr);
}

bool FileSystem::changeDir(unsigned char* des) {
    unsigned short dir_backup = dir;
    Path path(des);
    if(!path.isRelative()) {
        dir = manager.getRootBlock();
        if(path.isRoot()) {
            return true;
        }
    }
    unsigned char path_name[62];
    unsigned char name[62];
    unsigned char temp[2];
    bool match = false;
    bool match_whole = true;
    for (auto path_iter = path.begin(); path_iter < path.end(); path_iter = path.next(path_iter)) {
        path.getPath(path_iter, path_name);
        Inode curr_node = Inode(disk[dir], manager);
        for (auto node_iter = curr_node.begin(); node_iter < curr_node.end(); node_iter = curr_node.next(node_iter)) {
            curr_node.getAddress(node_iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)]);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, name);
                if(strcmp((char*)path_name, (char*)name) == 0) {
                    match = true;
                    curr_dir.getNum(dir_iter, temp);
                    dir = byteToShort(temp);
                    break;
                }
            }
            if(match) {
                break;
            }
        }

        if(match) {
            match = false;
        }
        else {
            match_whole = false;
            break;
        }
    }

    if(match_whole) {
        Inode curr_node = Inode(disk[dir], manager);
        if(!curr_node.isDir()) {
            std::cout << "destination is not a folder" <<std::endl;
            dir = dir_backup;
            return false;
        }
        else {
            return true;
        }
    }
    else {
        std::cout << "Can not find path" << std::endl;
        dir = dir_backup;
        return false;
    }
}

void FileSystem::createDir(unsigned char* des) {
    unsigned short dir_backup = dir;
    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Directory / already exists"<<std::endl;
        return;
    }

    unsigned char path[2048];
    unsigned char name[62];
    unsigned char entry_name[62];
    syspath.separate(path, name);

    if(!changeDir(path)) {
        return;
    }
    else {
        Inode dir_node = Inode(disk[dir], manager);
        // 查找是否存在重名文件夹
        unsigned char temp[2];
        for (auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)]);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, entry_name);
                if(strcmp((char*)name, (char*)entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(curr_node.isDir()) {
                        std::cout<<"folder already exists"<<std::endl;
                        dir = dir_backup;
                        return;
                    }
                }
            }
        }
        // 确认无重名文件夹后插入
        unsigned char entry[64];
        for(auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)]);
            if(curr_dir.getRecord() >= 15) {
                continue;
            }
            else {
                unsigned short node_num = manager.allocate();
                Inode new_node = Inode(disk[node_num], manager);
                new_node.initialize();
                new_node.setName(name);
                new_node.setIsDir(true);
                shortToByte(node_num, temp);
                entry[0] = temp[0];
                entry[1] = temp[1];
                size_t i = 0;
                for(i; i < strlen((char*)name); i++) {
                    entry[2+i] = name[i];
                }
                entry[2+i] = '\0';
                curr_dir.appendEntry(entry);

                unsigned short new_dir_num = manager.allocate();
                Directory new_dir = Directory(disk[new_dir_num]);
                shortToByte(new_dir_num, temp);
                new_node.appendAddress(temp);

                shortToByte(node_num, temp);
                entry[0] = temp[0];
                entry[1] = temp[1];
                entry[2] = '.';
                entry[3] = '\0';
                new_dir.appendEntry(entry);

                shortToByte(dir, temp);
                entry[0] = temp[0];
                entry[1] = temp[1];
                entry[2] = '.';
                entry[3] = '.';
                entry[4] = '\0'; 
                new_dir.appendEntry(entry);

                dir = dir_backup;
                return;
            }
        }
        // 当前已分配directory node全部装满, 需要新分配一个

        if(dir_node.getTotalRecord() >= 520) {
            std::cout<<"destination folder is full"<<std::endl;
            dir = dir_backup;
            return;
        }
        else {
            unsigned short dir_node_dir_num = manager.allocate();
            Directory dir_node_dir = Directory(disk[dir_node_dir_num]);
            dir_node_dir.initialize();
            shortToByte(dir_node_dir_num, temp);
            dir_node.appendAddress(temp);

            unsigned short node_num = manager.allocate();
            Inode new_node = Inode(disk[node_num], manager);
            new_node.initialize();
            new_node.setName(name);
            new_node.setIsDir(true);
            shortToByte(node_num, temp);
            entry[0] = temp[0];
            entry[1] = temp[1];
            size_t i = 0; 
            for(i; i < strlen((char*)name); i++) {
                entry[2+i] = name[i];
            }
            entry[2+i] = '\0';
            dir_node_dir.appendEntry(entry);

            unsigned short new_dir_num = manager.allocate();
            Directory new_dir = Directory(disk[new_dir_num]);

            shortToByte(node_num, temp);
            entry[0] = temp[0];
            entry[1] = temp[1];
            entry[2] = '.';
            entry[3] = '\0';
            new_dir.appendEntry(entry);

            shortToByte(dir, temp);
            entry[0] = temp[0];
            entry[1] = temp[1];
            entry[2] = '.';
            entry[3] = '.';
            entry[4] = '\0'; 
            new_dir.appendEntry(entry);

            dir = dir_backup;
        }
    }
}

void FileSystem::listDir() {
    Inode node = Inode(disk[dir], manager);
    unsigned char temp[2];
    unsigned char name[62];
    for (auto iter = node.begin(); iter < node.end(); iter = node.next(iter)) {
        node.getAddress(iter, temp);
        Directory directory = Directory(disk[byteToShort(temp)]);
        for (auto dir_iter = directory.begin(); dir_iter < directory.end(); dir_iter = directory.next(dir_iter)) {
            directory.getName(dir_iter, name);
            std::cout<<name<<"  ";
        }
    }
    std::cout<<std::endl;
}

void FileSystem::getPath(unsigned char* path) {
    unsigned short chain[32];
    unsigned short curr = dir;
    unsigned char temp[2];
    unsigned char name_temp[62];
    int count = 0;
    while(true) {
        if (curr == 0) {
            break;
        }
        chain[count] = curr; 
        Inode curr_node = Inode(disk[curr], manager);
        auto iter = curr_node.begin();
        curr_node.getAddress(iter, temp);
        Directory curr_dir = Directory(disk[byteToShort(temp)]);
        iter = curr_dir.begin();
        iter = curr_dir.next(iter);
        curr_dir.getNum(iter, temp);
        curr = byteToShort(temp);
        count++;
    }

    if (count == 0) {
        path[0] = '/';
        path[1] = '\0';
    }
    else {
        int char_count = 0;
        for (int i = count - 1; i >= 0; i--) {
            path[char_count] = '/';
            char_count++;
            Inode curr_node1 = Inode(disk[chain[i]], manager);
            curr_node1.getName(name_temp);
            for (int j = 0; j < strlen((char*)name_temp); j++) {
                path[char_count] = name_temp[j];
                char_count++;
            }
            // if (i != 0) {
            //     path[char_count] = '/';
            //     char_count++;            
            // }
        }
        path[char_count] = '\0';
    }
}