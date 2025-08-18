#pragma once
#include <QPushButton>
#include <QIcon>

class CustomCheckBox : public QPushButton {
    Q_OBJECT
public:
    explicit CustomCheckBox(QWidget* parent = nullptr);
    void setChecked(bool checked);
    bool isChecked() const;

signals:
    void toggled(bool checked);

private slots:
    void handleClicked();

private:
    void updateIcon();
    bool checked;
    QIcon iconChecked;
    QIcon iconUnchecked;
}; 