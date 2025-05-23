#pragma once
#include <SDL3/SDL.h>
#include "YtxFile.h"

namespace UI
{
    extern std::string WINDOW_TITLE;
    extern int WINDOW_WIDTH;
    extern int WINDOW_HEIGHT;
    extern const int MAX_PATH;

    extern SDL_Window *window;
    extern SDL_Renderer *renderer;

    int init();

    void renderLoop();
    void renderTable();
    void renderSectionSelect();
    void renderFilterBox();
    void renderPopUpAddEntry();
    void renderMessagePopUp();

    void getFilePath(std::string &buffer);

    void loadFile(std::string path);
    void saveFile();

    void loadFileButton();
    void saveFileButton();

    bool isEntryDisplayed(Entry entry);

    void updateDisplayEntries();
    void fillSectionOptions();

    bool addEntryButton(std::string _string, int entryId, int sectionId);

    namespace PopUp
    {
        namespace Message
        {
            extern std::string message;
            void newPopUp(std::string _message);
        };

        namespace AddEntry
        {
            extern int selectedSection;
            extern std::string stringBuffer;
            extern std::string entryIdBuffer;

            extern bool displayError;
            extern std::string errorMessage;

            void renderErrorMessage();
            void addError(std::string message);
            void reset();
        };
    };
}