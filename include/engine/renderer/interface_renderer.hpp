#pragma once

class IRenderer
{
private:
    unsigned VBO, VAO;

public:
    virtual void init() = 0;
    virtual void render() = 0;
    virtual void cleanup() = 0;
    virtual ~IRenderer() = default;
};
