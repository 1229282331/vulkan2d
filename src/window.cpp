#include "window.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
const char* TITLE = "vulkan2D";

Window* Window::m_self_instance = nullptr;

void Window::init(int width, int height, const char *title)
{
    m_self_instance = new Window(width, height, title);
}

Window::Window(int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::destroy()
{
    delete m_self_instance;
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}




