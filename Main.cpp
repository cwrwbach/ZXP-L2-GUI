#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	QString cfg_file;
	bool edit_conf = false;

	QApplication a(argc, argv);
 
	MainWindow my_window(cfg_file, edit_conf);

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
