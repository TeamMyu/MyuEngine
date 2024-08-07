
#include "APIClient.h"
#include "nlohmannjson.hpp"

#include <vector>
#include <string>

using namespace nlohmann;

class SDAPIs{
public:
    SDAPIs();
    bool setModel(string model);
	string txt2img(string prompt, string n_prompt, string sampler, int steps, int width, int height, bool upsacle, float cfg, int seed);
    int getProgress();

    float progress = 0.f;
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
}

int SDAPIs::getProgress() {
    string result = APIClient::Get("GetProgress");
    auto models_json = json::parse(result);
    return models_json["progress"].template get<int>();
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
string SDAPIs::txt2img(string prompt, string n_prompt, string sampler, int steps, int width, int height, bool upsacle, float cfg, int seed = -1) {
    json args;
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

	return APIClient::Post("CallSD", params.dump());
}