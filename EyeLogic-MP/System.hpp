//
//  System.hpp
//  EyeLogic
//
//  Created by Dimitar Vasilev on 3/24/17.
//  Copyright © 2017 Dimitar Vasilev. All rights reserved.
//

#ifndef System_h
#define System_h

#ifdef __APPLE__
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ApplicationServices/ApplicationServices.h>
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
#include <QDebug>
#include <QThread>
#else
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "wtypes.h"
#include "stdafx.h"
#include "windows.h"
#include "targetver.h"
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QWidget>
#include <QtCore/QObject>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLayout>
#include <QtCore/QPoint>
#include <QtCore/QDir>
#include <QtGui/QPixmap>
#include <QtCore/QDebug>
#include <QtCore/Qthread>
#endif

class System {
public:
	virtual void setCurPos(cv::Point setCursor) { return; };
	virtual cv::Point getCurPos() { return cv::Point(); };
	virtual void click() { return; };
	virtual void drag() { return; };
	virtual void rightClick() { return; };
    virtual bool voiceFork(){return false;};
    virtual std::string readFromJulius(){return "";};
    virtual cv::Point getScreenResolution(){return cv::Point(); };
};

#endif /* System_h */
