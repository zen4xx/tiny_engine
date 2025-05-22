#include <iostream>
#include "../window/window.h"

class Tiny_engine{
    public:
        Tiny_engine();
        ~Tiny_engine();

    public:
        Window CreateWindow(int width, int height, const char* name);
    private:
};
