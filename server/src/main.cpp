#include <stdlib.h>
//#include <stdio.h>

#include "rpi_server.h"

#define LOGURU_IMPLEMENTATION 1
#include <loguru.hpp>

RpiServer g_server(8080);

void atexit_handler()
{
    LOG_F(INFO, "atexit_handler()");
    g_server.shutdown();
}

int main(int argc, char* argv[])
{
    loguru::init(argc, argv);
    loguru::add_file("run.log", loguru::Truncate, loguru::Verbosity_INFO);

    atexit(atexit_handler);

    g_server.startup();

    while(g_server.update())
    {
    }

    return 0;
}