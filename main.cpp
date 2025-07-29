#include <QApplication>
#include "mainwindow.h"
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // アプリケーションアイコンを設定
    app.setWindowIcon(QIcon(":/resources/strecs_icon.png"));

    // QVTKOpenGLNativeWidgetで使用するデフォルトのSurfaceFormatを設定
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    MainWindow window;
    window.show();

    return app.exec();
}
