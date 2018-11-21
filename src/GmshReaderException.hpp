
#pragma once

#include <stdexcept>
#include <string>

class irm_exception : public std::runtime_error
{
public:
    irm_exception(const std::string& msg) : std::runtime_error(msg) {}
};

class unsupported_gmsh_version : public std::runtime_error
{
public:
    unsupported_gmsh_version(const std::string& msg) : std::runtime_error(msg) {}
};
