#include "pch.h"
#include "GlfwInstance.h"

GlfwInstance::GlfwInstance(int32_t width, int32_t height)
    : m_width(width), m_height(height)
{
}

GlfwInstance::~GlfwInstance()
{
    if (m_initialized)
    {
        if (m_window)
        {
            glfwDestroyWindow(m_window);
        }

        glfwTerminate();
    }
}

int GlfwInstance::windowShouldClose() const
{
    if (m_initialized && m_window)
    {
        return glfwWindowShouldClose(m_window);
    }

    throw std::exception("Attempt to use uninitialized GLFW Instance");
}

void GlfwInstance::init()
{
    glfwInit();
    // Disables OpenGL context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
}
