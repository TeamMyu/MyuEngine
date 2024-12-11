#include "MyuEngine/Character.h"

Character::Character(std::string textureName, glm::vec2 position)
: position(position)
{
    std::filesystem::path texturesPath = std::filesystem::path(__FILE__).parent_path() / ".." / ".." / "resources" / "textures";

    this->texture = Texture2D((texturesPath / (textureName + ".png")).generic_string(), GL_RGBA, GL_RGBA);

    this->sprite = std::make_shared<Sprite>(
        this->texture,
        glm::vec3(position.x, position.y, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f)
    );
}

Character::Character(std::string textureName, float x, float y)
: Character(textureName, glm::vec2(x, y))
{
}
