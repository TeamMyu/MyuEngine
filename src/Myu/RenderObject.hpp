#pragma once


class RenderObject
{
public:
    RenderObject();
    virtual ~RenderObject();

    virtual void OnUpdate() {}
    virtual void OnBind() {}
    virtual void OnDrawStart() {}
    virtual void OnDrawEnd() {}
    virtual void OnEvent() {}
};

