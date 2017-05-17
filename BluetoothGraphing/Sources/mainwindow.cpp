#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include "shockclockreader.h"
#include <qwt_plot_canvas.h>
#include <qwt_plot_glcanvas.h>
#include <qevent.h>
#include <qwt_plot_marker.h>
#include <stdint.h>
#include <qwt_symbol.h>
#include <qwt_plot_glcanvas.h>
#include <QApplication>
#include <QFileDialog>
class Tester : public QWidget
{
public:
  void openFile()
  {
    QFileDialog::getOpenFileName( this, tr("Open Document"), QDir::currentPath(), tr("Document files (*.doc *.rtf);;All files (*.*)"), 0, QFileDialog::DontUseNativeDialog );

    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open Document"),
        QDir::currentPath(),
        tr("Document files (*.doc *.rtf);;All files (*.*)") );
    if( !filename.isNull() )
    {
      //qDebug() << filename;
    }
  }

  void openFiles()
  {
    QStringList filenames = QFileDialog::getOpenFileNames(
        this,
        tr("Open Document"),
        QDir::currentPath(),
        tr("CSV (*.csv);;All files (*.*)") );
    if( !filenames.isEmpty() )
    {
      //qDebug() << filenames.toVector();
    }
  }

  void openDir()
  {
    QString dirname = QFileDialog::getExistingDirectory(
        this,
        tr("Select a Directory"),
        QDir::currentPath() );
    if( !dirname.isNull() )
    {
      //qDebug() << dirname;
    }
  }

  QString saveFile()
  {
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Save Document"),
        QDir::currentPath(),
        tr("CSV (*.csv)") );
    if( !filename.isNull() )
    {
      //qDebug() << filename;
      return filename;
    }
    return "";
  }
};
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    pressedKey = event->key();
    pressedKey += "\r\n";
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<QVector<double> >("QVector<double>");


    //thread for the shockClockReader
    scThread = new QThread();
    showSamplesCount = 240;
    reader = new ShockClockReader();
    QObject::connect(scThread, SIGNAL(finished()), scThread, SLOT(deleteLater()));
    QObject::connect(scThread, SIGNAL(started()), reader, SLOT(ConnectBLEDevice()) );
    QObject::connect(scThread, SIGNAL(finished()), reader, SLOT(deleteLater()));
    QObject::connect(reader, SIGNAL(newValues(QVector<double>)), this, SLOT(receiveReading(QVector<double>)));
    QObject::connect(ui->saveData, SIGNAL(released()), this, SLOT(saveData()));
    reader->moveToThread(scThread);
    scThread->start();
    scThread->setPriority(QThread::HighestPriority);
    //ui->vectorLayout->
    initializePlots();
    recording = false;
    pressedKey = "";
}

