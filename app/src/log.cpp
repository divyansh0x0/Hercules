#include <log.h>
#include <iostream>
// #include <string.h>
// #include <iostream>
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET_NEWLINE "\x1b[0m\n"

// Implementation of variable arguments using variadic templates.
// using namespace std;

void log::info(std::string s)
{
    std::cout<< CYAN << s << RESET_NEWLINE;
}

void log::success(std::string s)
{
    std::cout<< GREEN << s << RESET_NEWLINE;
    // printf("%s%s%s", GREEN, s, RESET_NEWLINE);
}

void log::error(std::string s)
{
    std::cout<< RED << s << RESET_NEWLINE;
}

void log::warn(std::string s)
{
    std::cout<< YELLOW << s << RESET_NEWLINE;
}
