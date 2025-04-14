#include <loguru.hpp>
#include "App;h"

int main(int argc, char **argv)
{
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::init(argc, argv);

    App::run();
    return 0;
}