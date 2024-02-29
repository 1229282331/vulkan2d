#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <string>


namespace utils{

template<typename T>
void display(std::vector<T> enums)
{
    std::cout << "[";
    for(auto e : enums)
        std::cout << e << ",";
    std::cout << "\b]";
}

inline std::vector<char> readFile(const std::string& filename, std::ios_base::openmode mode=std::ios::ate|std::ios::binary)
{
    std::ifstream file(filename, mode);
    if(!file.is_open())
    {
        std::cout << "Failed to read " << filename << " !" << std::endl;
        return std::vector<char>();
    }

    auto size = file.tellg();
    std::vector<char> content(size);
    file.seekg(0);
    file.read(content.data(), size);
    file.close();

    return content;
}

}

template<typename T>
std::ostream&  operator<<(std::ostream& os, std::vector<T> enums)
{
    if(enums.empty())
        std::cout << "[]";
    else
    {
        std::cout << "[";
        for(auto e : enums)
            std::cout << e << ", ";
        std::cout << "\b\b]";
    }

    return os;
}

