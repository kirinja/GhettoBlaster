#include "mainwindow.h"
#include <QApplication>
#include "fluidsynth.h"


/*
 * Crashes in Debug but works normally in Release. Need to check on QString (ASSERT: "uint(i) < uint(size())" in file D:/Software/Qt/5.9.1/mingw53_32/include/QtCore/qstring.h, line 887)
 * Problem is most likely string convertion when opening a file
 *
 * Need to break out the code into seperate classes for easier maintainability
 */
int main(int argc, char *argv[])
{


    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("icon.ico")); // look it up
    MainWindow w;
    w.show();

    return a.exec();
}
