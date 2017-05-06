#include "widget.h"
#include "ui_widget.h"


QGroupBox *Widget::userInfoBox()
{
    // define widget to add to main widget
    QGroupBox *userGroupBox = new QGroupBox(tr("User Info"));

    // declare layout for this widget
    QVBoxLayout *vbox = new QVBoxLayout();

    // textbox for the username
    QLineEdit *textBox = new QLineEdit();
    textBox->setPlaceholderText("Enter UserName");
    textBox->setObjectName("userName");

    // add textbox to layout, and layout to widget, and return widget
    vbox->addWidget(textBox);
    userGroupBox->setLayout(vbox);
    return userGroupBox;
}

QGroupBox *Widget::calibrationSettingsBox()
{
    // define widget to be added to main widget
    QGroupBox *calibGroupBox = new QGroupBox(tr("Calibration Settings"));

    // define layout to hold the radio buttons for settings
    QVBoxLayout *vbox = new QVBoxLayout();

    // radio buttons to determine what settings user wants for calibration
    QRadioButton *right = new QRadioButton(tr("Use Right Eye"));
    right->setProperty("trackEye", rightEye);
    right->setObjectName("calibrationButton");
    QRadioButton *left = new QRadioButton(tr("Use Left Eye"));
    left->setProperty("trackEye", leftEye);
    left->setObjectName("calibrationButton");
    QPushButton *startCalib = new QPushButton(tr("Start Calibration Process"));

    QObject::connect(startCalib, SIGNAL(clicked(bool)), this, SLOT(calibrate()));

    // set first radio button checked by default
    right->setChecked(true);

    // add buttons to layout, and layout to widget, return widget
    vbox->addWidget(right);
    vbox->addWidget(left);
    vbox->addWidget(startCalib);
    calibGroupBox->setLayout(vbox);
    return calibGroupBox;
}

QGroupBox *Widget::clickSettingsBox()
{
    // define widget to hold click settings
    QGroupBox *clickGroupBox = new QGroupBox(tr("Set Voice Option"));

    // define layout for the widget
    QVBoxLayout *vbox = new QVBoxLayout();

    // add the click setting buttons
    QRadioButton *onVoice = new QRadioButton(tr("On"));
    onVoice->setProperty("Voice", on);
    onVoice->setObjectName("clickSettingsButton");
    QRadioButton *offVoice = new QRadioButton(tr("Off"));
    offVoice->setProperty("Voice", off);
    offVoice->setObjectName("clickSettingsButton");



    // set first radio button checked by default
    onVoice->setChecked(true);

    // add buttons to layout to widget, return widget
    vbox->addWidget(onVoice);
    vbox->addWidget(offVoice);
    clickGroupBox->setLayout(vbox);

    return clickGroupBox;
}


QGroupBox *Widget::startOrStopBox()
{
    // initilize widget to be added to mainLayout
    QGroupBox *box = new QGroupBox();

    // define layout for the start and stop buttons
    QHBoxLayout *hbox = new QHBoxLayout();

    // define the buttons
    QPushButton *start = new QPushButton(tr("Start Eyelogic"));
    start->setObjectName("runButton");
    QPushButton *quit = new QPushButton(tr("Quit Eyelogic"));

    QObject::connect(quit, SIGNAL(clicked(bool)), QApplication::instance(), SLOT(quit()));
    QObject::connect(start, SIGNAL(clicked(bool)), this, SLOT(run()));

    // add buttons to layout to widget, return widget
    hbox->addWidget(start);
    hbox->addWidget(quit);

    box->setLayout(hbox);
    return box;
}

