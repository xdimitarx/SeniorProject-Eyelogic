#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets>
#include <QWidget>
#include <QObject>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QLayout>

#include <iostream>
#include <string>

using namespace std;

namespace Ui {
class Widget;
}

class QGroupBox;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
};


class clickSettings : public QObject
{
    Q_OBJECT
public:
    clickSettings();
    QGroupBox *clickSettingsBox();

}; // clickSettings Class

class calibrationSettings : public QObject
{
    Q_OBJECT
public:
    calibrationSettings();
    QGroupBox *calibrationSettingsLayout();
private:
    bool righteye;
    bool lefteye;
};  // calibration Settings Class

class userInfo : public QObject
{
    Q_OBJECT
public:
    userInfo();
    void printHello();
    QGroupBox *UserInfoLayout();

private:
    string name;
}; // user info class

class StartQuit : public QObject
{
    Q_OBJECT
public:
    StartQuit();
    QGroupBox *StartOrStopBox();
    void status();
//public slots:
//    void pressed();
//signals:
//    void quit();
//    void calibrate();


};


#endif // WIDGET_H
