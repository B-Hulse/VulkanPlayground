#pragma once
class GlfwInstance
{
public:
    GlfwInstance(int32_t width, int32_t height);
    ~GlfwInstance();

    int windowShouldClose() const;

    void init();
private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;

    int32_t m_width;
    int32_t m_height;
};

