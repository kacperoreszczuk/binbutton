#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QVector>
#include <QClipboard>
#include <QTextStream>
#include <QMessageBox>
#include <QFile>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <iostream>

#include "serialsettingswidget.h"

namespace Ui {
class Widget;
}

double operationSubtract(double a, double b);
double operationAdd(double a, double b);
double operationDivide(double a, double b);

class Widget : public QWidget
{
    Q_OBJECT

    SerialSettingsWidget *serialSettingsWidget;

    QClipboard *clipboard;

    QVector< QVector<double> > readFromClipboard(int &firstRowLength, int &secondRowLength, int &dataBeginRow);
    void writeToClipboard(QVector< QVector<double> > data);

    QVector< QVector<double> > background;
    int backgroundFirstRowLength, backgroundSecondRowLength, backgroundDataBeginRow;

    QSerialPort* serialPort = nullptr;
    QSystemTrayIcon * trayIcon;
    QIcon *icon;
    QAction *quitAction;
    QMenu *trayIconMenu;

    void performOperation(double (*operation)(double, double));

    bool flatButtons;

public:
    explicit Widget(QWidget *parent=nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    void closeEvent(QCloseEvent *event);

public slots:

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    [[ noreturn ]] void quit();

    void bin2();
    void toE();
    void flatten();
    void transpose();
    void reverse();
    void save();
    void subtract();
    void add();
    void divide();
    void multiply();

    void readFromSerialPort();
    void connectToSerialPort();
    void showSerialSetup();
};
#endif // WIDGET_H





































