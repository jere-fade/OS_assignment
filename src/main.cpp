#include <iostream>
#include <fstream>
#include <string.h>
#include <ctime>
#include <time.h>
#include <stdlib.h>
#include <exception>
#include <iomanip>
#include "BlockManager.h"
#include "inode.h"
#include "directory.h"
#include "constant.h"
#include "utils.h"
#include "FileSystem.h"
#include "path.h"

int main(int, char**) {

    system("clear");

    char default_color[] = "\033[0m";
    char blue_b[] = "\033[104m";
    char blue_f[] = "\033[94m";
    char white_f[] = "\033[97m";
    char cyan[] = "\033[96m";
    char light_cyan[] = "\033[36m";

    char black_folder[5];
    black_folder[0] = 0xF0;
    black_folder[1] = 0x9F;
    black_folder[2] = 0x96;
    black_folder[3] = 0xBF;
    black_folder[4] = '\0';

    char folder[5];
    folder[0] = 0xF0;
    folder[1] = 0x9F;
    folder[2] = 0x93;
    folder[3] = 0x82;
    folder[4] = '\0';

    srand(time(NULL));

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
    bool initialize = false;
    if (initialize) {
        manager.format();
        manager.buildVolume();
    }
    // std::cout<<"list head: "<< manager.getHead() <<std::endl;
    // std::cout<<"list offset: "<< manager.getOffset() <<std::endl;
    // std::cout<<"list size: "<< manager.listSize() << std::endl;

    FileSystem fs = FileSystem(disk);
    if (initialize) {
        fs.initialize();
    }

    FILE* fp = popen("tput cols", "r");
    char result[10];
    fgets(result, sizeof(result), fp);
    int len = std::stoi(result);

    while(true) {

        char command[4096];
        char path[2048];
        int argc = 0;
        char* argv[3];

        double percent = (double) (BLOCK_NUM - manager.listSize()) / BLOCK_NUM * 100;

        fs.getPath(path);
        // std::cout<<"\r"<<std::setw(len-1)<<std::setprecision(2)<<std::fixed<<percent<<"%";
        printf("\r%s%*s  %.2f%c%s", cyan, len-6, black_folder, percent, '%', default_color);
        printf("\r%s%s %s %c%s%s %s\n", blue_b, white_f, folder, '[' ,path, "] >", default_color);
        printf("%s\u2570\u2500\ue285\ufb00%s ", light_cyan, default_color);

        if(fgets(command, 4096, stdin) != NULL) {
            argv[argc] = strtok(command, " \n\t");
            argc+=1;
            while(argv[argc-1] != NULL) {
                argv[argc] = strtok(NULL, " \n\t");
                argc+=1;
            }
            argc-=1;
        }

        if(argc == 0) {
            std::cout<<"input can not be empty"<<std::endl;
        }
        else if(strcmp(argv[0], "cd") == 0) {
            if(argc != 2) {
                std::cout<<"changeDir: arguments number is wrong"<<std::endl;
            }
            else {
                if(strlen(argv[1]) >= 2048){
                    std::cout<<"path length exceed limit"<<std::endl;
                }
                else {
                    if(!fs.changeDir(argv[1])) {
                        std::cout<<"Can not find path"<<std::endl;
                    }
                }
            }
        }
        else if(strcmp(argv[0], "mkdir") == 0) {
            if(argc != 2) {
                std::cout<<"createDir: arguments number is wrong"<<std::endl;
            }
            else {
                if(strlen(argv[1]) >= 2048) {
                    std::cout<<"path length exceed limit"<<std::endl;
                }
                else {
                    fs.createDir(argv[1]);
                }
            }
        }
        else if(strcmp(argv[0], "createFile") == 0) {
            if(argc != 3) {
                std::cout<<"createFile: arguments number is wrong"<<std::endl;
            }
            else {
                if(strlen(argv[1]) >= 2048) {
                    std::cout<<"path length exceed limit"<<std::endl;
                }
                else {
                    try{
                        unsigned short filesize = std::stoi(argv[2]);
                        fs.createFile(argv[1], filesize);
                    }
                    catch(std::exception& e) {
                        std::cout<<"createFile: file size must be a number"<<std::endl;
                    }
                }
            }
        }
        else if(strcmp(argv[0], "rm") == 0) {
            if(argc == 2) {
                if(strlen(argv[1]) >= 2048) {
                    std::cout<<"path length exceed limit"<<std::endl;
                }
                else {
                    fs.deleteFile(argv[1]);
                }
            }
            else if(argc == 3) {
                if(strcmp(argv[1], "-r") == 0) {
                    if(strlen(argv[2]) >= 2048) {
                        std::cout<<"path length exceed limit"<<std::endl;
                    }
                    else {
                        fs.deleteDir(argv[2]);
                    }                   
                }
                else {
                    std::cout<<"deleteDir: unknown command"<<std::endl;
                }
            }
            else {
                std::cout<<"deleteDir: arguments number is wrong"<<std::endl;
            }
        }
        else if(strcmp(argv[0], "cp") == 0) {
            if(argc != 3) {
                std::cout<<"cp: arguments number is wrong"<<std::endl;
            }
            else {
                if(strlen(argv[1]) >= 2048 || strlen(argv[2]) >= 2048) {
                    std::cout<<"path length exceed limit"<<std::endl;
                }
                else {
                    fs.copyFile(argv[1], argv[2]);
                }
            }
        }
        else if(strcmp(argv[0], "ls") == 0) {
            if(argc == 1) {
                fs.listDir(nullptr);
            }
            else if(argc == 2) {
                fs.listDir(argv[1]);
            }
            else {
                std::cout<<"ls: arguments number is wrong"<<std::endl;
            }
        }
        else if(strcmp(argv[0], "cat") == 0) {
            if(argc != 2) {
                std::cout<<"arguments number is wrong"<<std::endl;
            }
            else {
                fs.concatenate(argv[1]);
            }
        }
        else if(strcmp(argv[0], "sum") == 0) {
            if(argc != 1) {
                std::cout<<"arguments number is wrong"<<std::endl;
            }
            else {
                fs.usage();
            }
        }
        else if(strcmp(argv[0], "exit") == 0) {
            if(argc != 1) {
                std::cout<<"arguments number is wrong"<<std::endl;
            }
            else {
                break;
            }
        }
        else {
            std::cout<<"unknown command"<<std::endl;
        }
        std::cout<<std::endl;
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

