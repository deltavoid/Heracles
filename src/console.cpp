#include "console.h"
#include <string>
#include <iostream>

Console::Console(Tap* tap) : tap(tap)
{
}

void* Console::run(void* arg)
{
    Console* This = (Console*)arg;
    std::string operation;

    while (std::cin >> operation)
    {   
        if  (operation == "lc")
        {   int pid = 0;
            std::cin >> pid;
            This->tap->set_LC_pid(pid);
            std::cout << "lc: " << pid << std::endl;
        }
        else if  (operation == "be")
        {   int pid = 0;
            std::cin >> pid;
            This->tap->set_BE_pid(pid);
            std::cout << "be: " << pid << std::endl;
        }
        else
        {   std::cout << "usage:" << std::endl;
        }
    }

}