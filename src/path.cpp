#include "path.h"
#include "directory.h"
#include <string.h>
#include <iostream>

Path::Path(char* raw_path) {
    depth = 0;
    relative = true;
    unsigned short name_length = 0;
    size_t length = strlen(raw_path);
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
                if (name_length == Directory::meta.name_length) {
                    std::cout<<"[ERROR] name length can not be greater than 62 (including ending /0)!"<<std::endl;
                    exit(1);
                }
            }
            else {
                path[depth][name_length] = '\0';
                depth++;
                name_length = 0;
                if ( i == length - 1) {
                    path[depth][name_length] = '.';
                    name_length++;
                }
            }
        }
        path[depth][name_length] = '\0';
    }
} 

void Path::getPath(unsigned short iter, char* name) {
    for (int i = 0; i < Directory::meta.name_length; i++) {
        name[i] = path[iter][i];
    }
}

bool Path::isRelative() {
    return relative;
}

bool Path::isRoot() {
    if (depth == 0 && strlen(path[0]) == 0) {
        return true;
    }
    else {
        return false;
    }
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

void Path::separate(char* path, char* name) {
    int path_count = 0;
    int name_count = 0;
    char temp[Directory::meta.name_length];
    if(isRoot()) {
        path[0] = '/';
        path[1] = '\0';
        name[0] = '\0';
    }
    else {
        for (auto iter = begin(); iter < end(); iter = next(iter)) {
            getPath(iter, temp);
            if(isRelative() && iter == begin()) {
                path[path_count] = '.';
                path_count++;
                path[path_count] = '/';
                path_count++;
            }
            else {
                path[path_count] = '/';
                path_count++; 
            }
            if(next(iter) == end()) {
                for(size_t k = 0; k < strlen(temp); k++) {
                    name[name_count] = temp[k];
                    name_count++;
                }
                break;
            }
            for(size_t i = 0; i < strlen(temp); i++) {
                path[path_count] = temp[i];
                path_count++;
            }
        }
        if(path_count > 1 && path[path_count - 1] == '/') {
            path[path_count-1] = '\0';
            name[name_count] = '\0';
        } 
        else {
            path[path_count] = '\0';
            name[name_count] = '\0';
        }
    }
}