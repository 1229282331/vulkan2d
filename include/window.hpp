#pragma once
#include "GLFW/glfw3.h"
#include "utils.hpp"

extern const int WIDTH;
extern const int HEIGHT;
extern const char* TITLE;

class Window{
public:
    static Window& self() { return *m_self_instance; }
    static void init(int width=WIDTH, int height=HEIGHT, const char *title=TITLE);
    static void destroy();

    GLFWwindow *window;

private:
    Window(int width=WIDTH, int height=HEIGHT, const char *title=TITLE);
    ~Window();

    static Window* m_self_instance;

};