#include "path.h"
#include <string.h>
#include <iostream>

Path::Path(unsigned char* raw_path) {
    depth = 0;
    relative = true;
    unsigned short name_length = 0;
    size_t length = strlen((char*)raw_path);
    if(length == 0) {
        std::cout<<"[ERROR] path can not be empty!"<<std::endl;
        exit(1);
    }
    else {
        size_t i = 0;
        if(raw_path[0] == '/') {
            relative = false;
            i = 1;
        }
        for (i; i < length; i++) {
            if (raw_path[i] != '/') {
                path[depth][name_length] = raw_path[i];
                name_length++;
                if (name_length == 62) {
                    std::cout<<"[ERROR] name length can not be greater than 62 (including ending /0)!"<<std::endl;
                    exit(1);
                }
            }
            else {
                path[depth][name_length] = '\0';
                depth++;
                name_length = 0;
            }
        }
        path[depth][name_length] = '\0';
    }
} 

void Path::getPath(unsigned short iter, unsigned char* name) {
    for (int i = 0; i < 62; i++) {
        name[i] = path[iter][i];
    }
}

bool Path::isRelative() {
    return relative;
}

unsigned short Path::begin() {
    return 0;
}

unsigned short Path::end() {
    return depth + 1;
}

unsigned short Path::next(unsigned short iter) {
    return iter+1;
} 