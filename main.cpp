#include <QApplication>
#include "mainwindow.h"
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // QVTKOpenGLNativeWidgetで使用するデフォルトのSurfaceFormatを設定
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    MainWindow window;
    window.show();

    return app.exec();
}
