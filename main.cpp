#include "core/core.h"

int main(){
    auto engine = Tiny_engine("app", 800, 600, "test");
    std::vector<Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    std::vector<Vertex> vertices2 = {
            {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
            {{1.0f, 0.5f}, {1.0f, 1.0f, 1.0f}},
            {{0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    glm::mat4 matrix = {};
    engine.addObject(vertices, matrix);
    engine.addObject(vertices2, matrix);

    while(engine.isWindowOpen()){
        engine.update();     
    }

    return 0;
}
