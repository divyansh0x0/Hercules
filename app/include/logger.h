#pragma once

#include <string>

namespace logger
{
    void info(std::string s);
    void warn(std::string s);
    void error(std::string s);
    void success(std::string s);

}
