
#include "APIClient.h"
#include "nlohmannjson.hpp"

#include <vector>
#include <string>
#include <future>

using namespace nlohmann;

class SDAPIs {
public:
    SDAPIs();
    bool setModel(string model);
    void txt2img(std::function<void(string)> callback, string prompt, string n_prompt, string sampler, int steps, int width, int height, bool upsacle, float cfg, bool noBackground, string version, int seed);
    float getProgress();
    bool Init();

    float progress = 0.f;
    bool isStarted;
    std::vector<std::string> models;
    std::vector<std::string> loras;
    std::vector<std::string> samplers;

private:

};

std::string& removeQuotes(std::string& str) {
    str.erase(remove(str.begin(), str.end(), '\"'), str.end());
    return str;
}

std::string& removeExtension(std::string& str) {
    str.erase(str.find_last_of("."), string::npos);
    return str;
}

SDAPIs::SDAPIs() :
    models(std::vector<std::string>()),
    loras(std::vector<std::string>()),
    samplers(std::vector<std::string>())
{
}

bool SDAPIs::Init() {
    loras.push_back("None");

    string result = APIClient::Get("GetSDModels");
    auto models_json = json::parse(result);
    for (auto& element : models_json) {
        result = element.dump();
        models.push_back(removeQuotes(result));
    }

    result = APIClient::Get("GetSamplers");
    auto samplers_json = json::parse(result);
    for (auto& element : samplers_json) {
        result = element["name"].dump();
        samplers.push_back(removeQuotes(result));
    }

    result = APIClient::Get("GetLoRAs");
    auto loras_json = json::parse(result);
    for (auto& element : loras_json) {
        result = element.dump();
        loras.push_back(removeExtension(removeQuotes(result)));
    }

    return true;
}

float SDAPIs::getProgress() {
    static int tick = GetTickCount();
    static float prev = 0.f;

    if (GetTickCount() - tick > 512) {
        tick = GetTickCount();
        string result = APIClient::Get("GetProgress");
        auto models_json = json::parse(result);
        prev = models_json["progress"].get<float>();
    }

    return prev;
}

bool SDAPIs::setModel(string model) {
    json args;
    args["model"] = model;
    json params = { {"args", args} };
    string result = APIClient::Post("SetSDModel", params.dump());

    if (strcmp(result.c_str(), "true"))
        return true;
    else
        return false;

    return  true;
}

void SDAPIs::txt2img(std::function<void(string)> callback, string prompt, string n_prompt, string sampler, int steps, int width, int height, bool upsacle, float cfg, bool noBackground, string version, int seed = -1) {
    json args;
    args["version"] = version;
    args["background"] = !noBackground;

    args["seed"] = seed;
    args["prompt"] = prompt;
    args["negative_prompt"] = n_prompt;
    args["upscale"] = upsacle;
    args["sampler"] = sampler;
    args["steps"] = steps;
    args["cfg"] = 7.f;
    args["width"] = width;
    args["height"] = height;
    json params = { {"args", args} };

    APIClient::PostAsync("CallSD", params.dump(), callback);
    return;
}