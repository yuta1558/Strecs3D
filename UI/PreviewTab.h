#ifndef PREVIEWTAB_H
#define PREVIEWTAB_H

#include <QWidget>
#include <QComboBox>

class MainWindow;

class PreviewTab : public QWidget {
    Q_OBJECT
public:
    explicit PreviewTab(MainWindow* mainWindow, QWidget* parent = nullptr);
    QComboBox* getModeComboBox() const { return modeComboBox; }

private:
    void setupUI();

    MainWindow* mainWindow;
    QComboBox* modeComboBox;
};

#endif // PREVIEWTAB_H 