#include <SDL3/SDL.h>
#include <iostream>
#include <imgui.h>
#include <imgui_stdlib.h>
#include "imgui_impl_sdl3.h"
#include <imgui_impl_sdlrenderer3.h>
#include <nfd.h>
#include <loguru.hpp>
#include <optional>
#include <sstream>
#include <atomic>
#include <thread>

#include "UI.h"
#include "YtxFile.h"
#include "App;h"
#include "Utils.h"

namespace UI
{
    const char* WINDOW_TITLE = "UD3 String Editor";
    int WINDOW_WIDTH = 1150;
    int WINDOW_HEIGHT = 646;

    const int MAX_PATH = 256;

    std::vector<Entry*> displayEntries = {};
    std::vector<std::string> sectionOptions = {"All sections"};
    int selectedSection = 0;


    SDL_Window* window;
    SDL_Renderer* renderer;

    std::string stringFilter;
    std::string filePathBuffer;


    std::atomic<bool> isLoadingFile = false;
    bool isFileOpen = false;
    
    int init()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            ABORT_F("Error: SDL_Init(): %s", SDL_GetError());
        }

        window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!window)
        {
            ABORT_F("Error: SDL_CreateWindow(): %s", SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer)
        {
            ABORT_F("Error: SDL_CreateRenderer(): %s", SDL_GetError());
        }

        SDL_SetRenderVSync(renderer, 1);
        SDL_ShowWindow(window);

        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);

        return 0;
    }

    void renderLoop()
    {
        bool quit = false;

        nfdu8char_t *outPath = "##";
        while (!quit)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT)
                {
                    quit = true;
                }
            }

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            SDL_GetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);

            ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
            ImGui::SetNextWindowPos(ImVec2(0, 0));

            ImGui::Begin("Hello World!", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("Open a .ytx file:");
            ImGui::InputText("##path", &filePathBuffer);
            ImGui::SameLine();
            if (ImGui::Button("Browse"))
            {
                getFilePath(filePathBuffer);
            }

            if (ImGui::Button("Load file") && !isLoadingFile)
            {
                loadFileButton();
            }

            if (isLoadingFile)
            {
                ImGui::Text("Loading file ...");
            }

            if (isFileOpen && !isLoadingFile)
            {
                ImGui::SameLine();
                if (ImGui::Button("Save Changes"))
                {
                    App::file->saveChanges();
                }

                renderFilterBox();
                renderSectionSelect();
                renderTable();
            }

            ImGui::End();
            ImGui::Render();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
        }
    }

    void renderTable()
    {
        if (!isFileOpen)
        {
            LOG_F(ERROR, "Table cannot be displayed: No .ytx files are open.");
            return;
        }

        const int COLUMNS_COUNT = 4;
        const char* headers[] = {"ID", "String", "Address"};

        if (ImGui::BeginTable("main_table",
                              COLUMNS_COUNT,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg,
                              ImVec2(0, WINDOW_HEIGHT * 0.75f)))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.1f);
            ImGui::TableSetupColumn(headers[0], ImGuiTableColumnFlags_WidthStretch, 0.2f);
            ImGui::TableSetupColumn(headers[1]);
            ImGui::TableSetupColumn(headers[2]);

            ImGui::TableHeadersRow();

            int selectedId = std::stoul(sectionOptions.at(selectedSection), nullptr, 16);

            ImGuiListClipper clipper;
            clipper.Begin(displayEntries.size());

            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    Entry* entry = displayEntries.at(row);
                    ImGui::TableNextRow();

                    // Row Index
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(std::to_string(row + 1).c_str());

                    // String ID
                    std::stringstream stringId;
                    stringId << std::hex << entry->id;

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(stringId.str().c_str());

                    // String
                    ImGui::TableSetColumnIndex(2);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushID(row);
                    ImGui::InputText("##", &entry->_string);
                    ImGui::PopID();

                    // Address
                    std::stringstream address;
                    address << std::hex << entry->stringAddress;

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text(address.str().c_str());
                }
            }

            ImGui::EndTable();
        }
    }

    void renderSectionSelect()
    {
        if (!isFileOpen)
        {
            LOG_F(ERROR, "Cannot load Entry Sections selection box: No .ytx files are open.");
            return;
        }

        if (App::file->entrySections.size() == 0)
        {
            LOG_F(ERROR, "Cannot load Entry Sections selection box: No entry sections found.");
            return;
        }

        ImGui::Text("Select section:");
        if (ImGui::BeginCombo("##combo", sectionOptions.at(selectedSection).c_str()))
        {
            if (sectionOptions.size() == 1)
            {
                for (EntrySection section : App::file->entrySections)
                {
                    std::stringstream sstream;
                    sstream << std::hex << section.id;
                    
                    sectionOptions.push_back(std::string(sstream.str()));
                }
            }
            
            for (int i = 0; i < sectionOptions.size(); i++)
            {
                if (ImGui::Selectable(sectionOptions.at(i).c_str()))
                {
                    selectedSection = i;
                    updateDisplayEntries();
                }
            }
            
            ImGui::EndCombo();
        }
    }

    void renderFilterBox()
    {
        ImGui::Text("Filter:");
        if (ImGui::InputText("##filter", &stringFilter))
        {
            updateDisplayEntries();
        }
    }

    void updateDisplayEntries()
    {
        displayEntries.clear();

        // All sections
        if (selectedSection == 0)
        {
            for (EntrySection& section : App::file->entrySections)
            {
                for (Entry& entry : section.entries)
                {
                    if (stringFilter.size() == 0)
                    {
                        displayEntries.push_back(&entry);
                    }
                    else if (entry._string.find(stringFilter) != std::string::npos)
                    {
                        displayEntries.push_back(&entry);
                    }
                }
            }
        }
        else
        {
            int selectedSectionId = std::stoul(sectionOptions.at(selectedSection), nullptr, 16);
            for (EntrySection& section : App::file->entrySections)
            {
                if (section.id == selectedSectionId)
                {
                    for (Entry &entry : section.entries)
                    {
                        if (stringFilter.size() == 0)
                        {
                            displayEntries.push_back(&entry);
                        }
                        else if (entry._string.find(stringFilter) != std::string::npos)
                        {
                            displayEntries.push_back(&entry);
                        }
                    }
                    break;
                }
            }
        }
        
    }

    void getFilePath(std::string& buffer)
    {
        NFD_Init();

        char *outPath;
        nfdopendialogu8args_t args = {0};
        nfdu8filteritem_t filters[1] = {{"YTX Files", "ytx"}};
        args.filterList = filters;
        args.filterCount = 1;

        nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

        NFD_Quit();
        
        if (result == NFD_OKAY)
        {
            buffer = outPath;
        }
    }

    void loadFile(std::string path)
    {
        selectedSection = 0;
        sectionOptions.resize(1);
        App::file.emplace(path);
        App::file->load();
        updateDisplayEntries();

        isLoadingFile = false;
        isFileOpen = true;
    }

    void loadFileButton()
    {
        if (filePathBuffer.empty())
        {
            LOG_F(WARNING, "Attempting to load a file without a specified path.");
            return;
        }

        if (isFileOpen)
        {
            // Prevent opening a file that's already open file
            if (!(App::file->comparePath(filePathBuffer)))
            {
                isLoadingFile = true;

                std::thread loadFileThread(loadFile, filePathBuffer);
                loadFileThread.detach();
            }
            return;
        }

        isLoadingFile = true;

        std::thread loadFileThread(loadFile, filePathBuffer);
        loadFileThread.detach();
    }
}