#include <cassert>
#include <iostream>
#include <exception>

int main()
{
    try
    { 
        // Test if assertions are enabled; THIS TEST SHOULD FAIL!
        assert(false);
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    
}