#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#undef slots
#include "Python.h"
#define slots Q_SLOTS
#include <QLabel>
//#include <QProcess>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Delay_MSec(unsigned int msec);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    // PyObject *pModule;
    PyObject *pProcessFrameFunc;
    QLabel *videoLabel;
    bool pythonInitialized;
    bool isProcessing;
    int x;
    int y;
    int color_detected;
    int color;
    bool objectDetected;
    QWebSocketServer *webServer;
    QList<QWebSocket *> clients;
    int brownBlockCount = 0;
    int blueBlockCount = 0;
    QTimer *statsTimer;
    QTimer gpioPollTimer;  // 使用成员变量而非指针
    QFile gpioValueFile;
    const int gpioPin = 57; // 监控的GPIO引脚号，根据实际修改

private slots:
    //void importFrame();
    void processFrame();
    void initializePython();
    void updateVideoDisplay(const QImage &image);
    void on_StartButton_clicked();
    void waitNonBlocking();
    void startDetachedPython();
    void onNewWebSocketConnection();
    void socketDisconnected();
    void sendBlockCounts();
    void pollGpioState();
    void setupGpioMonitoring();
};
#endif // MAINWINDOW_H
