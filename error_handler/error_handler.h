#ifndef ERR_HANDLER_H
#define ERR_HANDLER_H

#include <stdexcept>
#include <fstream>
#include <iostream>
#ifdef _WIN32
#include <string>
#endif

void err(const char* msg, int err_code);

#endif