void MainWindow::saveData()
{
    if(recording == false)
    {
        Tester t;
        saveFileLocation = t.saveFile();
       // qDebug() << saveFileLocation;
        file = new QFile(saveFileLocation);
        file->open(QIODevice::ReadWrite);
        stream = new QTextStream(file);
        if(file->isOpen())
        {
            recording = true;
            ui->saveData->setText("Stop");
        }
    } else {
        ui->saveData->setText("Record Data");
        recording = false;
        file->close();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

double previousMainGyro, previousAntGyro, previousMotor,previousEncoder = 0;
QVector<double> gyro;
QVector<double> encoder;
bool hitGyro = false;
bool hitEncoder = false;
int hitEncoderCount = 0;
int hitGyroCount = 0;
void MainWindow::receiveReading(QVector<double> values)
{
    int xcount = 0;
    int ycount = 0;

    for(xcount; xcount < (int)values.at(0); xcount++){

        if((values.at(xcount+2)<36) & (values.at(xcount+2)>-36) ){
            motorEncoder.push_back(values.at(xcount+2));
            //qDebug() << "motor data: " << values.at(xcount + 2);
        }
    }
    //qDebug() << "x\n " << xpos;
    for(ycount; ycount < (int)values.at(1); ycount++){
        int index = xcount+ycount+2;

        if( (values.at(index) < 36) & (values.at(index) > -36)){
            if((previousEncoder < (values.at(index) - 10)) | (previousEncoder > (values.at(index)+10))){
                hitEncoder = true;
            }
            if(hitEncoder){
                hitEncoderCount++;
                if(hitEncoderCount < 10){
                    encoder.push_back(values.at(index));
                }
                else{
                    hitEncoder = false;
                    hitEncoderCount = 0;
                    qDebug() << "encoder hit " << encoder;
                }
            }
            freeEncoder.push_back(values.at(index));
            previousEncoder = values.at(index);
            //qDebug() << "encoder data: " << values.at(index) << "\n";
        }
    }

    mainGyro.push_back(values.at(xcount + ycount+2));
    if((previousAntGyro < (values.at(xcount+ycount+3)-6)) | (previousAntGyro > (values.at(xcount+ycount+3)+6))){
        hitGyro = true;
    }
    if(hitGyro){
        hitGyroCount++;
        if(hitGyroCount < 10){
            gyro.push_back(values.at(xcount+ycount+3));
        }
        else{
            hitGyro = false;
            hitGyroCount = 0;
            qDebug() << "gyro hit " << gyro;
        }
    }
    anteriorGyro.push_back(values.at(xcount + ycount+3));
    previousAntGyro = values.at(xcount +ycount+3);
    anteriorADXL.push_back(values.at(xcount + ycount+7));
    mainADXL.push_back(values.at(xcount + ycount+8));

    //qDebug() << "values " << values.at(xcount+ycount+2) << ", " << values.at(xcount+ycount+3);

    if(recording == true)
    {
        QString string = pressedKey + QString::number(values.at(0)) + ", " + QString::number(values.at(1)) + ", " + QString::number(values.at(2)) + ", " + QString::number(values.at(3)) + ", " + QString::number(values.at(4)) + ", " + QString::number(values.at(5)) + "\r\n";
        *stream << string;

        stream->flush();
        file->flush();
        pressedKey = "";
    }
    //make sure that there aren't too many values
    while(motorEncoder.size() > showSamplesCount)
    {
        motorEncoder.pop_front();

    }
    while(freeEncoder.size() > showSamplesCount)
    {
        freeEncoder.pop_front();
    }
    while(mainGyro.size() > showSamplesCount)
    {
        mainGyro.pop_front();
    }

    while(anteriorGyro.size() > showSamplesCount)
    {
        anteriorGyro.pop_front();
    }
    while(anteriorADXL.size() > showSamplesCount)
    {
        anteriorADXL.pop_front();
    }
    while(mainADXL.size() > showSamplesCount)
    {
        mainADXL.pop_front();
    }


    posX.clear();
    posY.clear();
    posZ.clear();

    antX.clear();
    antY.clear();
    antZ.clear();



    for(int i = 0; i < motorEncoder.size(); i ++){
        posX << QPointF(i, motorEncoder.at(i));
    }

    for(int i = 0; i < freeEncoder.size(); i++){
        posY << QPointF(i, freeEncoder.at(i));
    }

    for(int i = 0; i < mainGyro.size(); i++){
        posZ << QPointF(i, mainGyro.at(i));
    }
    for(int i = 0; i < anteriorGyro.size(); i++){
        antX << QPointF(i, anteriorGyro.at(i));
    }
    for(int i = 0; i < anteriorADXL.size(); i++){
        antY << QPointF(i, anteriorADXL.at(i));
    }
    for(int i = 0; i < mainADXL.size(); i++){
        antZ << QPointF(i, mainADXL.at(i));
    }
    //set the vaues for the posterior


    posXCurve->setSamples(posX);
    posYCurve->setSamples(posY);
    posZCurve->setSamples(posZ);

    antXCurve->setSamples(antX);
    antYCurve->setSamples(antY);
    antZCurve->setSamples(antZ);

    pposx->replot();
    pposy->replot();
    pposz->replot();

    pantx->replot();
    panty->replot();
    pantz->replot();
}


void MainWindow::initializePlots()
{

    QBrush brush;
    gyronum = ui->gyroDifference;

    //posterior
    pposx = ui->posteriorX;
    pposx->setAxisAutoScale(QwtPlot::yLeft, false);
    pposx->setAxisScale(QwtPlot::yLeft, -35, 35, 7);
    pposx->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pposx->setAxisTitle(QwtPlot::yLeft, "rad/s");

    posXCurve = new QwtPlotCurve();

    pposx->setTitle("motor encoder");
    posXCurve->setPen(Qt::red, 1);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    posXCurve->setBrush(brush);
    posXCurve->attach(pposx);
    posXCurve->setSamples(posX);

    pposy = ui->posteriorY;
    pposy->setAxisAutoScale(QwtPlot::yLeft, false);
    pposy->setAxisScale(QwtPlot::yLeft, -35, 35, 7);
    pposy->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pposy->setAxisTitle(QwtPlot::yLeft, "rad/s");

    posYCurve = new QwtPlotCurve();
    pposy->setTitle("free-spinning encoder");
    posYCurve->setPen(Qt::black, 1);
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    posYCurve->setBrush(brush);
    posYCurve->attach(pposy);
    posYCurve->setSamples(posY);

    pposz = ui->posteriorZ;
    pposz->setAxisAutoScale(QwtPlot::yLeft, false);
    pposz->setAxisScale(QwtPlot::yLeft, -35, 35, 7);
    pposz->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pposz->setAxisTitle(QwtPlot::yLeft, "rad/s");

    posZCurve = new QwtPlotCurve();
    pposz->setTitle("Gyro Main");
    posZCurve->setPen(Qt::blue, 1);
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    posZCurve->setBrush(brush);
    posZCurve->attach(pposz);
    posZCurve->setSamples(posZ);



    //anterior
    pantx = ui->anteriorX;
    pantx->setAxisAutoScale(QwtPlot::yLeft, false);
    pantx->setAxisScale(QwtPlot::yLeft, -35,35,7);
    pantx->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pantx->setAxisTitle(QwtPlot::yLeft, "rad/s");
    antXCurve = new QwtPlotCurve();
    pantx->setTitle("Gyro Anterior");
    antXCurve->setPen(Qt::red, 1);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    antXCurve->setBrush(brush);
    antXCurve->attach(pantx);
    antXCurve->setSamples(antX);

    panty = ui->anteriorY;
    panty->setAxisAutoScale(QwtPlot::yLeft, false);
    panty->setAxisScale(QwtPlot::yLeft, 0,50,5);
    panty->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    panty->setAxisTitle(QwtPlot::yLeft, "G");
    antYCurve = new QwtPlotCurve();
    panty->setTitle("ADXL2");
    antYCurve->setPen(Qt::black, 1);
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    antYCurve->setBrush(brush);
    antYCurve->attach(panty);
    antYCurve->setSamples(antY);

    pantz = ui->anteriorZ;
    pantz->setAxisAutoScale(QwtPlot::yLeft, false);
    pantz->setAxisScale(QwtPlot::yLeft, 0, 50, 5);
    pantz->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pantz->setAxisTitle(QwtPlot::yLeft, "G");

    antZCurve = new QwtPlotCurve();
    pantz->setTitle("ADXL1");
    antZCurve->setPen(Qt::blue, 1);
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    antZCurve->setBrush(brush);
    antZCurve->attach(pantz);
    antZCurve->setSamples(antZ);

}



