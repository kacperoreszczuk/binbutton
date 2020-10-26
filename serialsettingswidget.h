#ifndef SERIALSETTINGSWIDGET_H
#define SERIALSETTINGSWIDGET_H

#include <QWidget>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QSerialPortInfo>
#include <QIcon>
#include <QMenu>

namespace Ui {
class SerialSettingsWidget;
}

class SerialSettingsWidget : public QWidget
{
    Q_OBJECT

    QMenu *trayIconMenu;
    Ui::SerialSettingsWidget *ui;

public:
    explicit SerialSettingsWidget(QWidget *parent = nullptr);
    ~SerialSettingsWidget();

    QString getPort();
    void setSuccess(int success);
    int saveSettings();
    int loadSettings();

public slots:
    void getSerialPortList();
    void selectSerialPort();
    void connectToSerialPort();

signals:
    void connectingToSerialPort();
};

#endif // SERIALSETTINGSWIDGET_H
