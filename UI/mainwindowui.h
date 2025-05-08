#ifndef MAINWINDOWUI_H
#define MAINWINDOWUI_H

#include <QWidget>
#include <QTabWidget>
#include "ImportTab.h"
#include "SettingsTab.h"
#include "PreviewTab.h"

class MainWindow;

class MainWindowUI {
public:
    explicit MainWindowUI(MainWindow* mainWindow);
    ~MainWindowUI() = default;

    void setupUI();
    QWidget* getCentralWidget() const { return centralWidget; }
    ImportTab* getImportTab() const { return importTab; }
    SettingsTab* getSettingsTab() const { return settingsTab; }
    PreviewTab* getPreviewTab() const { return previewTab; }

private:
    void setupTabWidget();
    void setupStyle();

    MainWindow* mainWindow;
    QWidget* centralWidget;
    QTabWidget* tabWidget;
    ImportTab* importTab;
    SettingsTab* settingsTab;
    PreviewTab* previewTab;
};

#endif // MAINWINDOWUI_H 