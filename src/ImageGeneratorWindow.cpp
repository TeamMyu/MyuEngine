
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

// #include <Rpc.h>
// #pragma comment(lib, "Rpcrt4.lib")

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
        ImGuiFileDialog::Instance()->OpenDialog(title, "choose file", ".png,.jpg,.bmp,.jpeg,.webp", config);
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
            image_component.path = filepathname;
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

StableDiffusionAPI stableDiffusion;

ImageGeneratorWindow::ImageGeneratorWindow() {

}

//void ImageGeneratorWindow::GenCharacter(PoseT2iParams params) {
//
//    cout << sd_api->setModel(params.model) << endl;
//
//    stringstream ss;
//    std::function<void(std::string)> callback = [this](std::string file) {
//        while (!queueMutex.try_lock());
//        genQueue.push(file);
//        queueMutex.unlock();
//    };
//
//    PoseT2iParams params;
//    params.workflowPath = "PoseT2i.json";
//    params.positivePrompt = "1 girl, no background";
//    params.negativePrompt = "";
//    params.width = "832";
//    params.height = "1216";
//    params.posePath = "pose1.png";
//    stableDiffusion.callSD(params);
//
//
//
//    unordered_map<string, string> id2class;
//    string k_sampler;
//
//    std::ifstream workflow("workflow.json");
//    json prompt;
//    workflow >> prompt;
//
//    for (auto& el : prompt.items()) {
//        std::cout << el.key() << " : " << el.value()["class_type"] << std::endl;
//        id2class.insert({ el.value()["class_type"],  el.key() });
//
//    }
//
//    auto inputs = prompt[id2class["KSampler"]]["inputs"];
//    auto seed = inputs["seed"];
//    auto posText = inputs["positive"];
//    std::cout << seed << std::endl;
//    std::cout << posText << std::endl;
//    //k_sampler = j3[id2class["KSampler"]];
//
//    UUID uuid;
//    UuidCreate(&uuid);
//
//    char* client_id = nullptr;
//    UuidToStringA(&uuid, (RPC_CSTR*)&client_id);
//
//    json params;
//    params["prompt"] = prompt;
//    params["client_id"] = client_id;
//
//    json result;
//    result = json::parse(APIClient::Post("prompt", params.dump()));
//    string prompt_id = result["prompt_id"];
//    std::cout << prompt_id << std::endl;
//
//    APIClient::test(client_id, prompt_id, callback);
//
//    return;
//}

