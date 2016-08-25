/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <stdexcept>
#include <string>

class GmshReaderException: public std::runtime_error
{
public:
    GmshReaderException(const std::string& msg) : std::runtime_error(msg){}
};

class UnsupportedGmshVersion: public std::runtime_error
{
public:
    UnsupportedGmshVersion(const std::string& msg):
        std::runtime_error(msg){}
};
