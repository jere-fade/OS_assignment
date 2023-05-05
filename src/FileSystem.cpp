#include "FileSystem.h"
#include "inode.h"
#include "directory.h"
#include "utils.h"
#include "path.h"
#include "constant.h"
#include "string.h"
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <chrono>
#include <thread>

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
    Directory dir_node = Directory(disk[dir_node_num], manager);
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

bool FileSystem::changeDir(char* des) {
    unsigned short dir_backup = dir;
    Path path(des);
    if(!path.isRelative()) {
        dir = manager.getRootBlock();
        if(path.isRoot()) {
            return true;
        }
    }
    char path_name[62];
    char name[62];
    unsigned char temp[2];
    bool match = false;
    bool match_whole = true;
    for (auto path_iter = path.begin(); path_iter < path.end(); path_iter = path.next(path_iter)) {
        path.getPath(path_iter, path_name);
        Inode curr_node = Inode(disk[dir], manager);
        for (auto node_iter = curr_node.begin(); node_iter != curr_node.end(); node_iter++) {
            Directory curr_dir = Directory(disk[node_iter.value(&curr_node)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)name);
                if(strcmp(path_name, name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    unsigned short node_num = byteToShort(temp);
                    Inode next_node = Inode(disk[node_num], manager);
                    if(next_node.isDir()) {
                        match = true;
                        dir = node_num;
                        break;
                    }
                    else {
                        continue;
                    }
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
        // Inode curr_node = Inode(disk[dir], manager);
        // if(!curr_node.isDir()) {
        //     std::cout << "destination is not a folder" <<std::endl;
        //     dir = dir_backup;
        //     return false;
        // }
        // else {
            return true;
        // }
    }
    else {
        // std::cout << "Can not find path" << std::endl;
        dir = dir_backup;
        return false;
    }
}

void FileSystem::createDir(char* des) {
    unsigned short dir_backup = dir;
    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Directory / already exists"<<std::endl;
        return;
    }

    char path[2048];
    char name[62];
    char entry_name[62];
    syspath.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not create folder: . or .. exists"<<std::endl;
        return; 
    }

    if(!changeDir(path)) {
        std::cout<<"Can not fine path"<<std::endl;
        return;
    }
    else {
        Inode dir_node = Inode(disk[dir], manager);
        // 查找是否存在重名文件夹
        unsigned char temp[2];
        for (auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    std::cout<<"Can not create folder: folder/file having the same name already exists"<<std::endl;
                    dir = dir_backup;
                    return;
                    // curr_dir.getNum(dir_iter, temp);
                    // Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    // if(curr_node.isDir()) {
                    //     std::cout<<"Can not create folder: folder already exists"<<std::endl;
                    //     dir = dir_backup;
                    //     return;
                    // }
                }
            }
        }
        // 确认无重名文件夹后插入
        unsigned char entry[64];
        for(auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            if(curr_dir.getRecord() >= 15) {
                continue;
            }
            else {
                unsigned short node_num = manager.allocate();
                Inode new_node = Inode(disk[node_num], manager);
                new_node.initialize();
                new_node.setName((unsigned char*)name);
                new_node.setIsDir(true);
                shortToByte(node_num, temp);
                entry[0] = temp[0];
                entry[1] = temp[1];
                size_t i = 0;
                for(i; i < strlen(name); i++) {
                    entry[2+i] = name[i];
                }
                entry[2+i] = '\0';
                curr_dir.appendEntry(entry);

                unsigned short new_dir_num = manager.allocate();
                Directory new_dir = Directory(disk[new_dir_num], manager);
                new_dir.initialize();
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
            std::cout<<"Can not create folder: destination folder is full"<<std::endl;
            dir = dir_backup;
            return;
        }
        else {
            unsigned short dir_node_dir_num = manager.allocate();
            Directory dir_node_dir = Directory(disk[dir_node_dir_num], manager);
            dir_node_dir.initialize();
            shortToByte(dir_node_dir_num, temp);
            dir_node.appendAddress(temp);

            unsigned short node_num = manager.allocate();
            Inode new_node = Inode(disk[node_num], manager);
            new_node.initialize();
            new_node.setName((unsigned char*)name);
            new_node.setIsDir(true);
            shortToByte(node_num, temp);
            entry[0] = temp[0];
            entry[1] = temp[1];
            size_t i = 0; 
            for(i; i < strlen(name); i++) {
                entry[2+i] = name[i];
            }
            entry[2+i] = '\0';
            dir_node_dir.appendEntry(entry);

            unsigned short new_dir_num = manager.allocate();
            Directory new_dir = Directory(disk[new_dir_num], manager);
            new_dir.initialize();
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
        }
    }
}

