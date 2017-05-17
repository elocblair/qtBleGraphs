#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include "shockclockreader.h"
#include <QThread>
#include <QFile>
#include <QTextEdit>
#include <QtSerialPort/QSerialPort>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initializePlots();
    QTextEdit *gyronum;
    QSerialPort* serial;
    QString readData;
    //variables
    QPolygonF posX, posY, posZ, antX, antY, antZ, vecX, vecY, vecZ;
    QwtPlotCurve *posXCurve, *posYCurve, *posZCurve, *antXCurve, *antYCurve, *antZCurve, *vecXCurve, *vecYCurve, *vecZCurve;
    QwtPlot *pposx, *pposy, *pposz, *pantx, *panty, *pantz, *pvecx, *pvecy, *pvecz;
    QVector<double> motorEncoder, freeEncoder, mainGyro, anteriorGyro, anteriorADXL, mainADXL, xvec, yvec, zvec;
    ShockClockReader *reader;
    QThread *scThread;
    int showSamplesCount;
    QFile *file;
    QString saveFileLocation;
    QTextStream *stream;
    bool recording;
    QString pressedKey;
private:
    Ui::MainWindow *ui;
public slots:
    void receiveReading(QVector<double> values);
    void saveData();
    void ready2read();
protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // MAINWINDOW_H
