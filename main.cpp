#include "nailab.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);			
	Nailab w;
	if(!w.Initialize())
		return 1;
	w.show();    
    return a.exec();
}
