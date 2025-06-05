#include "core/core.h"

int main(){
    auto engine = Tiny_engine("app", 800, 600, "test");

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    glm::mat4 matrix = {};

    engine.addObject("test", vertices, indices, matrix);

    while(engine.isWindowOpen()){
        engine.update();     
    }

    return 0;
}
