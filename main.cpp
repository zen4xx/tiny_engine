#include "core/core.h"

int main(){
    auto engine = Tiny_engine("app");
    auto window = engine.CreateWindow(800, 600, "window");

    while(!glfwWindowShouldClose(window.GetWindow())){
        glfwPollEvents();
    }

    return 0;
}
