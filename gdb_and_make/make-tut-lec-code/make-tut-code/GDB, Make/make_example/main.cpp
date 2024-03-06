// main.cpp
#include "file.h"
#include "directory.h"
#include <iostream>

int main() {
    File file("example.txt");
    file.create();

    Directory directory("example_dir");
    directory.create();

    return 0;
}