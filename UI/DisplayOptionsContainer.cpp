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

    // スクロールエリアを作成
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // スクロールエリア内のコンテンツウィジェット
    scrollContent = new QWidget();
    mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(8);

    // 各表示オプションウィジェットを作成
    stlDisplayWidget = new ObjectDisplayOptionsWidget("No STL file selected", scrollContent);
    vtkDisplayWidget = new ObjectDisplayOptionsWidget("No VTK file selected", scrollContent);
    dividedMeshWidget1 = new ObjectDisplayOptionsWidget("Divided Mesh 1", scrollContent);
    dividedMeshWidget2 = new ObjectDisplayOptionsWidget("Divided Mesh 2", scrollContent);
    dividedMeshWidget3 = new ObjectDisplayOptionsWidget("Divided Mesh 3", scrollContent);
    dividedMeshWidget4 = new ObjectDisplayOptionsWidget("Divided Mesh 4", scrollContent);

    // レイアウトに追加
    mainLayout->addWidget(stlDisplayWidget);
    mainLayout->addWidget(vtkDisplayWidget);
    mainLayout->addWidget(dividedMeshWidget1);
    mainLayout->addWidget(dividedMeshWidget2);
    mainLayout->addWidget(dividedMeshWidget3);
    mainLayout->addWidget(dividedMeshWidget4);
    mainLayout->addStretch(); // 下部にスペースを追加

    // スクロールエリアにコンテンツを設定
    scrollArea->setWidget(scrollContent);
    containerLayout->addWidget(scrollArea);
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