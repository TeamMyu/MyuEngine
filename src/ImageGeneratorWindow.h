#pragma once

#include "SDAPIs.h"

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <queue>

class ImageComponent
{
public:
    enum FITSTYLE
    {
        NONE,
        WIDTH_AND_HEIGHT,
        WIDTH_OR_HEIGHT,
    };

    unsigned int handle = 0;
    int width = 0;
    int height = 0;
    FITSTYLE fitFrame = FITSTYLE::WIDTH_OR_HEIGHT;

    bool FromFile(const char* filename);
    bool FromMemory(std::string file);
    void ToFile(const char* filename);
    std::string Data();
    void Draw(float x, float y, float pad);
};

class ImageGeneratorWindow {
public:
    ImageGeneratorWindow();

    void GenCharacter();
    void Draw();

private:
    std::mutex queueMutex;
    std::queue<std::string> genQueue;
    ImageComponent result_image;
    ImageComponent base_image;
    std::vector<ImageComponent> reference_images;

    float progress = 0;

};