#include "core.h"

Tiny_engine::Tiny_engine(const char* applictaion_name) 
            :m_renderer(std::make_unique<Renderer>(applictaion_name)){
    glfwInit();
}

Tiny_engine::~Tiny_engine(){
    glfwTerminate();
}

Window Tiny_engine::CreateWindow(int width, int height, const char* name){
    return Window(width, height, name);
}
