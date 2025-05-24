#include "core/core.h"

int main(){
    auto engine = Tiny_engine("app", 800, 600, "test");

    while(engine.isWindowOpen()){
        engine.update();
    }

    return 0;
}
