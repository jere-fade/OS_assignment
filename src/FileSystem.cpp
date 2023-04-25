#include "FileSystem.h"
#include "inode.h"
#include "directory.h"
#include "utils.h"
#include "path.h"
#include "constant.h"
#include "string.h"
#include <iostream>
#include <stdlib.h>
#include <ctime>

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
        for (auto node_iter = curr_node.begin(); node_iter < curr_node.end(); node_iter = curr_node.next(node_iter)) {
            curr_node.getAddress(node_iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)name);
                if(strcmp(path_name, name) == 0) {
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
        return;
    }
    else {
        Inode dir_node = Inode(disk[dir], manager);
        // 查找是否存在重名文件夹
        unsigned char temp[2];
        for (auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(curr_node.isDir()) {
                        std::cout<<"Can not create folder: folder already exists"<<std::endl;
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
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
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
        return;
    }
    else {
        bool find = false;
        unsigned short aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for (auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
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
                        std::cout<<"Can not delete folder: "<<name<<" is a file"<<std::endl;
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
            unsigned short curr_dir_num = byteToShort(temp);
            Directory curr_dir = Directory(disk[curr_dir_num], manager);
            
            curr_dir.getNum(aim_dir_iter, temp);
            unsigned short aim_node = byteToShort(temp);
            if(dir_backup == aim_node) {
                std::cout<<"Can not delete folder: you can not delete the working directory"<<std::endl;
                dir = dir_backup;
                return;
            }

            unsigned short last_iter = dir_node.begin();
            for (last_iter; dir_node.next(last_iter) < dir_node.end(); last_iter = dir_node.next(last_iter)) {}
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
        return;
    }
    else {
        Inode dir_node = Inode(disk[dir], manager);

        unsigned char temp[2];
        for (auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
            for (auto dir_iter = curr_dir.begin(); dir_iter < curr_dir.end(); dir_iter = curr_dir.next(dir_iter)) {
                curr_dir.getName(dir_iter, (unsigned char*)entry_name);
                if(strcmp(name, entry_name) == 0) {
                    curr_dir.getNum(dir_iter, temp);
                    Inode curr_node = Inode(disk[byteToShort(temp)], manager);
                    if(!curr_node.isDir()) {
                        std::cout<<"Can not create file: file already exists"<<std::endl;
                        dir = dir_backup;
                        return;
                    }
                }
            }
        }

        unsigned char entry[64];
        for (auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
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
        return;
    }
    else {
        bool find = false;
        unsigned short aim_iter;
        unsigned short aim_dir_iter;
        unsigned char temp[2];
        Inode dir_node = Inode(disk[dir], manager);

        for(auto iter = dir_node.begin(); iter < dir_node.end(); iter = dir_node.next(iter)) {
            dir_node.getAddress(iter, temp);
            Directory curr_dir = Directory(disk[byteToShort(temp)], manager);
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
                        std::cout<<"Can not delete file: "<<name<<" is a folder"<<std::endl;
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
            unsigned short curr_dir_num = byteToShort(temp);
            Directory curr_dir = Directory(disk[curr_dir_num], manager);

            curr_dir.getNum(aim_dir_iter, temp);
            unsigned short aim_node = byteToShort(temp);

            unsigned short last_iter = dir_node.begin();
            for(last_iter; dir_node.next(last_iter) < dir_node.end(); last_iter = dir_node.next(last_iter)) {}
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
            std::cout<<"Can not delete file: file not exists"<<std::endl;
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
    for (auto iter = node.begin(); iter < node.end(); iter = node.next(iter)) {
        node.getAddress(iter, temp);
        Directory directory = Directory(disk[byteToShort(temp)], manager);
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
                        printf("%6d %s %s%-62s%s\n", size, t, dark_blue, name, default_color);
                    }
                }
                else {
                    if(curr_node.isDir()) {
                        printf("%3dK %s %s%-62s%s\n", size, t, dark_blue, name, default_color);
                    }
                    else {
                        printf("%3dK %s %s%-62s%s\n", size, t, dark_blue, name, default_color);
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

void FileSystem::fillFile(unsigned short block_num) {
    int randchar;
    unsigned char* block = disk[block_num];
    for(int i = 0; i < BLOCK_SIZE; i++) {
        randchar = rand() % 94;
        block[i] = '!' + randchar;
    }
    block[BLOCK_SIZE - 1] = '\0';
}