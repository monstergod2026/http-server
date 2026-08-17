#pragma once
#include "err.hpp"
#include <iostream>

inline void LogMessage(const std::string& message) //简单实现
{
    std::cout << "[LOG]: " << message << std::endl;
}