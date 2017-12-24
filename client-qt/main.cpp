#include "kart_application.h"

int main(int argc, char *argv[])
{
    KartApplication a(argc, argv);
    a.init();
    return a.exec();
}
