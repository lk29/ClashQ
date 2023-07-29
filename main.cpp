#include "Application.h"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    Application::mainWindow().show();
    return a.exec();
}
