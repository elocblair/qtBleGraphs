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


void MainWindow::receiveReading(QVector<double> values)
{
    int xcount = 0;
    int ycount = 0;

    for(xcount; xcount < (int)values.at(0); xcount++){

        if((values.at(xcount+2)<36) & (values.at(xcount+2)>-36) ){
            xpos.push_back(values.at(xcount+2));
           // qDebug() << "x data: " << xpos.at(xcount);
        }
    }
    //qDebug() << "x\n " << xpos;
    for(ycount; ycount < (int)values.at(1); ycount++){
        int index = xcount+ycount+2;

        if( (values.at(index) < 36) & (values.at(index) > -36)){

            ypos.push_back(values.at(index));
            //qDebug() << "y data: " << ypos.at(ycount) << "\n";
        }
    }
    //qDebug() << "y\n " << ypos;
    //qDebug() << values.size();
    //xpos.push_back(values.at(0));
    //ypos.push_back(values.at(1));
    zpos.push_back(values.at(xcount + ycount+2));
    xant.push_back(values.at(xcount + ycount+3));
    yant.push_back(values.at(xcount + ycount+4));
    zant.push_back(values.at(xcount + ycount+5));
    /*xvec.push_back(values.at(6));
    yvec.push_back(values.at(7));
    zvec.push_back(values.at(8));*/

    if(recording == true)
    {
        QString string = pressedKey + QString::number(values.at(0)) + ", " + QString::number(values.at(1)) + ", " + QString::number(values.at(2)) + ", " + QString::number(values.at(3)) + ", " + QString::number(values.at(4)) + ", " + QString::number(values.at(5)) + "\r\n";
        *stream << string;

        stream->flush();
        file->flush();
        pressedKey = "";
    }
    //make sure that there aren't too many values
    while(xpos.size() > showSamplesCount)
    {
        xpos.pop_front();

    }
    while(ypos.size() > showSamplesCount)
    {
        ypos.pop_front();
    }
    while(zpos.size() > showSamplesCount)
    {
        zpos.pop_front();
    }

    while(xant.size() > showSamplesCount)
    {
        xant.pop_front();
    }
    while(yant.size() > showSamplesCount)
    {
        yant.pop_front();
    }
    while(zant.size() > showSamplesCount)
    {
        zant.pop_front();
    }

    /*while(xvec.size() > showSamplesCount)
    {
        xvec.pop_front();
    }
    while(yvec.size() > showSamplesCount)
    {
        yvec.pop_front();
    }
    while(zvec.size() > showSamplesCount)
    {
        zvec.pop_front();
    }*/
    posX.clear();
    posY.clear();
    posZ.clear();

    antX.clear();
    antY.clear();
    antZ.clear();

   /* vecX.clear();
    vecY.clear();
    vecZ.clear();*/

    for(int i = 0; i < xpos.size(); i ++)
    {

        posX << QPointF(i, xpos.at(i));
        //posY << QPointF(i, ypos.at(i));
        //posZ << QPointF(i, zpos.at(i));

        //antX << QPointF(i, xant.at(i));
        //antY << QPointF(i, yant.at(i));
        //antZ << QPointF(i, zant.at(i));

        /*vecX << QPointF(i, xvec.at(i));
        vecY << QPointF(i, yvec.at(i));
        vecZ << QPointF(i, zvec.at(i));*/
    }

    for(int i = 0; i < ypos.size(); i++){
        posY << QPointF(i, ypos.at(i));
    }

    for(int i = 0; i < zpos.size(); i++){
        posZ << QPointF(i, zpos.at(i));
    }
    for(int i = 0; i < xant.size(); i++){
        antX << QPointF(i, xant.at(i));
    }
    for(int i = 0; i < yant.size(); i++){
        antY << QPointF(i, yant.at(i));
    }
    for(int i = 0; i < zant.size(); i++){
        antZ << QPointF(i, zant.at(i));
    }
    //set the vaues for the posterior


    posXCurve->setSamples(posX);
    posYCurve->setSamples(posY);
    posZCurve->setSamples(posZ);

    antXCurve->setSamples(antX);
    antYCurve->setSamples(antY);
    antZCurve->setSamples(antZ);

    /*vecXCurve->setSamples(vecX);
    vecYCurve->setSamples(vecY);
    vecZCurve->setSamples(vecZ);*/

    pposx->replot();
    pposy->replot();
    pposz->replot();

    pantx->replot();
    panty->replot();
    pantz->replot();

   /* pvecx->replot();
    pvecy->replot();
    pvecz->replot();*/


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
    pposz->setAxisTitle(QwtPlot::yLeft, "radians/s");

    posZCurve = new QwtPlotCurve();
    pposz->setTitle("Gyro Difference");
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
    pantx->setAxisTitle(QwtPlot::yLeft, "radians/s");
    antXCurve = new QwtPlotCurve();
    pantx->setTitle("Gyro");
    antXCurve->setPen(Qt::red, 1);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    antXCurve->setBrush(brush);
    antXCurve->attach(pantx);
    antXCurve->setSamples(antX);

    panty = ui->anteriorY;
    panty->setAxisAutoScale(QwtPlot::yLeft, false);
    panty->setAxisScale(QwtPlot::yLeft, -200,200,25);
    panty->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    panty->setAxisTitle(QwtPlot::yLeft, "G");
    antYCurve = new QwtPlotCurve();
    panty->setTitle("ADXL1");
    antYCurve->setPen(Qt::black, 1);
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    antYCurve->setBrush(brush);
    antYCurve->attach(panty);
    antYCurve->setSamples(antY);

    pantz = ui->anteriorZ;
    pantz->setAxisAutoScale(QwtPlot::yLeft, false);
    pantz->setAxisScale(QwtPlot::yLeft, -200, 200, 25);
    pantz->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);
    pantz->setAxisTitle(QwtPlot::yLeft, "G");

    antZCurve = new QwtPlotCurve();
    pantz->setTitle("ADXL2");
    antZCurve->setPen(Qt::blue, 1);
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    antZCurve->setBrush(brush);
    antZCurve->attach(pantz);
    antZCurve->setSamples(antZ);



    //vector
   /* pvecx = ui->vectorX;
    pvecx->setAxisAutoScale(QwtPlot::yLeft, false);
    pvecx->setAxisScale(QwtPlot::yLeft, -5000, 5000, 1000);
    pvecx->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);

    vecXCurve = new QwtPlotCurve();
    pvecx->setTitle("magnetometer X Values");
    vecXCurve->setPen(Qt::red, 1);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    vecXCurve->setBrush(brush);
    vecXCurve->attach(pvecx);
    vecXCurve->setSamples(vecX);

    pvecy = ui->vectorY;
    pvecy->setAxisAutoScale(QwtPlot::yLeft, false);
    pvecy->setAxisScale(QwtPlot::yLeft, -5000, 5000, 1000);
    pvecy->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);

    vecYCurve = new QwtPlotCurve();
    pvecy->setTitle("magnetometer Y Values");
    vecYCurve->setPen(Qt::black, 1);
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    vecYCurve->setBrush(brush);
    vecYCurve->attach(pvecy);
    vecYCurve->setSamples(vecY);

    pvecz = ui->vectorZ;
    pvecz->setAxisAutoScale(QwtPlot::yLeft, false);
    pvecz->setAxisScale(QwtPlot::yLeft, -5000, 5000, 1000);
    pvecz->setAxisScale(QwtPlot::xBottom, 0, showSamplesCount, 100);

    vecZCurve = new QwtPlotCurve();
    pvecz->setTitle("magnetometer X Values");
    vecZCurve->setPen(Qt::blue, 1);
    brush.setColor(Qt::blue);
    brush.setStyle(Qt::SolidPattern);
    vecZCurve->setBrush(brush);
    vecZCurve->attach(pvecz);
    vecZCurve->setSamples(vecZ);*/


}



