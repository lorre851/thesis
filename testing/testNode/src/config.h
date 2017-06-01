//
// Created by lorre851 on 16.03.17.
//

#ifndef THESIS_CONFIG_H
#define THESIS_CONFIG_H

#include <iostream>
#include <sys/stat.h>
#include <stdio.h>

using namespace std;

class config {
public:
    static bool file_exists(const string&);
    static bool to_bool(int);
};


#endif //THESIS_CONFIG_H
