#include "mainwindow.h"
#include "ui_mainwindow.h"
#undef slots
#include "Python.h"
#define slots Q_SLOTS
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QThread>
#include <QProcess>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), timer(new QTimer(this))
    , pythonInitialized(false), isProcessing(false)
{
    ui->setupUi(this);
    // timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::processFrame);

    // 初始化Python
    initializePython();

    // 如果Python初始化成功，启动定时器
    

    timer->start(33); // 约30fps
    webServer = new QWebSocketServer("BlockCounterServer", QWebSocketServer::NonSecureMode, this);
    if (webServer->listen(QHostAddress::Any, 8080)) {
        qDebug() << "WebSocket server listening on port 8080";
        connect(webServer, &QWebSocketServer::newConnection, this, &MainWindow::onNewWebSocketConnection);
    } else {
        qDebug() << "Failed to start WebSocket server:" << webServer->errorString();
    }
    setupGpioMonitoring();
    // 初始化统计定时器
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &MainWindow::sendBlockCounts);
    statsTimer->start(1000); // 每秒发送一次统计数据

}

MainWindow::~MainWindow()
{
    Py_Finalize();
    gpioPollTimer.stop();
    QFile unexportFile("/sys/class/gpio/unexport");
    if (unexportFile.open(QIODevice::WriteOnly)) {
        unexportFile.write(QString::number(gpioPin).toLatin1());
        unexportFile.close();
    }
    delete ui;
}

void MainWindow::initializePython()
{
    Py_Initialize();
    if(!Py_IsInitialized()){
        qDebug() << "pyinit error";
    }
    PyRun_SimpleString("import os");
    PyRun_SimpleString("import sys");
    // PyRun_SimpleString("sys.path.append(os.path.dirname(os.path.dirname(os.getcwd())))");//ubuntu version
    PyRun_SimpleString("sys.path.append(os.getcwd())");//loongson version
    //PyRun_SimpleString("sys.path.append(os.getcwd())");
    //PyRun_SimpleString("sys.path.append('/home/bkst/Desktop/project/python_qt_test'))");
    PyRun_SimpleString("print(sys.path)");

    PyObject * pModule = PyImport_ImportModule("cv_helloworld");
    if(!pModule){
        qDebug()<< "can not open module";

    }
       // 获取处理帧的函数
    pProcessFrameFunc = PyObject_GetAttrString(pModule, "process_frame");
    if (!pProcessFrameFunc || !PyCallable_Check(pProcessFrameFunc)) {
        PyErr_Print();
        QMessageBox::critical(this, "Error", "Failed to get process_frame function");
        return;
    }

    // 调用初始化函数
    PyObject *pInitFunc = PyObject_GetAttrString(pModule, "initialize");
    if (pInitFunc && PyCallable_Check(pInitFunc)) {
        PyObject *pArgs = PyTuple_New(0);
        PyObject *pResult = PyObject_CallObject(pInitFunc, pArgs);
        Py_DECREF(pArgs);

        if (!pResult) {
            PyErr_Print();
            QMessageBox::warning(this, "Warning", "Failed to initialize video processor");
        } else {
            Py_DECREF(pResult);
        }
    } else {
        PyErr_Clear(); // 如果没有初始化函数也没关系
    }

    pythonInitialized = true;

}

