#include <iostream>

void printOptions()
{
    std::cout << "Options:" << std::endl;
    std::cout << "\t-h, --help: print options" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        printOptions();
        return 0;
    }

    return 0;
}
