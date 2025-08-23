#include "error_handler.h"
#include <stdexcept>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <string>
#endif

void err(const char* msg, int err_code){
    std::string full_msg = msg + (std::string)" with code " + std::to_string(err_code);
    std::ofstream log("log", std::ios::app);
    log << full_msg << "\n";
    log.close();
    throw std::runtime_error(full_msg);
}