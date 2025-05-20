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
            background-color: #1e1e1e;
            color: #ffffff;
            border: 1px solid #444;
            border-radius: 4px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
    )");

    layout->addWidget(textEdit);
    setLayout(layout);
}

void MessageConsole::appendMessage(const QString& message)
{
    textEdit->append(message);
    textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->maximum());
    emit messageAdded(message);
} 