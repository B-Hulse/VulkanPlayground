#include "pch.h"

#include <iostream>
#include "BasicTriangleApplication.h"

int main()
{
    BasicTriangleApplication app(2);

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    EXIT_SUCCESS;
}