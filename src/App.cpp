#include <loguru.hpp>
#include "App;h"
#include "UI.h"

namespace App
{
    std::optional<YtxFile> file;

    void run()
    {
        if (UI::init() != 0)
        {
            ABORT_F("Error: Failed to initialize UI.");
        }

        UI::renderLoop();
    }
}