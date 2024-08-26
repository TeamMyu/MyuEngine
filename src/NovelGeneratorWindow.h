#pragma once

#include <string>
#include <vector>

class NovelGeneratorWindow {
public:
    NovelGeneratorWindow();

    struct NovelGenParams
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

    std::string GenNovel(std::string params);
    void Draw();

private:

};