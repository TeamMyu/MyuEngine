
#include "APIClient.h"
#include "nlohmannjson.hpp"

#include <vector>
#include <string>

using namespace nlohmann;

class LLMAPIs{
public:
    LLMAPIs();
    string genText(string prompt);
    string callRAG(string content);
    void addRAG(string subject, string content);
    void deleteRAG(int index);
    void updateRAG(int index, string subject, string content);
    void syncRAG();
    void resetRAG();

    bool Init();

    std::vector<std::pair<string, string>> rag;
private:
};

LLMAPIs::LLMAPIs()
{
}

bool LLMAPIs::Init() {
    return true;
}

string LLMAPIs::genText(string prompt) {
    return APIClient::Post("CallAPI", prompt);
}

string LLMAPIs::callRAG(string content) {
    return APIClient::Post("CallRAG", content);
}

void LLMAPIs::addRAG(string subject, string content) {
    rag.push_back({ subject, content });
    return;
}

void LLMAPIs::deleteRAG(int index) {
    rag.erase(rag.begin() + index);
    return;
}

void LLMAPIs::updateRAG(int index, string subject, string content) {
    rag[index] = { subject, content };
    return;
}

void LLMAPIs::syncRAG() {

    if (rag.empty()) {
        string result = APIClient::Post("AddRAG", "");
        auto rags_json = json::parse(result);
        for (auto& el : rags_json.items()) {
            rag.push_back({ el.key(), el.value() });
        }
    }
    else {
        json args;
        for (auto ele : rag)
        {
            args[ele.first] = ele.second;
        }

        string result = APIClient::Post("AddRAG", args.dump());
        auto rags_json = json::parse(result);
        for (auto& el : rags_json.items()) {
            rag.push_back({ el.key(), el.value() });
        }
    }
    return;
}

void LLMAPIs::resetRAG() {
    APIClient::Get("InitAPI");
    rag.clear();
    return;
}