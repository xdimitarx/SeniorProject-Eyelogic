#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QtWidgets>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace Ui {
class Window;
}

class QGroupBox;

class Window : public QWidget
{
    Q_OBJECT

public:
    // constructor
    explicit Window(QWidget *parent = 0);

    // destructor for window
    ~Window();

private:
    Ui::Window *ui;

    // function prototypes for implementing UI grid
    QGroupBox *userNameExclusiveGroup();
    QGroupBox *calibSettingsExclusiveGroup();
    QGroupBox *clickSettingsExclusiveGroup();
    QGroupBox *startExclusiveGroup();
    QGroupBox *OSExclusiveGroup();
    void setTitle();
};

class section : public QObject
{
    Q_OBJECT

public:

};

#endif // WINDOW_H
