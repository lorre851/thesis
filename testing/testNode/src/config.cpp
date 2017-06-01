//
// Created by lorre851 on 16.03.17.
//

#include "config.h"



bool config::file_exists(const string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool config::to_bool(int size) {
    if(size <= 0) return false;
    else return true;
}

