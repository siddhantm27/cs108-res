// file.h
#ifndef FILE_H
#define FILE_H

#include <string>

class File {
public:
    File(const std::string& name) : name(name) {}
    void create();

private:
    std::string name;
};

#endif // FILE_H
