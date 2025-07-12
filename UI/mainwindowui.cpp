#include "mainwindowui.h"
#include "../mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include "Button.h"
#include "ColorManager.h"
#include "ObjectDisplayOptionsWidget.h"

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
    outerLayout->setContentsMargins(0, 0, 0, 0); 

    // ロゴ画像（全体の一番上・左寄せ）
    QWidget* headerWidget = new QWidget(centralWidget);
    headerWidget->setFixedHeight(70); 
    headerWidget->setStyleSheet(QString("background-color: %1;").arg(ColorManager::HEADER_COLOR.name()));
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0); 
    QLabel* logoLabel = new QLabel(centralWidget);
    QPixmap logoPixmap(":/resources/white_symbol.png");
    logoLabel->setPixmap(logoPixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoLabel);
    QLabel* logoTypeLabel = new QLabel(centralWidget);
    QPixmap logoTypePixmap(":/resources/logo_type.png");
    logoTypeLabel->setPixmap(logoTypePixmap.scaledToHeight(19, Qt::SmoothTransformation));
    logoTypeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(logoTypeLabel);
    headerLayout->addStretch(); // 右側にスペース
    headerLayout->setSpacing(15);
    headerLayout->setContentsMargins(35, 0, 0, 3);
    outerLayout->addWidget(headerWidget);

    // メインの横並びレイアウト
    QHBoxLayout* mainLayout = new QHBoxLayout();

    // 左ペイン
    QVBoxLayout* leftPaneLayout = new QVBoxLayout();
    
    // 3Dオブジェクト表示オプションウィジェットを追加（左ペインから削除）
    // ObjectDisplayOptionsWidget* objectOptions = new ObjectDisplayOptionsWidget("sample.stl", centralWidget);
    // leftPaneLayout->addWidget(objectOptions);
    openStlButton = new Button("Open STL File", centralWidget);
    openVtkButton = new Button("Open VTK File", centralWidget);
    rangeSlider = new DensitySlider(centralWidget);
    modeComboBox = new ModeComboBox(centralWidget);
    
    processButton = new Button("Process", centralWidget);
    export3mfButton = new Button("Export 3MF", centralWidget);
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

    // 右ペイン（VTKウィジェット）
    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    renderer->SetBackground(0.1, 0.1, 0.1);

    mainLayout->addWidget(vtkWidget, 1); // まずvtkWidgetだけをレイアウトに追加
    // mainLayoutをouterLayoutに追加
    outerLayout->addLayout(mainLayout);
    outerLayout->setSpacing(5);

    // leftPaneWidgetをvtkWidgetの上に重ねて配置
    leftPaneWidget->setParent(vtkWidget);
    leftPaneWidget->adjustSize(); // レイアウト内容に合わせて自動でサイズを決める
    leftPaneWidget->move(20, 20); // 位置だけ指定
    leftPaneWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 0); border-radius: 10px; }");
    leftPaneWidget->raise();
    leftPaneWidget->show();
    // 必要なら: leftPaneWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 右ペイン（VTKウィジェットの上に重ねて表示）
    objectDisplayOptionsWidget = new ObjectDisplayOptionsWidget("No stl file selected", vtkWidget);
    objectDisplayOptionsWidget->move(1100, 20); // 右ペイン内の表示位置を調整
    objectDisplayOptionsWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    objectDisplayOptionsWidget->raise();
    objectDisplayOptionsWidget->show();

    // VTKファイル用の表示オプションウィジェット
    vtkDisplayOptionsWidget = new ObjectDisplayOptionsWidget("No vtk file selected", vtkWidget);
    vtkDisplayOptionsWidget->move(1100, 120); // STLウィジェットの下に配置
    vtkDisplayOptionsWidget->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    vtkDisplayOptionsWidget->raise();
    vtkDisplayOptionsWidget->show();

    // 分割されたメッシュ用の表示オプションウィジェット（4つ）
    dividedMeshWidget1 = new ObjectDisplayOptionsWidget("Divided Mesh 1", vtkWidget);
    dividedMeshWidget1->move(1100, 220); // VTKウィジェットの下に配置
    dividedMeshWidget1->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    dividedMeshWidget1->raise();
    dividedMeshWidget1->show();

    dividedMeshWidget2 = new ObjectDisplayOptionsWidget("Divided Mesh 2", vtkWidget);
    dividedMeshWidget2->move(1100, 320); // 1番目の下に配置
    dividedMeshWidget2->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    dividedMeshWidget2->raise();
    dividedMeshWidget2->show();

    dividedMeshWidget3 = new ObjectDisplayOptionsWidget("Divided Mesh 3", vtkWidget);
    dividedMeshWidget3->move(1100, 420); // 2番目の下に配置
    dividedMeshWidget3->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    dividedMeshWidget3->raise();
    dividedMeshWidget3->show();

    dividedMeshWidget4 = new ObjectDisplayOptionsWidget("Divided Mesh 4", vtkWidget);
    dividedMeshWidget4->move(1100, 520); // 3番目の下に配置
    dividedMeshWidget4->setStyleSheet("QWidget { background-color:rgba(45, 45, 45, 200); border-radius: 10px; }");
    dividedMeshWidget4->raise();
    dividedMeshWidget4->show();

    setupStyle();
}

void MainWindowUI::setupStyle()
{
    mainWindow->setStyleSheet("background-color: #1a1a1a;");
} 