
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

//const char* models[] = { "None", "Test", "Test", "Test" };
//static int current_model_idx = 0;
//MyCombo("model", models, IM_ARRAYSIZE(models), current_model_idx);

SDAPIs* sd_api;

ImageGeneratorWindow::ImageGeneratorWindow() {
    APIClient::Init("http://127.0.0.1");
    cout << APIClient::Get("InitAPI") << endl;
    sd_api = new SDAPIs();
}

void ImageGeneratorWindow::GenCharacter(ImageGenParams params) {
    cout << sd_api->setModel(params.model) << endl;

    stringstream ss;
    ss << params.prompt;
    ss << ", <" << params.lora << ":1>";

    string file = sd_api->txt2img(ss.str(), params.n_prompt, params.sampler, params.steps,
        params.width, params.height, params.upsacle, params.cfg, params.seed);

    ofstream ofs;
    ofs.open("../resources/out1.png", ios::out | ios::binary);
    ofs.write(file.c_str(), file.size());
    ofs.close();

    LoadTextureFromFile("../resources/out1.png", (GLuint*)&this->image, &this->width, &this->height);
    return;
}

void ImageGeneratorWindow::Draw() {
    
    ImGui::Begin("Image Gen");

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("ImageGenTab", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Image"))
        {
            ImGui::Text("size = %d x %d", this->width, this->height);
            if (this->image != 0) {
                ImGui::Image((void*)(intptr_t)this->image, ImVec2(this->width, this->height));
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Option"))
        {

            // model
            static int current_model_idx = 0;
            MyCombo("model", sd_api->models, current_model_idx);

            // sampler
            static int current_sampler_idx = 3;
            MyCombo("sampler", sd_api->samplers, current_sampler_idx);
            
            // lora
            static int current_lora_idx = 0;
            MyCombo("lora", sd_api->loras, current_lora_idx);

            // width
            const char* widths[] = { "512", "640", "768", "832", "896", "1024", "1080", "1152", "1216", "1344", "1536", "1920"};
            static int current_width_idx = 5;
            MyCombo("width", widths, IM_ARRAYSIZE(widths), current_width_idx);

            // height
            const char* heights[] = { "512", "640", "768", "832", "896", "1024", "1080", "1152", "1216", "1344", "1536", "1920" };
            static int current_height_idx = 5;
            MyCombo("height", heights, IM_ARRAYSIZE(heights), current_height_idx);

            // seed
            static char input_seed[32] = "-1";
            ImGui::InputText("random seed", input_seed, IM_ARRAYSIZE(input_seed));

            // step
            static char input_steps[32] = "28";
            ImGui::InputText("sampling step", input_steps, IM_ARRAYSIZE(input_steps));

            // cfg
            static char input_cfg[32] = "7.0";
            ImGui::InputText("cfg scale", input_cfg, IM_ARRAYSIZE(input_cfg));

            // positive
            static char input_prompt[512 * 16] = "score_9,score_8_up,score_7_up,source_anime,anime, head-tilt, (spoken question mark):1.2, (simple background):1.2, ((masterpiece,best quality)), 1girl, hanami ume, kindergarten uniform, blush, standing";
            ImGui::InputTextMultiline("positive prompt", input_prompt, IM_ARRAYSIZE(input_prompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            // negative
            static char input_nprompt[512 * 16] = "score_6,score_5,score_4,(worst quality, low quality:1.4), lowres, (bad), text, error, fewer, extra, missing, worst quality, jpeg artifacts, low quality, watermark, unfinished, displeasing, oldest, early, chromatic aberration, signature, extra digits, artistic error, username, scan, [abstract], censored, copyright name, malformed hands, poorly drawn hands, mutated fingers, extra limbs, poorly drawn face, bad leg, strange leg, nsfw, ugly, out of frame, bad hand, bad feet, 3d, cropped, siblings, clones, monochrome, sketch, rought lines, patreon username, logo, bad proportions, artist signature, artist name, manmilk, sperm, cum";
            ImGui::InputTextMultiline("negative prompt", input_nprompt, IM_ARRAYSIZE(input_nprompt), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);

            // progress
            ImGui::SliderFloat("progress", &sd_api->progress, 0.0f, 1.0f);

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
                this->GenCharacter({ sd_api->models[current_model_idx], sd_api->loras[current_lora_idx],
                    string(input_prompt), string(input_nprompt), sd_api->samplers[current_sampler_idx],
                     stoi(input_steps), stoi(widths[current_width_idx]), stoi(heights[current_height_idx]),
                    true, stof(input_cfg), stoi(input_seed)});
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    
}