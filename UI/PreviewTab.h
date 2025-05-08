#ifndef PREVIEWTAB_H
#define PREVIEWTAB_H

#include <QWidget>

class MainWindow;

class PreviewTab : public QWidget {
    Q_OBJECT
public:
    explicit PreviewTab(MainWindow* mainWindow, QWidget* parent = nullptr);

private:
    void setupUI();

    MainWindow* mainWindow;
};

#endif // PREVIEWTAB_H 