void FileSystem::deleteDir(char* des) {
    // change path 会检查路径是否存在
    // change 成功后还需要检查是否是当前working directory
    // 然后检查要删除的项目在当前文件夹是否存在
    // 找到对应的directory并从中删除entry
    // 如果这个directory node空了, 就从inode中删除 (指定iter删除)

    unsigned short dir_backup = dir;
    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Can not delete folder: Permission Denied, You can not delete the root directory"<<std::endl;
        return;
    }
    char path[2048];
    char name[62];
    char entry_name[62];
    syspath.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not delete folder: refuse to remove . or .."<<std::endl;
        return; 
    }

    if(!changeDir(path)) {
        std::cout<<"Can not fine path"<<std::endl;
        return;
    }
    else {
        bool find = false;
        Inode::iterator aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for (auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(curr_node.isDir()) {
                        find = true;
                        aim_iter = iter;
                        aim_dir_iter = dir_iter;
                        break;
                    }
                    else {
                        deleteFile(des);
                        dir = dir_backup;
                        return;
                    }
                }
            }
            if(find) {
                break;
            }
        }

        if(find) {
            // dir_node.getAddress(aim_iter, temp);
            unsigned short curr_dir_num = aim_iter.value(&dir_node);
            Directory curr_dir = Directory(disk[curr_dir_num], manager);
            
            curr_dir.getNum(aim_dir_iter, temp);
            unsigned short aim_node = byteToShort(temp);
            if(dir_backup == aim_node) {
                std::cout<<"Can not delete folder: you can not delete the working directory"<<std::endl;
                dir = dir_backup;
                return;
            }

            Inode::iterator last_iter = dir_node.begin();
            for (last_iter; dir_node.next(last_iter) != dir_node.end(); last_iter++) {}
            dir_node.getAddress(last_iter, temp);
            unsigned short last_dir_num = byteToShort(temp);
            Directory last_dir = Directory(disk[last_dir_num], manager);
            unsigned short last_dir_iter = last_dir.begin();
            for(last_dir_iter; last_dir.next(last_dir_iter) < last_dir.end(); last_dir_iter = last_dir.next(last_dir_iter)) {}

            if((last_iter == aim_iter) && (last_dir_iter == aim_dir_iter)) {
                curr_dir.deleteEntry(aim_dir_iter);
                if(curr_dir.getRecord() == 0) {
                    dir_node.deleteAddress(aim_iter);
                }
            }
            else {
                unsigned char entry[64];
                last_dir.getEntry(last_dir_iter, entry);
                last_dir.removeEntry(last_dir_iter);
                if(last_dir.getRecord() == 0) {
                    dir_node.deleteAddress(last_iter);
                }
                curr_dir.deleteEntry(aim_dir_iter);
                curr_dir.appendEntry(entry);
            }
            
            dir = dir_backup;
        }
        else {
            std::cout<<"Can not delete folder: folder not exists"<<std::endl;
            dir = dir_backup;
            return;
        }
    }
}

