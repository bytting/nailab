#include <exception>
#include <cstdio>
#include "exceptions.h"
#include "nailab.h"
#include <windows.h>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    int retVal = 0;
    try
    {
        QApplication a(argc, argv);
        Nailab w;
        if(!w.Initialize())
            return 1;
        w.show();
        retVal = a.exec();
    }
    catch(BaseException& bex)
    {
        retVal = 1;
        char msg[1024];
        sprintf(msg, "%s - %s: line %d\n\n%s", bex.file(), bex.function(), bex.line(), bex.what());
        MessageBoxA(NULL, msg, "Error", MB_ICONERROR);
    }
    catch(std::exception& ex)
    {
        retVal = 1;
        MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
    }

    return retVal;
}
