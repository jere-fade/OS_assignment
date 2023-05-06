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

    bool initialize = false;
    bool loadExample = false;

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

    if (initialize && loadExample) {
        fs.createDir((char*)"/bin");
        fs.createDir((char*)"/dev");
        fs.createDir((char*)"/etc");
        fs.createDir((char*)"/lib");
        fs.createDir((char*)"/mnt");
        fs.createDir((char*)"/var");
        fs.createDir((char*)"/run");
        fs.createDir((char*)"/root");
        fs.createDir((char*)"/home");
        fs.createDir((char*)"/media");

        fs.changeDir((char*)"/etc");
        fs.createFile((char*)"aliases", 50);
        fs.createDir((char*)"anaconda");
        fs.createFile((char*)"bashrc", 40);
        fs.createDir((char*)"clash");
        fs.createFile((char*)"clash/cache.db", 20);
        fs.createFile((char*)"clash/config.yaml", 100);
        fs.createFile((char*)"clash/Country.mmdb", 200);
        fs.createDir((char*)"dnf");
        fs.createDir((char*)"frp");
        fs.createFile((char*)"frp/frpc.ini", 20);
        fs.createFile((char*)"frp/frpc_full.ini", 40);
        fs.createFile((char*)"frp/frps.ini", 30);
        fs.createFile((char*)"frp/frps_full.ini", 40);
        fs.createDir((char*)"systemd");
        fs.createFile((char*)"systemd/networkd.conf", 80);
        fs.createFile((char*)"systemd/sleep.conf", 70);
        fs.changeDir((char*)"/etc/systemd");
        fs.createDir((char*)"system");
        fs.createFile((char*)"system/clash.service", 40);
        fs.createFile((char*)"system/frpc.service", 50);
        fs.createFile((char*)"system/frps.service", 50);
        fs.createFile((char*)"system/span.nextcloud.apache.service", 30);
        fs.createFile((char*)"system/snap.nextcloud.mysql.service", 50);
        fs.createDir((char*)"texlive");
        fs.createDir((char*)"vulkan");
        fs.createDir((char*)"X11");

        fs.changeDir((char*)"/bin");
        fs.createFile((char*)"bash", 400);
        fs.createFile((char*)"gcc", 500);
        fs.createFile((char*)"gdb", 400);
        fs.createFile((char*)"git", 300);
        fs.createFile((char*)"gnome-disks", 240);
        fs.createFile((char*)"gnome-terminal", 332);
        fs.createFile((char*)"clash", 112);
        fs.createFile((char*)"ssh", 235);

        fs.changeDir((char*)"/var");
        fs.createDir((char*)"lib");
        fs.createDir((char*)"log");
        fs.changeDir((char*)"lib");
        fs.createDir((char*)"dnf");
        fs.createDir((char*)"snapd");
        fs.createFile((char*)"dnf/history.sqlite", 133);
        fs.createFile((char*)"dnf/history.sqlite-shm", 102);
        fs.createDir((char*)"dnf/repos");
        fs.createFile((char*)"dnf/repos/fedora-15f125461563fe27", 300);
        fs.createFile((char*)"dnf/repos/updates-8ef319d1faeb5d3e", 200);

        fs.changeDir((char*)"/dev");
        fs.createDir((char*)"cpu");
        fs.createDir((char*)"cpu/0");
        fs.createDir((char*)"cpu/1");
        fs.createDir((char*)"cpu/2");
        fs.createDir((char*)"cpu/3");
        fs.createDir((char*)"disk");
        fs.createDir((char*)"net");
        fs.createFile((char*)"net/tun", 198);
        fs.createFile((char*)"stdin", 77);
        fs.createFile((char*)"stdout", 65);
        fs.createFile((char*)"nvme0n1p8", 21);
        fs.createFile((char*)"nvme0n1p9", 22);

        fs.changeDir((char*)"/lib");
        fs.createDir((char*)"firewalld");
        fs.createDir((char*)"grub");
        fs.createDir((char*)"systemd");

        fs.changeDir((char*)"/lib/firewalld");
        fs.createDir((char*)"ipsets");
        fs.createDir((char*)"policies");
        fs.createFile((char*)"policies/allow-host-ipv6.xml", 33);
        fs.createDir((char*)"services");
        fs.createFile((char*)"services/ssh.xml", 40);
        fs.createFile((char*)"services/dns.xml", 55);
        fs.createFile((char*)"services/http.xml", 120);
        fs.createFile((char*)"services/https.xml", 199);   
        fs.createDir((char*)"zones");
        fs.createFile((char*)"zones/public.xml", 200);
        fs.createFile((char*)"zones/trusted.xml", 175);

        fs.changeDir((char*)"/lib/systemd");
        fs.createDir((char*)"system");
        fs.createFile((char*)"system/bluetooth.service", 80);
        fs.createFile((char*)"system/httpd.socket", 70);
        fs.createFile((char*)"systemd-systemctl", 50);
        fs.createFile((char*)"systemd-sleep", 20);

        fs.changeDir((char*)"/");
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
        else if(strcmp(argv[0], "tree") == 0) {
            if(argc == 1) {
                fs.printTree();
            }
            else {
                std::cout<<"tree: arguments number is wrong"<<std::endl;
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

