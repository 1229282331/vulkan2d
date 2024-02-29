#include "window.hpp"
#include "vkBase.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;
const char* TITLE = "vulkan2D";

Window* Window::m_self_instance = nullptr;

void windowResizedCallback(GLFWwindow* window, int width, int height)
{
    while(width==0 || height==0)
    {
        glfwGetWindowSize(Window::self().window, &width, &height);
        glfwWaitEvents();
    }
    vulkan2d::VkBase::self().recreateSwapchain();
}

void Window::init(int width, int height, const char *title)
{
    m_self_instance = new Window(width, height, title);
}

Window::Window(int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowSizeCallback(window, windowResizedCallback);
}

void Window::destroy()
{
    delete m_self_instance;
}

void Window::titleFPS() 
{
    static double time0 = glfwGetTime();
    static double time1;
    static double dt;
    static int dframe = 0;
    static std::stringstream info;
    time1 = glfwGetTime();
    dframe++;
    if ((dt = time1 - time0) >= 1) 
    {
        info.precision(1);  /*set 1bit precision*/
        info << "vulkan2D" << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(window, info.str().c_str());
        info.str("");   //别忘了在设置完窗口标题后清空所用的stringstream
        time0 = time1;
        dframe = 0;
    }
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}