void FileSystem::createFile(char* des, unsigned short size) {
    unsigned short dir_backup = dir;
    if(size > 520) {
        std::cout<<"Can not create file: file size exceeds maximum"<<std::endl;
        return;
    }

    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Can not create file: file name empty"<<std::endl;
        return;
    }

    char path[2048];
    char name[62];
    char entry_name[62];
    syspath.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not create file: . or .. exists"<<std::endl;
        return; 
    }

    if(!changeDir(path)) {
        std::cout<<"Can not find path"<<std::endl;
        return;
    }
    else {
        Inode dir_node = Inode(disk[dir], manager);

        unsigned char temp[2];
        for (auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    std::cout<<"Can not create file: folder/file having the same name already exists"<<std::endl;
                    dir = dir_backup;
                    return;
                    // curr_dir.getNum(dir_iter, temp);
                    // Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    // if(!curr_node.isDir()) {
                    //     std::cout<<"Can not create file: file already exists"<<std::endl;
                    //     dir = dir_backup;
                    //     return;
                    // }
                }
            }
        }

        unsigned char entry[64];
        for (auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            if(curr_dir.getRecord() >= 15) {
                continue;
            }
            else {
                unsigned short node_num = manager.allocate();
                Inode new_node = Inode(disk[node_num], manager);
                new_node.initialize();
                new_node.setName((unsigned char*)name);
                new_node.setIsDir(false);
                shortToByte(node_num, temp);
                entry[0] = temp[0];
                entry[1] = temp[1];
                size_t i = 0;
                for(i; i < strlen(name); i++) {
                    entry[2+i] = name[i];
                }
                entry[2+i] = '\0';
                curr_dir.appendEntry(entry);

                for (unsigned short block = 0; block < size; block++) {
                    unsigned short block_num = manager.allocate();
                    shortToByte(block_num, temp);
                    fillFile(block_num);
                    new_node.appendAddress(temp);
                }

                dir = dir_backup;
                return;
            }
        }

        if(dir_node.getTotalRecord() >= 520) {
            std::cout<<"Can not create file: destination folder is full"<<std::endl;
            dir = dir_backup;
            return;
        }
        else {
            unsigned short dir_node_dir_num = manager.allocate();
            Directory dir_node_dir = Directory(disk[dir_node_dir_num], manager);
            dir_node_dir.initialize();
            shortToByte(dir_node_dir_num, temp);
            dir_node.appendAddress(temp);

            unsigned short node_num = manager.allocate();
            Inode new_node = Inode(disk[node_num], manager);
            new_node.initialize();
            new_node.setName((unsigned char*)name);
            new_node.setIsDir(false);
            shortToByte(node_num, temp);
            entry[0] = temp[0];
            entry[1] = temp[1];
            size_t i = 0;
            for(i; i < strlen(name); i++) {
                entry[2+i] = name[i];
            }
            entry[2+i] = '\0';
            dir_node_dir.appendEntry(entry);

            for (unsigned short block = 0; block < size; block++) {
                unsigned short block_num = manager.allocate();
                shortToByte(block_num, temp);
                fillFile(block_num);
                new_node.appendAddress(temp);
            }

            dir = dir_backup;
            return;
        }
    }
}

void FileSystem::deleteFile(char* des) {
    unsigned short dir_backup = dir;
    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Can not delete file: file name emtpy"<<std::endl;
        return;
    }

    char path[2048];
    char name[62];
    char entry_name[62];
    syspath.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not delete file: . or .. are directories";
        return;
    }

    if(!changeDir(path)) {
        std::cout<<"Can not find path"<<std::endl;
        return;
    }
    else {
        bool find = false;
        bool findFolder = false;
        Inode::iterator aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for(auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for(auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(!curr_node.isDir()) {
                        find = true;
                        aim_iter = iter;
                        aim_dir_iter = dir_iter;
                        break;
                    }
                    else {
                        findFolder = true;
                        continue;
                        // std::cout<<"Can not delete file: "<<name<<" is a folder"<<std::endl;
                        // dir = dir_backup;
                        // return;
                    }
                }
            }
            if(find) {
                break;
            }
        }

        if(find) {
            dir_node.getAddress(aim_iter, temp);
            unsigned short curr_dir_num = byteToShort(temp);
            Directory curr_dir = Directory(disk[curr_dir_num], manager);

            curr_dir.getNum(aim_dir_iter, temp);
            // unsigned short aim_node = byteToShort(temp);

            Inode::iterator last_iter = dir_node.begin();
            for(last_iter; dir_node.next(last_iter) != dir_node.end(); last_iter++) {}
            dir_node.getAddress(last_iter, temp);
            unsigned short last_dir_num = byteToShort(temp);
            Directory last_dir = Directory(disk[last_dir_num], manager);
            unsigned short last_dir_iter = last_dir.begin();
            for(last_dir_iter; last_dir.next(last_dir_iter) < last_dir.end(); last_dir_iter = last_dir.next(last_dir_iter)) {}

            if((last_iter == aim_iter) && (last_dir_iter == aim_dir_iter)) {
                curr_dir.deleteEntry(aim_dir_iter);
                if(curr_dir.getRecord() == 0) {
                    dir_node.deleteAddress(aim_iter);
                }
            }
            else {
                unsigned char entry[64];
                last_dir.getEntry(last_dir_iter, entry);
                last_dir.removeEntry(last_dir_iter);
                if(last_dir.getRecord() == 0) {
                    dir_node.deleteAddress(last_iter);
                }
                curr_dir.deleteEntry(aim_dir_iter);
                curr_dir.appendEntry(entry);
            }

            dir = dir_backup;
        }
        else if(findFolder) {
            std::cout<<"Can not delete file: "<<name<<" is a folder"<<std::endl;
            dir = dir_backup;
            return;
        }
        else {
            std::cout<<"Can not delete file: file not exists"<<std::endl;
            dir = dir_backup;
            return;
        }

    }

}

