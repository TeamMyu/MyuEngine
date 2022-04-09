#pragma once

#include "../Myu/RenderObject.hpp"

class EditorApplication : public RenderObject
{
    EditorApplication();
    virtual ~EditorApplication() override;

    virtual void OnUpdate();
    virtual void OnBind();
    virtual void OnDrawStart();
    virtual void OnDrawEnd();
    virtual void OnEvent();
};
