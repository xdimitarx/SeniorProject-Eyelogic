#include "widget.h"
#include "ui_widget.h"


QGroupBox *userInfo:: UserInfoLayout()
{
    // define widget to add to main widget
    QGroupBox *userGroupBox = new QGroupBox(tr("User Info"));

    // declare layout for this widget
    QVBoxLayout *vbox = new QVBoxLayout();

    // textbox for the username
    QLineEdit *textBox = new QLineEdit();
    textBox->setPlaceholderText("Enter UserName");

    // add textbox to layout, and layout to widget, and return widget
    vbox->addWidget(textBox);
    userGroupBox->setLayout(vbox);
    return userGroupBox;
}

QGroupBox *calibrationSettings::calibrationSettingsLayout()
{
    // define widget to be added to main widget
    QGroupBox *calibGroupBox = new QGroupBox(tr("Calibration Settings"));

    // define layout to hold the radio buttons for settings
    QVBoxLayout *vbox = new QVBoxLayout();

    // radio buttons to determine what settings user wants for calibration
    QRadioButton *right = new QRadioButton(tr("Use Right Eye"));
    QRadioButton *left = new QRadioButton(tr("Use Left Eye"));
    QRadioButton *old = new QRadioButton(tr("Use Old Settings"));
    QPushButton *startCalib = new QPushButton(tr("Start Calibration Process"));

    // set initial values for radio buttons
    right->setChecked(false);
    left->setChecked(false);
    old->setChecked(false);

    // add buttons to layout, and layout to widget, return widget
    vbox->addWidget(right);
    vbox->addWidget(left);
    vbox->addWidget(old);
    vbox->addWidget(startCalib);
    calibGroupBox->setLayout(vbox);
    return calibGroupBox;
}

QGroupBox *clickSettings::clickSettingsBox()
{
    // define widget to hold click settings
    QGroupBox *clickGroupBox = new QGroupBox(tr("Enable Click Settings"));

    // define layout for the widget
    QVBoxLayout *vbox = new QVBoxLayout();

    // add the click setting buttons
    QRadioButton *voiceClick = new QRadioButton(tr("Enable Voice Clicks"));
    QRadioButton *blinkClick = new QRadioButton(tr("Enable Blink Clicks"));

    // add buttons to layout to widget, return widget
    vbox->addWidget(voiceClick);
    vbox->addWidget(blinkClick);
    clickGroupBox->setLayout(vbox);

    return clickGroupBox;
}
void StartQuit::status()
{
    cout << "start button was pressed" << endl;
}

QGroupBox *StartQuit::StartOrStopBox()
{
    // initilize widget to be added to mainLayout
    QGroupBox *box = new QGroupBox();

    // define layout for the start and stop buttons
    QHBoxLayout *hbox = new QHBoxLayout();

    // define the buttons
    QPushButton *start = new QPushButton(tr("Start Eyelogic"));
    QPushButton *quit = new QPushButton(tr("Quit Eyelogic"));

//    QObject started;

//    connect(start, SIGNAL(clicked()), started, SLOT(StartQuit::status()));

    // add buttons to layout to widget, return widget
    hbox->addWidget(start);
    hbox->addWidget(quit);

    box->setLayout(hbox);
    return box;
}

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    // set up UI and main layout for the widget
    ui->setupUi(this);
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout();

    // start adding in widets to the main layout
    userInfo *a = NULL;
    QGroupBox *userInfoBox = a->UserInfoLayout();

    calibrationSettings *b = NULL;
    QGroupBox *calibrationBox = b->calibrationSettingsLayout();

    clickSettings *c = NULL;
    QGroupBox *clickBox = c->clickSettingsBox();

    StartQuit *d = NULL;
    QGroupBox *startStopBox = d->StartOrStopBox();


    // add widgets to mainLayout
//    mainWidgetLayout->addWidget(userInfoBox);
//    mainWidgetLayout->addWidget(calibrationBox);
//    mainWidgetLayout->addWidget(clickBox);
//    mainWidgetLayout->addWidget(startStopBox);



    // set mainLayout
    this->setLayout(mainWidgetLayout);

}

Widget::~Widget()
{
    delete ui;
}