void MainWindow::updateVideoDisplay(const QImage &image)
{
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        // 假设UI中有一个QLabel命名为videoLabel
        ui->videoLabel->setPixmap(pixmap.scaled(
            ui->videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        char color_display[256];
        if (color_detected==1&&objectDetected==1)
            sprintf(color_display, "检测到紫色物块\n");
        else if (color_detected==2&&objectDetected==1)
            sprintf(color_display, "检测到蓝色物块\n");
        else
            sprintf(color_display, "N/A\n");
        ui->colorlabel->setText(color_display);
        char detected_display[256];
        sprintf(detected_display, "detected:%d\n",objectDetected);
        ui->detectedlabel->setText(detected_display);
        char position_display[256];
        sprintf(position_display, "x:%d ,y:%d\n",x,y);
        ui->positionlabel->setText(position_display);
    }
}
void MainWindow::processFrame()
{
    if (!pythonInitialized) return;

    PyObject *pResult = PyObject_CallObject(pProcessFrameFunc, nullptr);

    if (!pResult) {
        PyErr_Print();
        return;
    }
    PyObject *pFrameData = PyTuple_GetItem(pResult, 0);
    PyObject *pColor = PyTuple_GetItem(pResult, 1);
    PyObject *pPosition = PyTuple_GetItem(pResult, 2);
    PyObject *pDetected = PyTuple_GetItem(pResult, 3);

    char *bytesData;
    Py_ssize_t length;
    if (PyBytes_AsStringAndSize(pFrameData, &bytesData, &length)) {
        Py_DECREF(pResult);
        return;
    }

    QImage image((const uchar*)bytesData, 1280, 960, QImage::Format_BGR888);
    updateVideoDisplay(image);
    color_detected = PyLong_AsLong(pColor);
    if (PyTuple_Check(pPosition) && PyTuple_Size(pPosition) == 2) {
        x = PyLong_AsLong(PyTuple_GetItem(pPosition, 0));
        y = PyLong_AsLong(PyTuple_GetItem(pPosition, 1));
        //lastPosition = QPoint(x, y);
    }
    objectDetected = PyLong_AsLong(pDetected) != 0;
    //objectDetected = PyLong_AsLong(pDetected) ;
    Py_DECREF(pResult);
}
void MainWindow::waitNonBlocking()
{
    QEventLoop loop;
    QTimer timer;

    connect(&timer, &QTimer::timeout, this, [&]() {
        if (x<=550&&x>=300&&objectDetected) {
            color = color_detected;
            loop.quit();
        }
    });

    timer.start(100); // 每100ms检查一次
    loop.exec(); // 不会阻塞主事件循环

}
void MainWindow::on_StartButton_clicked()
{
    //system("cd ~/Desktop");
    //system("python3 ~/Desktop/color_v1_0.py");
    ui->StartButton->setEnabled(false);
    Delay_MSec(100);
    system("stty -F /dev/ttyS1 speed 115200 cs8");
    Delay_MSec(100);
    system("stty -F /dev/ttyS1 speed 115200 cs8");
    system("echo s 1 > /dev/ttyS1");

    waitNonBlocking();
    system("echo p > /dev/ttyS1");
    
    startDetachedPython();
    Delay_MSec(13000);
    system("echo s 1 3350 > /dev/ttyS1");
    Delay_MSec(3450);
    if (color==1){
        system("python3 ~/code/arm1.py");
        brownBlockCount++;
    }
    if (color==2){
        system("python3 ~/code/arm2.py");
        blueBlockCount++;
    }

    ui->StartButton->setEnabled(true);
}

void MainWindow::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
void MainWindow::startDetachedPython()
{
    bool success;
    if(color==1)
        success = QProcess::startDetached("python3", {"/home/sheshenxian/code/laser1.py"});
    if(color==2)
        success = QProcess::startDetached("python3", {"/home/sheshenxian/code/laser2.py"});
    if (!success) {
        qDebug() << "启动Python脚本失败";
    }
}




void MainWindow::sendBlockCounts()
{
    QJsonObject json;
    json["A"] = brownBlockCount;  // 对应HTML中的传感器A
    json["B"] = blueBlockCount;   // 对应HTML中的传感器B

    QJsonDocument doc(json);
    QString message = doc.toJson(QJsonDocument::Compact);

    for (QWebSocket *client : qAsConst(clients)) {
        if (client && client->isValid()) {
            client->sendTextMessage(message);
        }
    }
}
void MainWindow::onNewWebSocketConnection()
{
    QWebSocket *socket = webServer->nextPendingConnection();
    connect(socket, &QWebSocket::disconnected, this, &MainWindow::socketDisconnected);
    clients.append(socket);
    sendBlockCounts(); // 新连接建立时立即发送当前统计
}

void MainWindow::socketDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client) {
        clients.removeAll(client);
        client->deleteLater();
    }
}









void MainWindow::setupGpioMonitoring()
{
    // 导出GPIO
    qDebug() << "setupgpio";
    qDebug() << "Path exists:" << QFile::exists("/sys/class/gpio/export");
    //QFile::remove("/sys/class/gpio/unexport"); // 确保先取消导出
    QFile unexportFile("/sys/class/gpio/unexport");
    if (unexportFile.open(QIODevice::WriteOnly)) {
        unexportFile.write(QString::number(gpioPin).toLatin1());
        unexportFile.close();
    }
    QFile exportFile("/sys/class/gpio/export");
    if (exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "exportgpio";
        exportFile.write(QString::number(gpioPin).toLatin1());
        exportFile.close();
    }

    qDebug() << "Open error:" << exportFile.error() << exportFile.errorString();
    // 设置为输入模式
    QFile directionFile(QString("/sys/class/gpio/gpio%1/direction").arg(gpioPin));
    if (directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "gpiooutput";
        directionFile.write("in");
        directionFile.close();
    }

    // 配置轮询定时器
    gpioPollTimer.setInterval(1000); // 100ms轮询间隔
    connect(&gpioPollTimer, &QTimer::timeout, this, &MainWindow::pollGpioState);
    gpioPollTimer.start();

    // 初始读取
    pollGpioState();
}

// GPIO状态轮询函数
void MainWindow::pollGpioState()
{
    gpioValueFile.setFileName(QString("/sys/class/gpio/gpio%1/value").arg(gpioPin));
    if (gpioValueFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray value = gpioValueFile.readAll().trimmed();
        gpioValueFile.close();

        bool isLow = (value == "0");
       
        if(isLow){
        ui->gpioStatusLabel->setText("急停！请复位后重启程序！");
        ui->gpioStatusLabel->setStyleSheet("color: red; font-weight: bold;" );
        }
        else
            ui->gpioStatusLabel->setText("");
    }
}
