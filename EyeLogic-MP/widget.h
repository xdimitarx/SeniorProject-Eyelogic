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
#include <QPoint>
#include <QDir>
#include <QPixmap>
#include <iostream>
#include <string>

// debug header files
#include <QDebug>
#include <QThread>


using namespace std;


extern QPoint msgBoxSize;
extern int trackEye;
extern int clickType;
extern const std::string filenames [];
extern int screen_width;
extern int screen_height;
extern const std::string refImagesBefore [];
extern const std::string refImagesAfter [];
extern int imageCount;
extern int REFIMAGES;

enum TrackEye {
    rightEye,
    leftEye
};

enum ClickType {
    voice,
    blink
};

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

    void moveToCenter(QWidget *w);

    QGroupBox *clickSettingsBox();
    QGroupBox *calibrationSettingsBox();
    QGroupBox *userInfoBox();
    QGroupBox *startOrStopBox();
    QGroupBox *getCalibBox();

public slots:
    void calibrate();
    void run();
    void next();
    void cancel();

private:
    Ui::Widget *ui;
    QWidget *frontPage;
    QWidget *calibrationPage;
    QWidget *imageWidget;

    QGroupBox *userBox;
    QGroupBox *calibSettingsBox;
    QGroupBox *clickBox;
    QGroupBox *runBox;

    QGroupBox *calibBox;
    QLabel *imageLabel;

};

#endif  // WIDGET_H
