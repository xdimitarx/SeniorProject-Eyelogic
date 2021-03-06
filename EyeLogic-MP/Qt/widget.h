#ifndef WIDGET_H
#define WIDGET_H

#include "../EyeLogic/System.hpp"

#include "../EyeLogic/VoiceTool.hpp"

#include <iostream>
#include <string>

/*********************
 * MACRO DEFINITIONS *
 *********************/
#define REFIMAGES 4                      // number of reference points
#define MAXFRAMES 30

using namespace std;

/*********************
* EXTERNAL VARIABLES *
**********************/
extern QPoint msgBoxSize;
extern const std::string refImageNames [];
extern int imageCount;
extern cv::Point screenres;
extern QString user_path;
extern QString ref_images_path;
extern bool RUN;
extern bool CALIBRATED;
extern bool RunOnce;

/*********************
* EXTERNAL FUNCTIONS *
**********************/
bool runCalibrate();
void runMain();
void printError(std::string);
std::string toString(QString qs);

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

	QPushButton *nextButton;

public slots:
    void calibrate();
    void run();
    void next();
    void cancel();
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
