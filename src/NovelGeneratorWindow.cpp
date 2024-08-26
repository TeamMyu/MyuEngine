
#include "NovelGeneratorWindow.h"

#include "APIClient.h"
#include "LLMAPIs.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//void MyCombo(const char* label, const char** items, uint8_t size, int& current_idx)
//{
//    const char* preview = items[current_idx];
//    if (ImGui::BeginCombo(label, preview, 0))
//    {
//        for (int i = 0; i < size; i++)
//        {
//            const bool is_selected = (current_idx == i);
//            if (ImGui::Selectable(items[i], is_selected))
//                current_idx = i;
//
//            if (is_selected)
//                ImGui::SetItemDefaultFocus();
//        }
//
//        ImGui::EndCombo();
//    }
//}
//
//void MyCombo(const char* label, vector<string> items, int& current_idx)
//{
//    const char* preview = items[current_idx].c_str();
//    if (ImGui::BeginCombo(label, preview, 0))
//    {
//        for (int i = 0; i < items.size(); i++)
//        {
//            const bool is_selected = (current_idx == i);
//            if (ImGui::Selectable(items[i].c_str(), is_selected))
//                current_idx = i;
//
//            if (is_selected)
//                ImGui::SetItemDefaultFocus();
//        }
//
//        ImGui::EndCombo();
//    }
//}

LLMAPIs* llm_api;

NovelGeneratorWindow::NovelGeneratorWindow() {
    llm_api = new LLMAPIs();
    llm_api->Init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesKorean());
}

std::string NovelGeneratorWindow::GenNovel(std::string params) {
    
    return llm_api->genText(params);
}

struct PLOT {
    int id;
    char name[255];
};


struct EPISODE {
    int id;
    char* name;
    std::vector<PLOT> plots;
};

std::vector<EPISODE> episodes;

void NovelGeneratorWindow::Draw() {

    ImGui::Begin("Novel Gen");
    ImGui::SetWindowSize(ImVec2(1024, 800));

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("NovelGenTab", tab_bar_flags))
    {
        if (ImGui::BeginTabItem(u8"소설"))
        {
            static char novel[512 * 16] = "";

            ImGui::Text(u8"장르");
            static char genre[512 * 1] = "";
            ImGui::InputTextMultiline("genre", genre, IM_ARRAYSIZE(genre), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), 0);

            ImGui::BeginGroup();
            if (ImGui::Button(u8"에피소드 추가")) {
                episodes.push_back({ (int)episodes.size(), u8"에피소드"});
            }

            if (ImGui::TreeNode(u8"에피소드"))
            {
                for each (auto epi in episodes)
                {
                    if (epi.id == 0)
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                    ImGui::PushID(epi.id);
                    if (ImGui::TreeNode("", "%s %d", epi.name, epi.id))
                    {
                        ImGui::Text("blah blah");
                        ImGui::SameLine();
                        if (ImGui::SmallButton(u8"플롯 추가")) {
                            strcpy_s(novel, u8"테스트 2");
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginGroup();
            
            ImGui::InputTextMultiline("novel", novel, IM_ARRAYSIZE(novel), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);
            ImGui::EndGroup();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"데이터베이스"))
        {
            static int selectIndex = -1;
            static int prevIndex = -1;

            static char rag_subject[256] = "";
            static char rag_content[512 * 16] = "";

            if (ImGui::BeginListBox("##draw_list", ImVec2(200, 640))) {
        
                std::vector<const char*> items;
                int idx = 0;
                items.clear();
                for (int i = 0; i < llm_api->rag.size(); i++)
                {
                    items.push_back(llm_api->rag[i].first.c_str());
                }

                for (int n = 0; n < items.size(); ++n) {
                    const bool is_selected = (selectIndex == n);
                    const bool isIndexChanged = selectIndex != prevIndex;
                    if (ImGui::Selectable(items[n], is_selected)) { selectIndex = n; }

                    if (is_selected && isIndexChanged) {
                        std::cout << "selected: " << selectIndex << std::endl;
                        if (prevIndex > -1) {
                            strcpy_s(rag_subject, sizeof(rag_subject), llm_api->rag[selectIndex].first.c_str());
                            strcpy_s(rag_content, sizeof(rag_content), llm_api->rag[selectIndex].second.c_str());
                        }

                        ImGui::SetItemDefaultFocus();
                        prevIndex = selectIndex;
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::InputText("subject", rag_subject, IM_ARRAYSIZE(rag_subject));

            ImGui::SameLine();

            if (ImGui::Button(u8"자동 생성")) {
                //llm_api->addRAG(rag_subject, rag_content);    
                strcpy_s(rag_content, llm_api->genText(rag_content).c_str());
            }

            ImGui::InputTextMultiline("content", rag_content, IM_ARRAYSIZE(rag_content), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 32), 0);

            if (ImGui::Button(u8"추가")) {
                llm_api->addRAG(rag_subject, rag_content);
                memset(rag_subject, 0, sizeof(rag_subject));
                memset(rag_content, 0, sizeof(rag_content));

                selectIndex = -1;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"수정")) {
                std::cout << selectIndex << std::endl;
                llm_api->updateRAG(selectIndex, rag_subject, rag_content);
                memset(rag_subject, 0, sizeof(rag_subject));
                memset(rag_content, 0, sizeof(rag_content));

                selectIndex = -1;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"삭제")) {
                llm_api->deleteRAG(selectIndex);
                memset(rag_subject, 0, sizeof(rag_subject));
                memset(rag_content, 0, sizeof(rag_content));

                selectIndex = -1;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"동기화")) {
                llm_api->syncRAG();
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"초기화")) {
                llm_api->resetRAG();
            }

            ImGui::EndGroup();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

}