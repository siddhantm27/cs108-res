// directory.h
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>

class Directory {
public:
    Directory(const std::string& name) : name(name) {}
    void create();

private:
    std::string name;
};

#endif // DIRECTORY_H
