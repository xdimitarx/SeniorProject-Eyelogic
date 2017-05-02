#ifndef WIDGET_H
#define WIDGET_H

#include "../EyeLogic/System.hpp"
#include <iostream>
#include <string>


using namespace std;


/*********************
* EXTERNAL VARIABLES *
**********************/
extern QPoint msgBoxSize;
extern int trackEye;
extern int clickType;
extern const std::string filenames [];
extern const std::string refImagesBefore [];
extern const std::string refImagesAfter [];
extern int imageCount;
extern int REFIMAGES;
extern cv::Point screenres;
extern QString user_path;
extern QString ref_images_path;
extern bool RUN;

/*********************
* EXTERNAL FUNCTIONS *
**********************/
void runCalibrate();
void runMain();
std::string toString(QString qs);


/***************
 * GLOBAL ENUM *
 ***************/
enum TrackEye {
    rightEye,
    leftEye
};

enum ClickType {
    voice,
    blink
};

/****************
 * WIDGET CLASS *
 ****************/
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
