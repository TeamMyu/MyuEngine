#pragma once

#include <string>
#include <vector>

class ImageGeneratorWindow {
public:
    ImageGeneratorWindow();

    struct ImageGenParams
    {
        std::string model;
        std::string lora;
        std::string prompt;
        std::string n_prompt;
        std::string sampler;
        int steps;
        int width;
        int height;
        bool upsacle;
        float cfg;
        int seed;
    };

    void GenCharacter(ImageGenParams params);
    void Draw();

private:
    unsigned int image = 0;
    int width = 0;
    int height = 0;
};