QGroupBox *Widget::getCalibBox()
{
    // create calibration page
    QGroupBox *calib_box = new QGroupBox();
    QHBoxLayout *hbox = new QHBoxLayout();

    QPushButton *next = new QPushButton(tr("Next"));
    next->setObjectName("next");
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    cancel->setObjectName("cancelOrDone");

    // connect signals
    QObject::connect(next, SIGNAL(clicked(bool)), this, SLOT(next()));
    QObject::connect(cancel, SIGNAL(clicked(bool)), this, SLOT(cancel()));

    // add buttons to layout, return widget
    hbox->addWidget(next);
    hbox->addWidget(cancel);
    calib_box->setLayout(hbox);

    return calib_box;
}

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
   ui->setupUi(this);

   frontPage = new QWidget;
   calibrationPage = new QWidget;

   QVBoxLayout *frontPageLayout = new QVBoxLayout;
   QVBoxLayout *calibrationPageLayout = new QVBoxLayout;

   // add widgets to group boxes
   userBox = userInfoBox();
   calibSettingsBox = calibrationSettingsBox();
   clickBox = clickSettingsBox();
   runBox = startOrStopBox();

   // add group boxes to frontPageLayout
   frontPageLayout->addWidget(userBox);
   frontPageLayout->addWidget(calibSettingsBox);
   frontPageLayout->addWidget(clickBox);
   frontPageLayout->addWidget(runBox);

   // get groupbox and add to calibrationPageLayout
   calibBox = getCalibBox();
   calibBox->setWindowFlags(Qt::WindowStaysOnTopHint);
   calibrationPageLayout->addWidget(calibBox);
   calibrationPage->setWindowFlags(Qt::WindowStaysOnTopHint);
   calibrationPage->setLayout(calibrationPageLayout);
   calibrationPage->hide();

   // add pages to stacked layout
   frontPage->setLayout(frontPageLayout);
   setLayout(frontPageLayout);


}

Widget::~Widget()
{
    delete ui;
}

void Widget::moveToCenter(QWidget *w)
{
    QSize window_size = this->size();
    QPoint center = QPoint(screenres.x/2 - w->width()/2, screenres.y/2 - w->height()/2);
    w->move(center);
}

void Widget::calibrate()
{

    imageCount = 0;

    QString user = userBox->findChild<QLineEdit *>("userName")->text();
    user_path = QDir::currentPath() + "/" + user;

    // set text on cancelOrDone button to `"cancel"`
    QPushButton *cancelOrDoneBtn = calibBox->findChild<QPushButton *>("cancelOrDone");
    cancelOrDoneBtn->setText("Cancel");

    // enable next button
    QPushButton *nextButton = calibBox->findChild<QPushButton *>("next");
    nextButton->setEnabled(true);

    // error message box
    QMessageBox messageBox;

    // no user specified
    if(user == ""){
        messageBox.setText("Please enter a username first");
        messageBox.setFixedSize(msgBoxSize.x(), msgBoxSize.y());
        messageBox.exec();
        return;
    }

    // user already exists
    if(QDir(user_path).exists()){
        messageBox.setText("User already exists. Would you like to overwrite?");
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        switch(messageBox.exec()){
            case QMessageBox::Yes:
                qDebug() << "overwriting files";
                QDir(user_path).removeRecursively();
                break;
            case QMessageBox::No:
                qDebug() << "returning to front page";
                return;
        }

    }

    // new user
    QDir().mkdir(user_path);

    // display red dot full screen
    imageLabel = new QLabel();
    QString ref_image = ref_images_path + QString::fromStdString(refImagesBefore[imageCount]) + ".jpg";
    imageLabel->setPixmap(QPixmap(ref_image));
    imageLabel->showFullScreen();


    // move calibration box on top of image to bottom-middle of screen
    calibrationPage->show();
    calibrationPage->move(screenres.x/2 - calibrationPage->width()/2, screenres.y - calibrationPage->height() - 30);

    // disable next button
    nextButton->setEnabled(false);

    //***************************
    // CALL CALIBRATION FUNCTION
    //***************************
//    runCalibrate();

    // display green dot
    ref_image = ref_images_path + QString::fromStdString(refImagesAfter[imageCount]) + ".jpg";
    imageLabel->setPixmap(QPixmap(ref_image));
    imageLabel->showFullScreen();

    // enable next button
    nextButton->setEnabled(true);

}

