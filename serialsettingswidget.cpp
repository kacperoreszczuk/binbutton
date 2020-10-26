#include "serialsettingswidget.h"
#include "ui_serialsettingswidget.h"

SerialSettingsWidget::SerialSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialSettingsWidget)
{
    ui->setupUi(this);
    getSerialPortList();
    if (!loadSettings())
        saveSettings();

    QIcon icon(":/img/icon.png");
    setWindowIcon(icon);

    connect(ui->buttonConnect, SIGNAL(pressed()), this, SLOT(connectToSerialPort()));
    connect(ui->buttonRefresh, SIGNAL(pressed()), this, SLOT(getSerialPortList()));
    connect(ui->buttonSelect, SIGNAL(pressed()), this, SLOT(selectSerialPort()));

    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
}

SerialSettingsWidget::~SerialSettingsWidget()
{
    delete ui;
}


int SerialSettingsWidget::saveSettings()
{
    QFile file("config.cfg");
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << ui->editPort->text() << "\n";
        stream.flush();
        file.close();
        return 0;
    }
    return -1;
}


int SerialSettingsWidget::loadSettings()
{
    QFile file("config.cfg");
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString port;
        stream >> port;
        ui->editPort->setText(port);
        file.close();
        return 0;
    }
    return -1;
}


void SerialSettingsWidget::getSerialPortList()
{
    QString currentPort = ui->comboPorts->currentText();
    bool deleted = 1;

    ui->comboPorts->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if(info.portName() == currentPort)
            deleted=0;

        ui->comboPorts->addItem(info.portName());
    }
    if(!deleted)
        ui->comboPorts->setCurrentText(currentPort);
}


void SerialSettingsWidget::selectSerialPort()
{
    ui->editPort->setText(ui->comboPorts->currentText());
}


void SerialSettingsWidget::connectToSerialPort()
{
    emit connectingToSerialPort();
}


void SerialSettingsWidget::setSuccess(int success)
{
    if (success)
    {
        ui->buttonConnect->setText("Reconnect");
        ui->buttonConnect->setStyleSheet("background-color: #99ff99");
        saveSettings();
    }
    else
    {
        ui->buttonConnect->setText("Connect");
        ui->buttonConnect->setStyleSheet("background-color: #ff9980");
    }
}


QString SerialSettingsWidget::getPort()
{
    return ui->editPort->text();
}
