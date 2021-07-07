#include "QMuPDFReader.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMuPDFReader w;
    w.show();
    return a.exec();
}
