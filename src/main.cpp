// main server loop
// goal implement with modules and c++ 23
#include <iostream>
#include "itchparser.h"

void itch_worker();

int main()
{
    std::cout << "Server loop running..." << std::endl;
    itch_worker();
    return 0;
}