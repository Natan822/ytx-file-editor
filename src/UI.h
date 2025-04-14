#pragma once
#include <SDL3/SDL.h>

namespace UI
{
    extern const char* WINDOW_TITLE;
    extern int WINDOW_WIDTH;
    extern int WINDOW_HEIGHT;
    extern const int MAX_PATH;

    extern SDL_Window* window;
    extern SDL_Renderer* renderer;

    int init();

    void renderLoop();
    void renderTable();
    void renderSectionSelect();
    void renderFilterBox();

    void updateDisplayEntries();
    void getFilePath(std::string& buffer);

    void loadFile(std::string path);
    void saveFile();

    void loadFileButton();
    void saveFileButton();
}