#include "error_handler.h"

void err(const char* msg){
    std::ofstream log("log", std::ios::app);
    log << msg << "\n";
    log.close();
    throw std::runtime_error(msg);
}