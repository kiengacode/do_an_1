// main.cpp — Entry point

#include "gui/MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Hamming (7,4) AWGN Simulator");
    app.setOrganizationName("DSP Lab");

    MainWindow win;
    win.show();

    return app.exec();
}
