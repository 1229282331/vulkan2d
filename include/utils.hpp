#pragma once
#include <iostream>
#include <vector>

namespace Utils{

template<typename T>
void display(std::vector<T> enums)
{
    std::cout << "[";
    for(auto e : enums)
        std::cout << e << ",";
    std::cout << "\b]";
}

}

template<typename T>
std::ostream&  operator <<(std::ostream& os, std::vector<T> enums)
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

