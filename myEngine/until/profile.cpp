#include "profile.hpp"
#include <iostream>

Profile::Profile(const std::string &name) : mName(name), mStart(std::chrono::high_resolution_clock::now())
{
}

Profile::~Profile()
{
    auto duration = std::chrono::high_resolution_clock::now() - mStart;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "Profile \"" << mName << "\":" << ms << "ms" << std::endl;
}
