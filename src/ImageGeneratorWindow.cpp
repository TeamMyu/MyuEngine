
#include "ImageGeneratorWindow.h"

#include "APIClient.h"
#include "SDAPIs.hpp"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <time.h>
#include <thread>
using std::thread;

#include "../vendor/ImGuiFileDialog/ImGuiFileDialog.h"

void MyCombo(const char* label, const char** items, uint8_t size, int& current_idx)
{
    const char* preview = items[current_idx];
    if (ImGui::BeginCombo(label, preview, 0))
    {
        for (int i = 0; i < size; i++)
        {
            const bool is_selected = (current_idx == i);
            if (ImGui::Selectable(items[i], is_selected))
                current_idx = i;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
}

void MyCombo(const char* label, vector<string> items, int& current_idx)
{
    const char* preview = items[current_idx].c_str();
    if (ImGui::BeginCombo(label, preview, 0))
    {
        for (int i = 0; i < items.size(); i++)
        {
            const bool is_selected = (current_idx == i);
            if (ImGui::Selectable(items[i].c_str(), is_selected))
                current_idx = i;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

bool LoadTextureFromMemory(std::string file, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    const unsigned char* uchar_str = reinterpret_cast<const unsigned char*>(file.c_str());
    unsigned char* image_data = stbi_load_from_memory(uchar_str, file.size(), &image_width, &image_height, NULL, 4);

    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

SDAPIs* sd_api;

ImageGeneratorWindow::ImageGeneratorWindow() {
    sd_api = new SDAPIs();
    sd_api->Init();
}

void ImageGeneratorWindow::GenCharacter(ImageGenParams params) {
    cout << sd_api->setModel(params.model) << endl;

    stringstream ss;
    ss << params.prompt;
    ss << ", <" << params.lora << ":1>";
    std::cout << "callback fnc: " << std::this_thread::get_id() << std::endl;
    std::function<void(std::string)> callback = [this](std::string file) {
        while (!queueMutex.try_lock());
        genQueue.push(file);
        queueMutex.unlock();
    };

    sd_api->txt2img(callback, ss.str(), params.n_prompt, params.sampler, params.steps,
        params.width, params.height, params.upsacle, params.cfg, params.noBackground, params.version ,params.seed);

    std::cout << "main: " << std::this_thread::get_id() << std::endl;
    return;
}

void ImageGeneratorWindow::Draw() {
    ImGui::Begin("Image Gen");
    ImGui::SetWindowSize(ImVec2(1920, 1080));

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("ImageGenTab", tab_bar_flags))
    {
        static bool onGenerate = false;

        if (ImGui::BeginTabItem("Simple"))
        {
            while (!queueMutex.try_lock());
            if (!genQueue.empty()) {
                std::cout << "get queue file" << std::endl;
                auto file = genQueue.front();
                genQueue.pop();

                std::cout << "gen image" << std::endl;
                LoadTextureFromMemory(file, (GLuint*)&this->image, &this->width, &this->height);
            } 
            queueMutex.unlock();

       
            
            //static ImVec2 size(512.f, 512.f);
            //static ImVec2 offset(0.0f, 0.0f);

            //ImGui::TextWrapped("(Click and drag to scroll)");

            //ImGui::PushID(0);
            //ImGui::InvisibleButton("##canvas", size);
            //if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            //{
            //    offset.x += ImGui::GetIO().MouseDelta.x;
            //    offset.y += ImGui::GetIO().MouseDelta.y;
            //}
            //ImGui::PopID();

        

            //const ImVec2 p0 = ImGui::GetItemRectMin();
            //const ImVec2 p1 = ImGui::GetItemRectMax();
            //const char* text_str = "Line 1 hello\nLine 2 clip me!";
            //const ImVec2 text_pos = ImVec2(p0.x + offset.x, p0.y + offset.y);
            //ImDrawList* draw_list = ImGui::GetWindowDrawList();

            //ImGui::PushClipRect(p0, p1, true);
            //draw_list->AddRectFilled(p0, p1, IM_COL32(45, 45, 45, 255));
            ////draw_list->AddText(text_pos, IM_COL32_WHITE, text_str);

            //ImGui::Image((void*)(intptr_t)this->image, ImVec2(128.f, 128.f));
            //ImGui::PopClipRect();
         
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(45, 45, 45, 255));
            ImGui::BeginChildFrame(1, { 512 + 32, 512 + 64 }, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Result Size = %d x %d", this->width, this->height);
            
            //ImGui::Image((void*)(intptr_t)this->image, ImVec2(this->width, this->height));

            if (this->image != 0) {
                onGenerate = false;
                ImGui::SetCursorPos({ ImGui::GetCursorPos().x + ((512.f + 32.f) / 2.f - (this->width / (this->height / 512.f) / 2.f)), ImGui::GetCursorPos().y });
                ImGui::Image((void*)(intptr_t)this->image, ImVec2(this->width / (this->height / 512.f), this->height / (this->height / 512.f)));
            }

            ImGui::EndChildFrame();
            ImGui::PopStyleColor();

            ImGui::SameLine();

            static int i2i_image;
            static int i2i_width;
            static int i2i_height;

            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(45, 45, 45, 255));
            ImGui::BeginChildFrame(2, { 512 + 32, 512 + 64 }, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("size = %d x %d", i2i_width, i2i_height);

            ImGui::SameLine();

            if (ImGui::Button("open file")) {
                std::locale::global(std::locale("ko_KR.UTF-8"));

                IGFD::FileDialogConfig config;
                config.path = ".";
                ImGuiFileDialog::Instance()->OpenDialog("filedialog1", "choose file", ".png,.jpg,.bmp,.jpeg", config);
            }

            if (i2i_image != 0) {
                onGenerate = false;
                ImGui::SetCursorPos({ ImGui::GetCursorPos().x + ((512.f + 32.f) / 2.f - (i2i_width / (i2i_height / 512.f) / 2.f)), ImGui::GetCursorPos().y });
                ImGui::Image((void*)(intptr_t)i2i_image, ImVec2(i2i_width / (i2i_height / 512.f), i2i_height / (i2i_height / 512.f)));
            }

            ImGui::EndChildFrame();
            ImGui::PopStyleColor();

            // display
            if (ImGuiFileDialog::Instance()->Display("filedialog1")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();
                    // action

                    std::cout << filepathname << std::endl;
                    LoadTextureFromFile(filepathname.c_str(), (GLuint*)&i2i_image, &i2i_width, &i2i_height);
                }

                // close
                ImGuiFileDialog::Instance()->Close();
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Advanced")) 
        {
            MyCombo("version", current_model_version, IM_ARRAYSIZE(current_model_version), current_version_idx);
            MyCombo("model", sd_api->models, current_model_idx);
            MyCombo("sampler", sd_api->samplers, current_sampler_idx);
            MyCombo("lora", sd_api->loras, current_lora_idx);
            MyCombo("width", widths, IM_ARRAYSIZE(widths), current_width_idx);
            MyCombo("height", heights, IM_ARRAYSIZE(heights), current_height_idx);
            ImGui::Checkbox("upsample", &upsample);
            ImGui::Checkbox("no background", &no_background);
            ImGui::InputText("random seed", input_seed, IM_ARRAYSIZE(input_seed));
            ImGui::InputText("sampling step", input_steps, IM_ARRAYSIZE(input_steps));
            ImGui::InputText("cfg scale", input_cfg, IM_ARRAYSIZE(input_cfg));
            ImGui::InputTextMultiline("positive prompt", input_prompt, IM_ARRAYSIZE(input_prompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);
            ImGui::InputTextMultiline("negative prompt", input_nprompt, IM_ARRAYSIZE(input_nprompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            //static char etc_text[512 * 16] = "etc input";
            //ImGui::InputTextMultiline("etc", etc_text, IM_ARRAYSIZE(etc_text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            //// 구도
            //const char* poses[] = { "standing", "sitting" };
            //static int current_pose_idx = 0;
            //MyCombo("pose", poses, IM_ARRAYSIZE(poses), current_pose_idx);

            //// 성별
            //const char* sex[] = { "male", "female" };
            //static int current_sex_idx = 0;
            //MyCombo("sex", sex, IM_ARRAYSIZE(sex), current_sex_idx);

            //// 체형
            //const char* body_types[] = { "fat", "no_fat" };
            //static int current_body_type_idx = 0;
            //MyCombo("body_type", body_types, IM_ARRAYSIZE(body_types), current_body_type_idx);

            //// 머리 스타일
            //const char* hair_styles[] = { "?", "?" };
            //static int current_hair_style_idx = 0;
            //MyCombo("hair_style", hair_styles, IM_ARRAYSIZE(hair_styles), current_hair_style_idx);

            //// 머리 색깔
            //// string input
            //static char hair_color_buffer[100];
            //ImGui::InputText("hair_color", hair_color_buffer, 100); 

            //// 표정
            //const char* look[] = { "smile", "sad" };
            //static int current_look_idx = 0;
            //MyCombo("look", look, IM_ARRAYSIZE(look), current_look_idx);

            //// 얼굴
            //const char* face[] = { "smile", "sad" };
            //static int current_face_idx = 0;
            //MyCombo("face", face, IM_ARRAYSIZE(face), current_face_idx);

            //// 의상
            //static char dress_text[512 * 16] = "dress input";
            //ImGui::InputTextMultiline("dress", dress_text, IM_ARRAYSIZE(dress_text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            //// 기타
            //static char etc_text[512 * 16] = "etc input";
            //ImGui::InputTextMultiline("etc", etc_text, IM_ARRAYSIZE(etc_text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            if (this->image != 0)
                onGenerate = false;

            if (ImGui::Button("Gen")) {
            /*    thread t3(&ImageGeneratorWindow::GenCharacter, this, ImageGenParams{ sd_api->models[current_model_idx], sd_api->loras[current_lora_idx],
                    string(input_prompt), string(input_nprompt), sd_api->samplers[current_sampler_idx],
                     stoi(input_steps), stoi(widths[current_width_idx]), stoi(heights[current_height_idx]),
                    true, stof(input_cfg), stoi(input_seed)});

                t3.detach();*/
                
                this->image = 0;
                onGenerate = true;

                this->GenCharacter({ sd_api->models[current_model_idx], sd_api->loras[current_lora_idx],
                    string(input_prompt), string(input_nprompt), sd_api->samplers[current_sampler_idx],
                     stoi(input_steps), stoi(widths[current_width_idx]), stoi(heights[current_height_idx]),
                    upsample, stof(input_cfg), stoi(input_seed), no_background, string(current_model_version[current_version_idx]) });
            }

            ImGui::EndTabItem();
        }

        if (onGenerate)
            this->progress = sd_api->getProgress();
        ImGui::ProgressBar(this->progress);

        ImGui::EndTabBar();
    }

    ImGui::End();
}