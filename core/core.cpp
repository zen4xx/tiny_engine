#include "core.h"

Tiny_engine::Tiny_engine(){
    glfwInit();
}

Tiny_engine::~Tiny_engine(){
    glfwTerminate();
}

Window Tiny_engine::CreateWindow(int width, int height, const char* name){
    return Window(width, height, name);
}
