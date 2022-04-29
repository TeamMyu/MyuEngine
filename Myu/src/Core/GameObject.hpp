#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

#include "Model.hpp"

namespace Myu
{
    struct Transform
    {
        glm::vec3 position = glm::vec3(0, 0, 0);
        glm::vec3 rotation = glm::vec3(0, 0, 0);
        glm::vec3 scale = glm::vec3(1, 1, 1);
        
        glm::mat4 toMat4();
    };

    class GameObject
    {
    public:
        using id_t = unsigned int;
        
        static GameObject createGameObject()
        {
            static id_t id = 0;
            return GameObject{id++};
        }
        
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;
        
        id_t getID() { return m_id; }
        
        
        Transform transform;
        
        std::shared_ptr<Model> model;
        
        std::vector<VkDescriptorSet> descriptorSets;
    private:
        id_t m_id;
        
        GameObject(id_t id) : m_id{id} {}
    };

}
