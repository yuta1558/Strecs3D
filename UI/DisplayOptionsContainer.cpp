#include "DisplayOptionsContainer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

DisplayOptionsContainer::DisplayOptionsContainer(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyle();
}

void DisplayOptionsContainer::setupUI()
{
    // メインレイアウト
    QVBoxLayout* containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(10, 10, 10, 10);
    containerLayout->setSpacing(10);

    // 各表示オプションウィジェットを作成
    stlDisplayWidget = new ObjectDisplayOptionsWidget("No STL file selected", this);
    vtkDisplayWidget = new ObjectDisplayOptionsWidget("No VTK file selected", this);
    dividedMeshWidget1 = new ObjectDisplayOptionsWidget("Divided Mesh 1", this);
    dividedMeshWidget2 = new ObjectDisplayOptionsWidget("Divided Mesh 2", this);
    dividedMeshWidget3 = new ObjectDisplayOptionsWidget("Divided Mesh 3", this);
    dividedMeshWidget4 = new ObjectDisplayOptionsWidget("Divided Mesh 4", this);

    // レイアウトに追加
    containerLayout->addWidget(stlDisplayWidget);
    containerLayout->addWidget(vtkDisplayWidget);
    containerLayout->addWidget(dividedMeshWidget1);
    containerLayout->addWidget(dividedMeshWidget2);
    containerLayout->addWidget(dividedMeshWidget3);
    containerLayout->addWidget(dividedMeshWidget4);
    containerLayout->addStretch(); // 下部にスペースを追加
}

void DisplayOptionsContainer::setupStyle()
{
    // コンテナ全体のスタイル
    setStyleSheet(
        "DisplayOptionsContainer {"
        "    background-color: rgba(45, 45, 45, 200);"
        "    border-radius: 10px;"
        "    border: 1px solid rgba(255, 255, 255, 0.1);"
        "}"
        "QScrollArea {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: rgba(60, 60, 60, 100);"
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: rgba(120, 120, 120, 150);"
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: rgba(150, 150, 150, 200);"
        "}"
    );
}

QList<ObjectDisplayOptionsWidget*> DisplayOptionsContainer::getAllDisplayWidgets() const
{
    QList<ObjectDisplayOptionsWidget*> widgets;
    widgets.append(stlDisplayWidget);
    widgets.append(vtkDisplayWidget);
    widgets.append(dividedMeshWidget1);
    widgets.append(dividedMeshWidget2);
    widgets.append(dividedMeshWidget3);
    widgets.append(dividedMeshWidget4);
    return widgets;
} 