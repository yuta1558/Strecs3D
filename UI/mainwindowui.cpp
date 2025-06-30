#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>

MainWindowUI::MainWindowUI(MainWindow* mainWindow)
    : mainWindow(mainWindow)
{
    setupUI();
}

void MainWindowUI::setupUI()
{
    centralWidget = new QWidget(mainWindow);
    // 新しい全体レイアウト（縦方向）
    QVBoxLayout* outerLayout = new QVBoxLayout(centralWidget);

    // ロゴ画像（全体の一番上・左寄せ）
    QHBoxLayout* logoRowLayout = new QHBoxLayout();
    QLabel* logoLabel = new QLabel(centralWidget);
    QPixmap logoPixmap(":/resources/white_symbol.png");
    logoLabel->setPixmap(logoPixmap.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logoRowLayout->addWidget(logoLabel);
    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(15, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logoRowLayout->addWidget(logoTypeLabel);
    logoRowLayout->addStretch(); // 右側にスペース
    outerLayout->addLayout(logoRowLayout);

    // メインの横並びレイアウト
    QHBoxLayout* mainLayout = new QHBoxLayout();

    // 左ペイン
    QVBoxLayout* leftPaneLayout = new QVBoxLayout();
    openStlButton = new QPushButton("Open STL File", centralWidget);
    openVtkButton = new QPushButton("Open VTK File", centralWidget);
    rangeSlider = new DensitySlider(centralWidget);
    modeComboBox = new QComboBox(centralWidget);
    modeComboBox->addItem("cura");
    modeComboBox->addItem("bambu");
    processButton = new QPushButton("Process", centralWidget);
    export3mfButton = new QPushButton("export 3mf", centralWidget);
    messageConsole = new MessageConsole(centralWidget);
    messageConsole->setMinimumHeight(200);

    leftPaneLayout->addWidget(openStlButton);
    leftPaneLayout->addWidget(openVtkButton);
    leftPaneLayout->addWidget(rangeSlider);
    leftPaneLayout->addWidget(modeComboBox);
    leftPaneLayout->addWidget(processButton);
    leftPaneLayout->addWidget(export3mfButton);
    leftPaneLayout->addWidget(messageConsole);
    leftPaneLayout->addStretch();

    QWidget* leftPaneWidget = new QWidget(centralWidget);
    leftPaneWidget->setLayout(leftPaneLayout);
    leftPaneWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftPaneWidget->setMaximumWidth(300);
    leftPaneWidget->setStyleSheet("QWidget { background-color: #2D2D2D; border-radius: 10px; }");

    // 右ペイン（VTKウィジェット）
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    renderer->SetBackground(0.1, 0.1, 0.1);

    mainLayout->addWidget(leftPaneWidget, 1);
    mainLayout->addWidget(vtkWidget, 3);

    // mainLayoutをouterLayoutに追加
    outerLayout->addLayout(mainLayout);

    setupStyle();
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 