void ImageGeneratorWindow::Draw() {
    ImGui::Begin("Image Gen");
    ImGui::SetWindowSize(ImVec2(1920, 1080));

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("ImageGenTab", tab_bar_flags))
    {
        static bool onGenerate = false;

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

        if (ImGui::BeginTabItem(u8"캐릭터 생성"))
        {
            ImGui::Text(u8"긍정 프롬프트");
            static char posPrompt[512 * 6] = "";
            ImGui::InputTextMultiline("posPrompt", posPrompt, IM_ARRAYSIZE(posPrompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 6), 0);

            ImGui::Text(u8"부정 프롬프트");
            static char negPrompt[512 * 6] = "";
            ImGui::InputTextMultiline("negPrompt", negPrompt, IM_ARRAYSIZE(negPrompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 6), 0);

            static int denoise = 0;
            ImGui::SetNextItemWidth(150);
            ImGui::DragInt(u8"원본 유지 강도", &denoise, 1.f, 0, 100, "%d %%");
            ImGui::SameLine(275);

            static vector<string> widths = { "512","768","832","896","1024" ,"1152" ,"1216" ,"1344" ,"1536" };
            static int widths_idx = 0;
            ImGui::SetNextItemWidth(150);
            MyCombo("width", widths, widths_idx);

            ImGui::SameLine(500);

            static vector<string> heights = { "512","768","832","896","1024" ,"1152" ,"1216" ,"1344" ,"1536" };
            static int heights_idx = 0;
            ImGui::SetNextItemWidth(150);
            MyCombo("height", heights, heights_idx);

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

                    std::cout << filepath << std::endl;
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

            ImGui::EndGroup();

            ImGui::SameLine();

            ImageDialog(base_image, 98, u8"베이스 이미지", 512.f, 512.f);

            ImGui::SameLine();

            reference_images.emplace_back();
            ImageDialog(reference_images[0], 99, u8"참고할 포즈", 256.f, 256.f);

            ImGui::SameLine();

            reference_images.emplace_back();
            ImageDialog(reference_images[1], 100, u8"참고할 얼굴", 256.f, 256.f);

            ImGui::SameLine();

            reference_images.emplace_back();
            ImageDialog(reference_images[2], 101, u8"참고할 이미지", 256.f, 256.f);

            if (ImGui::Button(u8"캐릭터 생성(포즈)")) {
                PoseT2iParams params;
                params.workflowPath = "workflows\\PoseT2i.json";
                params.positivePrompt = string(posPrompt);
                params.negativePrompt = string(negPrompt);
                params.width = stoi(widths[widths_idx]);
                params.height = stoi(heights[heights_idx]);

                params.posePath = reference_images[0].path;

                stableDiffusion.callSD(params);
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"손 수정")) {
                HandsRedrawParams params;
                params.region.prompt.workflowPath = "workflows\\HandsRedraw.json";
                params.region.prompt.positivePrompt = string(posPrompt);
                params.region.prompt.negativePrompt = string(negPrompt);
                params.region.prompt.width = stoi(widths[widths_idx]);
                params.region.prompt.height = stoi(heights[heights_idx]);
                params.region.prompt.posePath = reference_images[0].path;

                params.region.imagePath = base_image.path;
                params.region.redrawRegion = string("hands");
                params.region.strength = (100.f - denoise) / 100.f;

                stableDiffusion.callSD(params);
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"얼굴 수정(레퍼런스 페이스)")) {
                FaceRefRedrawParams params;
                params.region.prompt.workflowPath = "workflows\\FaceRedraw.json";
                params.region.prompt.positivePrompt = string(posPrompt);
                params.region.prompt.negativePrompt = string(negPrompt);
                params.region.prompt.width = stoi(widths[widths_idx]);
                params.region.prompt.height = stoi(heights[heights_idx]);
                params.region.prompt.posePath = reference_images[0].path;

                params.region.imagePath = base_image.path;
                params.region.redrawRegion = string("a face");
                params.region.strength = (100.f - denoise) / 100.f;

                params.referencePath = reference_images[1].path;
                params.referenceRegion = string("a face");

                stableDiffusion.callSD(params);
            }

            static char regionPrompt[512];
            ImGui::SetNextItemWidth(200);
            ImGui::InputText(u8"수정 영역", regionPrompt, IM_ARRAYSIZE(regionPrompt));

            ImGui::SameLine();

            if (ImGui::Button(u8"영역 수정(베이스, 포즈)")) {
                RegionRedrawParams params;
                params.prompt.workflowPath = "workflows\\RegRedraw.json";
                params.prompt.positivePrompt = string(posPrompt);
                params.prompt.negativePrompt = string(negPrompt);
                params.prompt.width = stoi(widths[widths_idx]);
                params.prompt.height = stoi(heights[heights_idx]);
                params.prompt.posePath = reference_images[0].path;

                params.multiRegion = false;

                params.imagePath = base_image.path;
                params.redrawRegion = string(regionPrompt);
                params.strength = (100.f - denoise) / 100.f;

                stableDiffusion.callSD(params);
            }

            static char referencePrompt[512];
            ImGui::SetNextItemWidth(200);
            ImGui::InputText(u8"참고 영역", referencePrompt, IM_ARRAYSIZE(referencePrompt));

            ImGui::SameLine();

            if (ImGui::Button(u8"영역 참고 수정(베이스, 포즈, 레퍼런스)")) {
                RefRedrawParams params;
                params.region.prompt.workflowPath = "workflows\\RefRedraw.json";
                params.region.prompt.positivePrompt = string(posPrompt);
                params.region.prompt.negativePrompt = string(negPrompt);
                params.region.prompt.width = stoi(widths[widths_idx]);
                params.region.prompt.height = stoi(heights[heights_idx]);
                params.region.prompt.posePath = reference_images[0].path;

                params.region.imagePath = base_image.path;
                params.region.redrawRegion = string(regionPrompt);
                params.region.strength = (100.f - denoise) / 100.f;

                params.referencePath = reference_images[2].path;
                params.referenceRegion = string(referencePrompt);
                stableDiffusion.callSD(params);
            }

            if (ImGui::Button(u8"일러스트 생성")) {
                T2iParams params;
                params.workflowPath = "workflows\\T2i.json";
                params.positivePrompt = string(posPrompt);
                params.negativePrompt = string(negPrompt);
                params.width = stoi(widths[widths_idx]);
                params.height = stoi(heights[heights_idx]);

                stableDiffusion.callSD(params);
            }


            ImGui::EndTabItem();
        }

        ImGui::ProgressBar(stableDiffusion.Progress());

        ImGui::EndTabBar();
    }

    ImGui::End();
}