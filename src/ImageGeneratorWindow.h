#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <queue>

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
        bool noBackground;
        std::string version;
    };

    void GenCharacter(ImageGenParams params);
    void Draw();

private:
    std::mutex queueMutex;
    std::queue<std::string> genQueue;
    unsigned int image = 0;
    int width = 0;
    int height = 0;
    float progress = 0;

    int current_model_idx = 0;
    int current_version_idx = 0;
    const char* current_model_version[2]{ "1.5", "XL" };
    int current_sampler_idx = 3;
    int current_lora_idx = 0;
    const char* widths[12] { "512", "640", "768", "832", "896", "1024", "1080", "1152", "1216", "1344", "1536", "1920" };
    const char* heights[12]{ "512", "640", "768", "832", "896", "1024", "1080", "1152", "1216", "1344", "1536", "1920" };
    int current_width_idx = 5;  
    int current_height_idx = 5;
    bool upsample = false;
    bool no_background = true;
    char input_seed[32] = "-1";
    char input_steps[32] = "28";
    char input_cfg[32] = "7.0";
    char input_prompt[512 * 16] = {0,};
    char input_nprompt[512 * 16] = { 0, };
};