void FileSystem::concatenate(char* des) {
    unsigned short dir_backup = dir;
    
    Path syspath(des);
    if(syspath.isRoot()) {
        std::cout<<"Can not cat: file name empty"<<std::endl;
        return;
    }
    
    char path[2048];
    char name[62];
    char entry_name[62];
    syspath.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not cat: can not cat . or .."<<std::endl;
        return;
    }

    if(!changeDir(path)) {
        std::cout<<"Can not find path"<<std::endl;
        return;
    }
    else {
        bool find = false;
        Inode::iterator aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for(auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for(auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(!curr_node.isDir()) {
                        find = true;
                        aim_iter = iter;
                        aim_dir_iter = dir_iter;
                        break;
                    }
                    else {
                        std::cout<<"Can not cat: "<<name<<" is a folder"<<std::endl;
                        dir = dir_backup;
                        return;
                    }
                }
            } 
            if(find) {
                break;
            }
        }

        if(find) {
            dir_node.getAddress(aim_iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
            curr_dir.getNum(aim_dir_iter, temp);
            Inode aim_node = Inode(disk[byteToShort(temp)], manager);

            for(auto iter = aim_node.begin(); iter != aim_node.end(); iter++) {
                unsigned char* curr_block = disk[iter.value(&aim_node)];
                std::cout<<curr_block;
            }
            std::cout<<std::endl;
            dir = dir_backup;

        }
        else {
            std::cout<<"Can not cat: file not exists"<<std::endl;
            dir = dir_backup;
            return;
        }
    }

}