void Widget::run()
{

    QPushButton *runButton = runBox->findChild<QPushButton *>("runButton");

    // Check if program is already running
    if(runButton->text() == "Start Eyelogic"){

        QMessageBox messageBox;
        QString user = userBox->findChild<QLineEdit *>("userName")->text();
        QString user_path = QDir::currentPath() + "/" + user;

        // no user specified
        if(user == ""){
           messageBox.setText("Please enter a username first");
           messageBox.setFixedSize(msgBoxSize.x(), msgBoxSize.y());
           messageBox.exec();
           return;
        }

       // check whether specified user has calibrated
       if(!QDir(user_path).exists()){
           messageBox.setText("Please calibrate first");
           messageBox.setFixedSize(msgBoxSize.x(), msgBoxSize.y());
           messageBox.exec();
           return;
       }

        // find whether left or right eye selected
        QButtonGroup group;
        QList<QRadioButton *> calibrationButtons = calibSettingsBox->findChildren<QRadioButton *>("calibrationButton");
        for(int i = 0; i < calibrationButtons.size(); ++i)
        {
          group.addButton(calibrationButtons[i],i);
        }

        if(group.checkedButton()->property("trackEye") == leftEye){
          trackEye = leftEye;
        }
        else if(group.checkedButton()->property("trackEye") == rightEye){
          trackEye = rightEye;
        }


        // find whether user or voice option selected
        QButtonGroup group2;
        QList<QRadioButton *> clickSettingsButtons = clickBox->findChildren<QRadioButton *>("clickSettingsButton");
        for(int i = 0; i < clickSettingsButtons.size(); ++i)
        {
          group2.addButton(clickSettingsButtons[i],i);
        }
        if(group2.checkedButton()->property("Voice") == on){
          voiceOption = on;
        }
        else if(group2.checkedButton()->property("Voice") == off){
          voiceOption = off;
        }


        // set start button to false
        userBox->findChild<QLineEdit *>("userName")->setDisabled(true);
        runButton->setText("Pause");

        // run main program message box
        messageBox.setText("running main program");
        messageBox.setFixedSize(msgBoxSize.x(), msgBoxSize.y());
        messageBox.exec();
        setWindowState(Qt::WindowMinimized);

        //*********************
        // CALL MAIN PROGRAM
        //*********************
        RUN = true;
//        runMain();



   }
   else if (runButton->text() == "Pause"){

       //********************
       // PAUSE MAIN PROGRAM
       //********************
        RUN = false;
        userBox->findChild<QLineEdit *>("userName")->setDisabled(false);
        runButton->setText("Start Eyelogic");
   }
}

void Widget::next()
{
    if(imageCount == REFIMAGES - 3){
        QPushButton *nextButton = calibBox->findChild<QPushButton *>("next");
        QPushButton *cancelOrDoneBtn = calibBox->findChild<QPushButton *>("cancelOrDone");
        nextButton->setEnabled(false);
        cancelOrDoneBtn->setText("Done");
    }
    imageCount++;
    QString ref_image = ref_images_path + QString::fromStdString(refImagesBefore[imageCount]) + ".jpg";
    imageLabel->setPixmap(QPixmap(ref_image));
    imageLabel->showFullScreen();

    //***************************
    // CALL CALIBRATION FUNCTION
    //***************************
//    runCalibrate();

    // display green dot
    ref_image = ref_images_path + QString::fromStdString(refImagesAfter[imageCount]) + ".jpg";
    imageLabel->setPixmap(QPixmap(ref_image));
    imageLabel->showFullScreen();


}

void Widget::cancel()
{

    QPushButton *cancelOrDoneBtn = calibBox->findChild<QPushButton *>("cancelOrDone");

    if(cancelOrDoneBtn->text() == "Cancel"){
        // delete directory
        QString user = userBox->findChild<QLineEdit *>("userName")->text();
        QString user_path = QDir::currentPath() + "/" + user;
        QDir dir(user_path);
        dir.removeRecursively();

        imageLabel->showNormal();
        delete imageLabel;
        calibrationPage->hide();

    }
    else if(cancelOrDoneBtn->text() == "Done"){
        imageLabel->showNormal();
        delete imageLabel;
        calibrationPage->hide();
    }


}
