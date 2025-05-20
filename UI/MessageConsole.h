#ifndef MESSAGECONSOLE_H
#define MESSAGECONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>

class MessageConsole : public QWidget {
    Q_OBJECT
public:
    explicit MessageConsole(QWidget* parent = nullptr);
    void appendMessage(const QString& message);

signals:
    void messageAdded(const QString& message);

private:
    QTextEdit* textEdit;
};

#endif // MESSAGECONSOLE_H 