void FileSystem::copyFile(char* src, char* des) {
    // 1. change path to src, get the inode
    // 检查是否存在
    // 2. change path to des, create file
    // 检查重复文件

    unsigned short dir_backup = dir;

    Path Src(src);
    Path Des(des);

    if(Src.isRoot()) {
        std::cout<<"Can not copy: file name emtpy"<<std::endl;
        return;
    }

    char path[2048];
    char name[62];
    char entry_name[62];
    Src.separate(path, name);

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        std::cout<<"Can not copy: . or .. are not files"<<std::endl;
        return;
    }

    if(!changeDir(path)) {
        std::cout<<"Can not find path"<<std::endl;
        return;
    }
    else {
        bool find = false;
        bool findFolder = false;
        Inode::iterator aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for(auto iter = dir_node.begin(); iter != dir_node.end(); iter++) {
            Directory curr_dir = Directory(disk[iter.value(&dir_node)], manager);
            for(auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(!curr_node.isDir()) {
                        find = true;
                        aim_iter = iter;
                        aim_dir_iter = dir_iter;
                        break;
                    }
                    else {
                        findFolder = true;
                        continue;
                    }
                }
            }
            if(find) {
                break;
            }
        }

        if(find) {
            dir = dir_backup;
            dir_node.getAddress(aim_iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager); 
            curr_dir.getNum(aim_dir_iter, temp);
            Inode src_node = Inode(disk[byteToShort(temp)], manager);

            for(int i = 0; i < strlen(path); i++) {
                path[i] = '\0';
            }
            for(int i = 0; i < strlen(name); i++) {
                name[i] = '\0';
            }
            for(int i = 0; i < strlen(entry_name); i++) {
                entry_name[i] = '\0';
            }

            bool changeName = true;
            bool pathExist = true;

            if(changeDir(des)) {
                changeName = false;  
                src_node.getName((unsigned char*)name);
            }
            else {
                Des.separate(path, name);
                if(!changeDir(path)) {
                    pathExist = false;
                }
            }


            // if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            //     std::cout<<"Can not copy: destination file name can not be . or .."<<std::endl;
            //     dir = dir_backup;
            //     return;
            // }

            if(!pathExist) {
                std::cout<<"Can not find path"<<std::endl;
                return;
            }
            else {
                Inode des_dir_node = Inode(disk[dir], manager);
                for(auto iter = des_dir_node.begin(); iter != des_dir_node.end(); iter++) {
                    Directory des_curr_dir = Directory(disk[iter.value(&des_dir_node)], manager);
                    for(auto dir_iter = des_curr_dir.begin(); dir_iter < des_curr_dir.end(); dir_iter = des_curr_dir.next(dir_iter)) {
                        des_curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                        if(strcmp(name, entry_name) == 0) {
                            std::cout<<"Can not copy: folder/file having the same name already exists"<<std::endl;
                            dir = dir_backup;
                            return;
                        }
                    }
                }
                unsigned char entry[64];
                for(auto iter = des_dir_node.begin(); iter != des_dir_node.end(); iter++) {
                    Directory des_curr_dir = Directory(disk[iter.value(&des_dir_node)], manager);
                    if(des_curr_dir.getRecord() >= 15) {
                        continue;
                    }
                    else {
                        unsigned short des_node_num = manager.allocate();
                        Inode des_node = Inode(disk[des_node_num], manager);
                        des_node.initialize();
                        des_node.setName((unsigned char*)name);
                        des_node.setIsDir(false);
                        shortToByte(des_node_num, temp);
                        entry[0] = temp[0];
                        entry[1] = temp[1];
                        size_t i = 0; 
                        for(i; i < strlen(name); i++) {
                            entry[2+i] = name[i];
                        }
                        entry[2+i] = '\0';
                        des_curr_dir.appendEntry(entry);

                        for(auto node_iter = src_node.begin(); node_iter != src_node.end(); node_iter++) {
                            unsigned short block_num = manager.allocate();
                            disk[block_num] = disk[node_iter.value(&src_node)];
                            shortToByte(block_num, temp);
                            des_node.appendAddress(temp);
                        }

                        dir = dir_backup;
                        return;
                    }
                }

                if(des_dir_node.getTotalRecord() >= 520) {
                    std::cout<<"Can not copy: destination folder is full"<<std::endl;
                    dir = dir_backup;
                    return;
                }
                else {
                    unsigned dir_node_dir_num = manager.allocate();
                    Directory dir_node_dir = Directory(disk[dir_node_dir_num], manager);
                    dir_node_dir.initialize();
                    shortToByte(dir_node_dir_num, temp);
                    des_dir_node.appendAddress(temp);

                    unsigned short des_node_num = manager.allocate();
                    Inode des_node = Inode(disk[des_node_num], manager);
                    des_node.initialize();
                    des_node.setName((unsigned char*)name);
                    des_node.setIsDir(false);
                    shortToByte(des_node_num, temp);
                    entry[0] = temp[0];
                    entry[1] = temp[1];
                    size_t i = 0;
                    for(i; i < strlen(name); i++) {
                        entry[2+i] = name[i];
                    }
                    entry[2+i] = '\0';
                    dir_node_dir.appendEntry(entry);

                    for(auto node_iter = src_node.begin(); node_iter != src_node.end(); node_iter++) {
                        unsigned short block_num = manager.allocate();
                        disk[block_num] = disk[node_iter.value(&src_node)];
                        shortToByte(block_num, temp);
                        des_node.appendAddress(temp);
                    }
                    dir = dir_backup;
                    return;
                }
            }

        }
        else if(findFolder) {
            std::cout<<"Can not copy: "<<name<<" is a folder"<<std::endl;
            dir = dir_backup;
            return;
        }
        else {
            std::cout<<"Can not copy: source file not exists"<<std::endl;
            dir = dir_backup;
            return;
        }
    }
    
}


