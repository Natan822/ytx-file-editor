#include <loguru.hpp>
#include "App;h"

int main(int argc, char **argv)
{
    loguru::init(argc, argv);
    
    App::run();
    return 0;
}