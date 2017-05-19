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
extern int voiceOption;
extern const std::string refImageNames [];
extern int imageCount;
extern int REFIMAGES;
extern cv::Point screenres;
extern QString user_path;
extern QString ref_images_path;
extern bool RUN;
extern bool CALIBRATED;

/*********************
* EXTERNAL FUNCTIONS *
**********************/
void runCalibrate();
void runMain();
std::string toString(QString qs);
void disableVoice();
void enableVoice();
void stopVoice();

/***************
 * GLOBAL ENUM *
 ***************/
enum TrackEye {
    rightEye,       // 0
    leftEye         // 1
};

enum Voice {
    on,             // 0
    off             // 1
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
    void toggleLeftEye();
    void toggleRightEye();
    void toggleVoiceOn();
    void toggleVoiceOff();
    void stop();

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
