
#include "ImageGeneratorWindow.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

//#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <time.h>
#include <thread>
using std::thread;
using namespace std;

#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

#include "../vendor/ImGuiFileDialog/ImGuiFileDialog.h"


bool ImageComponent::FromFile(const char* filename)
{
    // Load from file
    unsigned char* image_data = stbi_load(filename, &this->width, &this->height, NULL, 4);

    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    glGenTextures(1, &this->handle);
    glBindTexture(GL_TEXTURE_2D, this->handle);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
    return true;
}

bool ImageComponent::FromMemory(std::string file)
{
    // Load from file
    const unsigned char* uchar_str = reinterpret_cast<const unsigned char*>(file.c_str());
    unsigned char* image_data = stbi_load_from_memory(uchar_str, file.size(), &this->width, &this->height, NULL, 4);

    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    glGenTextures(1, &this->handle);
    glBindTexture(GL_TEXTURE_2D, this->handle);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);
    return true;
}

void ImageComponent::ToFile(const char* filename) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->handle, 0);

    int data_size = this->width * this->height * 4;
    GLubyte* pixels = new GLubyte[this->width * this->height * 4];
    glReadPixels(0, 0, this->width, this->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    stbi_write_png(filename, this->width, this->height, 4, pixels, 0);
}

std::string ImageComponent::Data() {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->handle, 0);

    int data_size = this->width * this->height * 4;
    GLubyte* pixels = new GLubyte[this->width * this->height * 4];
    glReadPixels(0, 0, this->width, this->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    unsigned char* image;
    int length;
    image = stbi_write_png_to_mem(pixels, 0, this->width, this->height, 4, &length);

    string result((char*)image, length);
    return result;
}

void ImageComponent::Draw(float x, float y, float pad = 24.f) {
    if (this->handle != 0) {
        float drawWidth = this->width;
        float drawHeight = this->height;
        float ratio = this->width / this->height;

        float scaleFactor = 1.f;

        if (this->fitFrame == FITSTYLE::WIDTH_AND_HEIGHT) {
            if (drawWidth > drawHeight) {
                scaleFactor = ((x - drawWidth) / drawWidth) + 1;
            }
            else {
                scaleFactor = ((y - drawHeight) / drawHeight) + 1;
            }

            drawWidth *= scaleFactor;
            drawHeight *= scaleFactor;
        }
        else if (this->fitFrame == FITSTYLE::WIDTH_OR_HEIGHT) {
            if (drawWidth > drawHeight) {
                scaleFactor = ((y - drawHeight) / drawHeight) + 1;
            }
            else {
                scaleFactor = ((x - drawWidth) / drawWidth) + 1;
            }

            drawWidth *= scaleFactor;
            drawHeight *= scaleFactor;
        }

        if (drawWidth < x)
            ImGui::SetCursorPos({ ImGui::GetCursorPos().x + (x / 2.f - drawWidth / 2.f), ImGui::GetCursorPos().y });
        
        if (drawHeight < y)
            ImGui::SetCursorPos({ ImGui::GetCursorPos().x , ImGui::GetCursorPos().y + (y / 2.f - drawHeight / 2.f) });
                  
        drawWidth -= pad;
        drawHeight -= pad;

        ImGui::Image((void*)(intptr_t)this->handle, ImVec2(drawWidth, drawHeight));
    }
}


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

