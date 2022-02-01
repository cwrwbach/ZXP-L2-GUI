#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	QString cfg_file;
	QApplication a(argc, argv);
 
	MainWindow my_window(cfg_file);

	if (my_window.configOk)
        {
		my_window.show();
	    return a.exec();
	    }
	else
	    {
		return 1;
	    }
}