void FileSystem::listDir(char* parameter) {
    char dark_blue[] = "\033[34m";
    char dark_green[] = "\033[32m";
    char default_color[] = "\033[0m";
    Inode node = Inode(disk[dir], manager);
    unsigned char temp[2];
    char name[62];
    for (auto iter = node.begin(); iter != node.end(); iter++) {
        Directory directory = Directory(disk[iter.value(&node)], manager);
        for (auto dir_iter = directory.begin(); dir_iter < directory.end(); dir_iter = directory.next(dir_iter)) {
            directory.getNum(dir_iter, temp);
            Inode curr_node = Inode(disk[byteToShort(temp)], manager);
            directory.getName(dir_iter, (unsigned char*)name);
            if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            } 
            if(parameter == nullptr) {
                if(curr_node.isDir()) {
                    printf("%s%s%s  ", dark_blue, name, default_color);
                }
                else {
                    printf("%s%s%s  ", dark_green, name, default_color);
                }
            }
            else if(strcmp(parameter, "-l") == 0 || strcmp(parameter, "-lh") == 0) {
                std::time_t time = curr_node.getCtime();
                char* t = ctime(&time);
                t[strlen(t) - 1] = '\0';
                int size = curr_node.getTotalRecord();
                if(strcmp(parameter, "-l") == 0) {
                    size *= 1024;
                    if(curr_node.isDir()) {
                        printf("%6d %s %s%-62s%s\n", size, t, dark_blue, name, default_color);
                    }
                    else {
                        printf("%6d %s %s%-62s%s\n", size, t, dark_green, name, default_color);
                    }
                }
                else {
                    if(curr_node.isDir()) {
                        printf("%3dK %s %s%-62s%s\n", size, t, dark_blue, name, default_color);
                    }
                    else {
                        printf("%3dK %s %s%-62s%s\n", size, t, dark_green, name, default_color);
                    }
                }
            }
            else {

            }
        }
        if(parameter == nullptr) {
            std::cout<<std::endl;
        }
    }
}

void FileSystem::getPath(char* path) {
    unsigned short chain[32];
    unsigned short curr = dir;
    unsigned char temp[2];
    char name_temp[62];
    int count = 0;
    while(true) {
        if (curr == 0) {
            break;
        }
        chain[count] = curr; 
        Inode curr_node = Inode(disk[curr], manager);
        auto iter = curr_node.begin();
        curr_node.getAddress(iter, temp);
        Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
        auto dir_iter = curr_dir.begin();
        dir_iter = curr_dir.next(dir_iter);
        curr_dir.getNum(dir_iter, temp);
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
            curr_node1.getName((unsigned char*)name_temp);
            for (int j = 0; j < strlen(name_temp); j++) {
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

double f(double t) {
    double pi = 3.14159;
    return sin(pi/2*t);
}

void FileSystem::usage() {
    int free = manager.listSize();
    int used = BLOCK_NUM - free;
    int steps = 800;
    double step = (double) 1 / steps;
    std::vector<double> point;
    for (int i = 1; i <= steps; i++) {
        point.push_back(f(step * i));
    }

    char blue[] = "\033[94m";
    char green[] = "\033[92m";
    char red[] = "\033[91m";
    char magenta[] = "\033[95m";
    char cyan[] = "\033[96m";
    char default_color[] = "\033[0m";
    double percent = (double) used / BLOCK_NUM;
    printf("Free Block: %s%5d%s  Used Block: %s%5d%s  Total Block: %s%5d%s\n", green, free, default_color, red, used, default_color, blue, BLOCK_NUM, default_color);
    std::string fill[] = {"▎", "▌", "▊", "█"};

    for (int j = 0; j < point.size(); j++)
    {
        int barWidth = 80;

        std::cout << "[";
        int pos = barWidth * point[j] * percent * 4;
        int floor = (pos/4)*4;
        for (int i = 0; i < barWidth * 4; ++i)
        {
            if (i < floor && i%4 == 0) {
                std::cout<<magenta;
                std::cout<<"█";
                std::cout<<default_color;
            }
            else if (i >= floor && (i == pos - 1)) {
                std::cout<<magenta;
                std::cout<<fill[i%4];
                std::cout<<default_color;
            }
            else if (i >= pos && (i%4 == 0))
                std::cout << " ";

        }
        std::cout << "] " << cyan <<std::fixed << std::setprecision(2) << std::setw(6) << point[j] * percent * 100 << " %\r";
        std::cout << default_color;
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
    std::cout<<std::endl;

}

void FileSystem::fillFile(unsigned short block_num) {
    int randchar;
    unsigned char* block = disk[block_num];
    for(int i = 0; i < BLOCK_SIZE; i++) {
        randchar = rand() % 94;
        block[i] = '!' + randchar;
    }
    block[BLOCK_SIZE - 1] = '\0';
}