void ImageDialog(ImageComponent& image_component, int id, const char* title, float sizeX, float sizeY) {
    ImGui::BeginGroup();

    ImGui::Text("[%s] %d x %d", title, image_component.width, image_component.height);
    ImGui::SameLine();

    ImGui::PushID(id);
    if (ImGui::Button("open file")) {
        
        std::locale::global(std::locale("ko_KR.UTF-8"));

        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog(title, "choose file", ".png,.jpg,.bmp,.jpeg", config);
    }
    ImGui::PopID();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(45, 45, 45, 255));
    ImGui::BeginChildFrame(id, { sizeX, sizeY }, ImGuiWindowFlags_HorizontalScrollbar);

    image_component.Draw(sizeX, sizeY);

    ImGui::EndChildFrame();
    ImGui::PopStyleColor();
    ImGui::EndGroup();

    if (ImGuiFileDialog::Instance()->Display(title)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

            image_component.FromFile(filepathname.c_str());
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

StableDiffusionAPI stableDiffusion;

ImageGeneratorWindow::ImageGeneratorWindow() {
    
}

void ImageGeneratorWindow::GenCharacter() {
    
    //cout << sd_api->setModel(params.model) << endl;
    //
    //stringstream ss;
    //std::function<void(std::string)> callback = [this](std::string file) {
    //    while (!queueMutex.try_lock());
    //    genQueue.push(file);
    //    queueMutex.unlock();
    //};

    stableDiffusion.makeCharacter({ "workflow.json","","" });

    //unordered_map<string, string> id2class;
    //string k_sampler;

    //std::ifstream workflow("workflow.json");
    //json prompt;
    //workflow >> prompt;

    //for (auto& el : prompt.items()) {
    //    std::cout << el.key() << " : " << el.value()["class_type"] << std::endl;
    //    id2class.insert({ el.value()["class_type"],  el.key() });

    //}

    //auto inputs = prompt[id2class["KSampler"]]["inputs"];
    //auto seed = inputs["seed"];
    //auto posText = inputs["positive"];
    //std::cout << seed << std::endl;
    //std::cout << posText << std::endl;
    ////k_sampler = j3[id2class["KSampler"]];

    //UUID uuid;
    //UuidCreate(&uuid);

    //char* client_id = nullptr;
    //UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

    //json params;
    //params["prompt"] = prompt;
    //params["client_id"] = client_id;

    //json result;
    //result = json::parse(APIClient::Post("prompt", params.dump()));
    //string prompt_id = result["prompt_id"];
    //std::cout << prompt_id << std::endl;

    //APIClient::test(client_id, prompt_id, callback);

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
                result_image.FromMemory(file);

                onGenerate = false;
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
            
            ImGui::BeginGroup();
            ImGui::Text("[result] %d x %d", result_image.width, result_image.height);
            ImGui::SameLine();

            if (ImGui::Button("save file")) {
                std::locale::global(std::locale("ko_KR.UTF-8"));

                IGFD::FileDialogConfig config;
                config.path = ".";
                config.fileName = "saved.png";

                ImGuiFileDialog::Instance()->OpenDialog("filedialog", "choose file", ".png,.jpg,.bmp,.jpeg", config);
            }

            ImGui::SameLine();

            if (ImGui::Button("move to base")) {
                base_image = result_image;
                result_image.handle = 0;
            }

            if (ImGuiFileDialog::Instance()->Display("filedialog")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string filepathname = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filepath = ImGuiFileDialog::Instance()->GetCurrentPath();

                    std::cout << filepathname << std::endl;
                    result_image.ToFile(filepathname.c_str());
                }

                // close
                ImGuiFileDialog::Instance()->Close();
            }

            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(45, 45, 45, 255));
            ImGui::BeginChildFrame(1, { 512, 512 }, ImGuiWindowFlags_HorizontalScrollbar);

            result_image.Draw(512, 512);

            ImGui::EndChildFrame();
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::EndGroup();

            ImGui::SameLine();
  
            ImageDialog(base_image, 98, "base image", 512.f, 512.f);
            
            reference_images.emplace_back();
            ImageDialog(reference_images[0], 99, "face reference", 256.f, 256.f);

            ImGui::SameLine();

            reference_images.emplace_back();
            ImageDialog(reference_images[1], 100, "clothes reference", 256.f, 256.f);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Advanced")) 
        {
            /*MyCombo("version", current_model_version, IM_ARRAYSIZE(current_model_version), current_version_idx);
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
            ImGui::InputTextMultiline("negative prompt", input_nprompt, IM_ARRAYSIZE(input_nprompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);*/

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

            if (ImGui::Button("Gen")) {
            /*    thread t3(&ImageGeneratorWindow::GenCharacter, this, ImageGenParams{ sd_api->models[current_model_idx], sd_api->loras[current_lora_idx],
                    string(input_prompt), string(input_nprompt), sd_api->samplers[current_sampler_idx],
                     stoi(input_steps), stoi(widths[current_width_idx]), stoi(heights[current_height_idx]),
                    true, stof(input_cfg), stoi(input_seed)});

                t3.detach();*/
                
                onGenerate = true;
                this->GenCharacter();
                /*this->GenCharacter({ sd_api->models[current_model_idx], sd_api->loras[current_lora_idx],
                    string(input_prompt), string(input_nprompt), sd_api->samplers[current_sampler_idx],
                    "", 0.4f,
                     stoi(input_steps), stoi(widths[current_width_idx]), stoi(heights[current_height_idx]),
                    upsample, stof(input_cfg), stoi(input_seed), no_background, string(current_model_version[current_version_idx]) });*/

        /*        SDAPIs::ImageGenParams params;
                params.version = string(current_model_version[current_version_idx]);
                params.model = sd_api->models[current_model_idx];
                params.lora = sd_api->loras[current_lora_idx];
                params.prompt = string(input_prompt);
                params.n_prompt = string(input_nprompt);
                params.width = stoi(widths[current_width_idx]);
                params.height = stoi(heights[current_height_idx]);           
                params.sampler = sd_api->samplers[current_sampler_idx];
                params.seed = stoi(input_seed);
                params.steps = stoi(input_steps);
                params.cfg = stof(input_cfg);
                params.noBackground = no_background;
                params.upsacle = upsample;

                params.image = base_image.Data();
                params.denoise = .4f;

                this->GenCharacter(params);*/
            }
            
            ImGui::EndTabItem();
        }

   /*     if (onGenerate)
            this->progress = sd_api->getProgress();
        ImGui::ProgressBar(this->progress);*/

        ImGui::EndTabBar();
    }

    ImGui::End();
}