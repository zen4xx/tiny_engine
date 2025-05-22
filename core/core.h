#include "../renderer/renderer.h"
#include "../window/window.h"
#include <memory>

class Tiny_engine{
    public:
        Tiny_engine(const char* application_name);
        ~Tiny_engine();

    public:
        Window CreateWindow(int width, int height, const char* name);
    private:
        std::unique_ptr<Renderer> m_renderer; 
};
