// Author: Alexander Soong
#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) : QWidget(parent), ui(new Ui::Window)
{
    ui->setupUi(this);

//    QGridLayout *mainGrid = new QGridLayout;
    QVBoxLayout *main = new QVBoxLayout;
//    mainGrid->addWidget(userNameExclusiveGroup(), 0, 0);
//    mainGrid->addWidget(calibSettingsExclusiveGroup(), 0, 1);
    main->addWidget(userNameExclusiveGroup());
    main->addWidget(calibSettingsExclusiveGroup());
    main->addWidget(clickSettingsExclusiveGroup());
    main->addWidget(OSExclusiveGroup());
    QPushButton *startButton = new QPushButton(tr("&Start EyeLogic"));
    main->addWidget(startButton);
    setLayout(main);

    setWindowTitle(tr("EyeLogic Start Page"));
    resize(800, 500);
}

Window::~Window() // destructor, delete the UI
{
    delete ui;
}


QGroupBox *Window::userNameExclusiveGroup()
{
    // group box to hold the layout
    QGroupBox *groupBox = new QGroupBox(tr("Login Info"));

    // declare the layout, and lineedit within
    QHBoxLayout *hbox = new QHBoxLayout;
    QLineEdit *usernameLine = new QLineEdit();

    // set placeholder text
    usernameLine->setPlaceholderText("Enter Username");
    hbox->addWidget(usernameLine);
    hbox->addStretch(1);
    groupBox->setLayout(hbox);

    return groupBox;
} // set layout for username section

QGroupBox *Window::calibSettingsExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Calibration Settings"));
    QRadioButton *left = new QRadioButton(tr("&Use Left Eye"));
    QRadioButton *right = new QRadioButton(tr("&Use Right Eye"));
    QRadioButton *previous = new QRadioButton(tr("&Use Previous Settings"));

    left->setChecked(false);
    right->setChecked(false);
    previous->setChecked(false);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(left);
    vbox->addWidget(right);
    vbox->addWidget(previous);
    vbox->addStretch(1);

    groupBox->setLayout(vbox);

    return groupBox;
} // set layout for calibration settings section

QGroupBox *Window::clickSettingsExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("&Enable Click Settings"));
    groupBox->setCheckable(true);
    groupBox->setChecked(false);

    QRadioButton *useVoice = new QRadioButton(tr("&Use Voice for Click"));
    QRadioButton *useBlink = new QRadioButton(tr("&Use Blink for Click"));

    useVoice->setChecked(false);
    useBlink->setChecked(false);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(useVoice);
    hbox->addWidget(useBlink);

    groupBox->setLayout(hbox);

    return groupBox;
} // set layout for click settings section

QGroupBox *Window::OSExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("&Select your OS"));
    QRadioButton *mac = new QRadioButton(tr("&MacOS"));
    QRadioButton *win = new QRadioButton(tr("&Windows"));

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(mac);
    hbox->addWidget(win);

    groupBox->setLayout(hbox);
    return groupBox;
} //what OS are you using?

QGroupBox *Window::startExclusiveGroup()
{

} // start button
