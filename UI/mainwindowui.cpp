#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFrame>

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
    logoLabel->setPixmap(logoPixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logoRowLayout->addWidget(logoLabel);
    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(19, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logoRowLayout->addWidget(logoTypeLabel);
    logoRowLayout->addStretch(); // 右側にスペース
    logoRowLayout->setSpacing(15);
    logoRowLayout->setContentsMargins(0, 0, 0, 3);
    outerLayout->addLayout(logoRowLayout);

    // 横線を追加
    QFrame* horizontalLine = new QFrame(centralWidget);
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Sunken);
    horizontalLine->setStyleSheet("color: #333; background: #333; min-height: 1px; max-height: 1px;");
    outerLayout->addWidget(horizontalLine);

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
    outerLayout->setSpacing(5);

    setupStyle();
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 