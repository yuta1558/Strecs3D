#include "MessageConsole.h"
#include <QScrollBar>

MessageConsole::MessageConsole(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: rgba(26, 26, 26, 180);
            color: #ffffff;
            border: 1px solid #444;
            border-radius: 4px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
    )");
    // スクロールバーを非表示に
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addWidget(textEdit);
    setLayout(layout);
}

void MessageConsole::appendMessage(const QString& message)
{
    textEdit->append(message);
    textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
    emit messageAdded(message);
} 