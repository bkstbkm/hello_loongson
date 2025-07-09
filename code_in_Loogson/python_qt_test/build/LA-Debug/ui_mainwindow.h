/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *StartButton;
    QLabel *videoLabel;
    QLabel *colorlabel;
    QLabel *detectedlabel;
    QLabel *positionlabel;
    QLabel *label;
    QLabel *gpioStatusLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(617, 370);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        StartButton = new QPushButton(centralwidget);
        StartButton->setObjectName(QString::fromUtf8("StartButton"));
        StartButton->setGeometry(QRect(40, 50, 89, 25));
        videoLabel = new QLabel(centralwidget);
        videoLabel->setObjectName(QString::fromUtf8("videoLabel"));
        videoLabel->setGeometry(QRect(190, 30, 320, 240));
        videoLabel->setAlignment(Qt::AlignCenter);
        colorlabel = new QLabel(centralwidget);
        colorlabel->setObjectName(QString::fromUtf8("colorlabel"));
        colorlabel->setGeometry(QRect(40, 160, 131, 17));
        detectedlabel = new QLabel(centralwidget);
        detectedlabel->setObjectName(QString::fromUtf8("detectedlabel"));
        detectedlabel->setGeometry(QRect(40, 200, 161, 17));
        positionlabel = new QLabel(centralwidget);
        positionlabel->setObjectName(QString::fromUtf8("positionlabel"));
        positionlabel->setGeometry(QRect(40, 240, 171, 17));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 110, 67, 17));
        gpioStatusLabel = new QLabel(centralwidget);
        gpioStatusLabel->setObjectName(QString::fromUtf8("gpioStatusLabel"));
        gpioStatusLabel->setGeometry(QRect(60, 280, 491, 31));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 617, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        StartButton->setText(QCoreApplication::translate("MainWindow", "start", nullptr));
        videoLabel->setText(QCoreApplication::translate("MainWindow", "camera", nullptr));
        colorlabel->setText(QCoreApplication::translate("MainWindow", "color", nullptr));
        detectedlabel->setText(QCoreApplication::translate("MainWindow", "detected", nullptr));
        positionlabel->setText(QCoreApplication::translate("MainWindow", "position", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        gpioStatusLabel->setText(QCoreApplication::translate("MainWindow", "txt", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
