#include "GL/Texture2D.h"
#include "GL/Sprite.h"
#include <memory>

#include <string>
#include <filesystem>

#include <glm/glm.hpp>

class Character
{
public:

    Character(std::string textureName, glm::vec2 position);

    Character(std::string textureName, float x, float y);

    glm::vec2 getPosition()
    {
        return this->position;
    }

    void setPosition(int x, int y)
    {
        this->position = glm::ivec2(x, y);
        sprite->setPosition(glm::vec3(x, y, 0.0f));
    }

    //get sprite
    std::shared_ptr<Sprite> getSprite()
    {
        return this->sprite;
    }

    Texture2D& getTexture()
    {
        return this->texture;
    }

private:
    glm::vec2 position;
    std::shared_ptr<Sprite> sprite;
    Texture2D texture;
};