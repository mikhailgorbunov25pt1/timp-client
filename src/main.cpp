#include "user_interface.h"

#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try {
        UserInterface().run(argc, argv);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
