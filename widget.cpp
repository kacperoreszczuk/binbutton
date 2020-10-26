#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    serialSettingsWidget = new SerialSettingsWidget();
    connect(serialSettingsWidget, SIGNAL(connectingToSerialPort()), this, SLOT(connectToSerialPort()));

    flatButtons = 0;

    trayIconMenu = new QMenu(this);
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
    trayIconMenu->addAction(quitAction);

    QIcon icon(":/img/icon.png");
    setWindowIcon(icon);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    connectToSerialPort();

    clipboard = QApplication::clipboard();
    connect(ui->buttonToE, SIGNAL(pressed()), this, SLOT(toE()));
    connect(ui->buttonBin, SIGNAL(pressed()), this, SLOT(bin2()));
    connect(ui->buttonFlatten, SIGNAL(pressed()), this, SLOT(flatten()));
    connect(ui->buttonTranspose, SIGNAL(pressed()), this, SLOT(transpose()));
    connect(ui->buttonReverse, SIGNAL(pressed()), this, SLOT(reverse()));
    connect(ui->buttonSave, SIGNAL(pressed()), this, SLOT(save()));
    connect(ui->buttonSubstract, SIGNAL(pressed()), this, SLOT(subtract()));
    connect(ui->buttonAdd, SIGNAL(pressed()), this, SLOT(add()));
    connect(ui->buttonDivide, SIGNAL(pressed()), this, SLOT(divide()));
    connect(ui->buttonMultiply, SIGNAL(pressed()), this, SLOT(multiply()));
    connect(ui->buttonSerialSetup, SIGNAL(pressed()), this, SLOT(showSerialSetup()));

    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
}


Widget::~Widget()
{
    delete ui;
}


void Widget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        this->show();
        this->raise();
        this->activateWindow();
    }
}


void Widget::closeEvent(QCloseEvent *event)
{
    serialSettingsWidget->hide();
    event->accept();
}


[[ noreturn ]] void Widget::quit()
{
   exit(0);
}


void Widget::showSerialSetup() // TODO: try to connect directly?
{
    serialSettingsWidget->show();
}


void Widget::connectToSerialPort()
{
    if(serialPort)
        delete serialPort;

#ifdef __linux__
    QString portName = serialSettingsWidget->getPort();
#else
    // Without this hack qt had problem connecting to COM ports with high numbers in windows
    QString portName = "\\\\.\\" + serialSettingsWidget->getPort();
#endif

    serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(19200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    int success = serialPort->open(QIODevice::ReadWrite);
    serialSettingsWidget->setSuccess(success);
    if (success)
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(readFromSerialPort()));
}


void Widget::readFromSerialPort()
{
    if(serialPort)
    {
        auto byteCount = serialPort->bytesAvailable();
        char* bytes = new char[static_cast<unsigned long long>(byteCount)];
        char reply[1];
        serialPort->read(bytes, byteCount);
        for(unsigned int i = 0; i < byteCount; i++)
        {
            if(bytes[i] < 'a' || bytes[i] > 'i')
                continue;  // Invalid operation

            // respond with same byte to turn on a LED on physical keyboard
            reply[0] = bytes[i];
            serialPort->write(reply, 1);
            serialPort->waitForBytesWritten(500);

            switch(bytes[i])
            {
            case 'a':
                save();
                break;
            case 'b':
                divide();
                break;
            case 'c':
                toE();
                break;
            case 'd':
                subtract();
                break;
            case 'e':
                add();
                break;
            case 'f':
                bin2();
                break;
            case 'g':
                reverse();
                break;
            case 'h':
                flatten();
                break;
            case 'i':
                transpose();
                break;
            }

            // Operation is finished. Turn off LED
            reply[0] = 'x';
            serialPort->write(reply,1);
            serialPort->waitForBytesWritten(500);
        }
        delete[] bytes;
    }
}


QVector< QVector<double> > Widget::readFromClipboard(int &firstRowLength, int &secondRowLength, int &dataBeginRow)
{
    QVector< QVector<double> > resultData;
    QString inputString = clipboard->text();
    QTextStream inputStream (&inputString);
    QString rowString;
    int currentRow = 0;
    firstRowLength = 0;
    secondRowLength = 0;

    while(!inputStream.atEnd())
    {
        rowString = inputStream.readLine();
        QTextStream rowStream (&rowString);
        QVector<double> rowData;
        double element;

        while(1)
        {
            rowStream >> element;
            if(rowStream.status() == QTextStream::ReadCorruptData ||
               rowStream.status() == QTextStream::ReadPastEnd )
                break;
            rowData.append(element);
        }

        if(rowData.size() == 0)
            continue;  // just ignore all blank or invalid lines
        if(currentRow == 0)
        {
            firstRowLength = rowData.size();
        }
        if(currentRow == 1)
        {
            secondRowLength = rowData.size();
            if(secondRowLength != firstRowLength + 1)
                secondRowLength = firstRowLength;  // Second row longer than expected. Trim to rectangle and continue
        }
        if(currentRow >= 1)
        {
            if(rowData.size() < secondRowLength)
                continue; // Row too short. ignore it completely and contiunue reading

            if(rowData.size() > secondRowLength)
                rowData.resize(secondRowLength);  // Row too long. Trim and continue
        }

        resultData.append(rowData);
        currentRow++;
    }

    dataBeginRow = (secondRowLength > firstRowLength ? 1 : 0);

    return resultData;
}


void Widget::writeToClipboard(QVector< QVector<double> > outputData)
{
    QString outputString;
    QTextStream outputStream(&outputString);
    outputStream.setRealNumberPrecision(12);

    for (int row = 0; row < outputData.size(); row++)
    {
        if(outputData[row].size() > 0)
            outputStream << outputData[row][0];
        for(int col = 1; col < outputData[row].size(); col++)
        {
            outputStream << "\t" << outputData[row][col];
        }
        outputStream << "\n";
    }

    clipboard->setText(outputString);
}


void Widget::toE()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);

    for(int i = dataBeginRow; i < inputData.size() ; i++)
        inputData[i][0] = 1239495.2775 / inputData[i][0];

    writeToClipboard(inputData);
}


void Widget::flatten()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);

    int dataBeginColumn = dataBeginRow;
    for(int i = dataBeginRow; i < inputData.size(); i++)
    {
        for( int j = dataBeginColumn + 1; j < dataBeginColumn + firstRowLength; j++)
        {
            inputData[i][1] += inputData[i][j];
        }
        inputData[i].resize(dataBeginColumn + 1);
    }
    if(dataBeginRow == 1)
        inputData.removeFirst();
    writeToClipboard(inputData);
}


void Widget::transpose()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);
    QVector< QVector<double> > outputData;
    if (inputData.size() < 1) return;

    if(dataBeginRow == 1)
        inputData[0].push_front(0);  // Make data array rectangular

    for( int j = 0; j < secondRowLength; j++)
    {
        outputData.push_back(QVector<double>());
        for(int i = 0; i < inputData.size(); i++)
        {
            outputData.back().push_back(inputData[i][j]);
        }
    }
    if(dataBeginRow == 1)
    {
        outputData[0].removeFirst();
    }
    writeToClipboard(outputData);
}


void Widget::reverse()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);

    if (inputData.size() < 2)
        return;

    if(dataBeginRow == 1)
        inputData.push_back(inputData[0]);

    std::reverse(inputData.begin(), inputData.end());

    if(dataBeginRow == 1)
    {
        inputData.pop_back();
    }

    writeToClipboard(inputData);
}


void Widget::bin2()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);
    if (inputData.size() < 2) return;

    for (int i = 0; 2 * i + dataBeginRow + 1 < inputData.size(); i++)
    {
        for(int j = 0 ; j < secondRowLength; j++)
        {
            inputData[i + dataBeginRow][j] = 0.5 * (inputData[i * 2 + dataBeginRow][j] +
                                            inputData[i * 2 + dataBeginRow + 1][j]);
        }
    }
    inputData.resize((inputData.size() - dataBeginRow) / 2 + dataBeginRow);
    writeToClipboard(inputData);
}


void Widget::save()
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);

    if(inputData.size() == 0)
        return;

    background = inputData;
    backgroundFirstRowLength = firstRowLength;
    backgroundSecondRowLength = secondRowLength;
    backgroundDataBeginRow = dataBeginRow;
}


double operationSubtract(double a, double b) {return a - b;}
double operationAdd(double a, double b) {return a + b;}
double operationDivide(double a, double b) {return a / b;}
double operationMultiply(double a, double b) {return a * b;}

void Widget::subtract() {performOperation(&operationSubtract);}
void Widget::add() {performOperation(&operationAdd);}
void Widget::divide() {performOperation(&operationDivide);}
void Widget::multiply() {performOperation(&operationMultiply);}


void Widget::performOperation(double (*operation)(double, double))
{
    int firstRowLength, secondRowLength, dataBeginRow;
    QVector< QVector<double> > inputData = readFromClipboard(firstRowLength, secondRowLength, dataBeginRow);
    if (inputData.size() < 1) return;

    if(inputData.size() == 1 && background.size() == 1 && background[0].size() == inputData[0].size())
    {
        // in this case background and data are a single-row datasets

        for(int i = 0; i < inputData[0].size(); i++)
        {
            inputData[0][i] = operation(inputData[0][i], background[0][i]);
        }
    }
    else if(backgroundSecondRowLength >= 3)
    {
        // in this case background has 2D shape

        // we can correct our guess of the intended background data shape if we compare it with to-be-modified data
        if(firstRowLength == secondRowLength && inputData.size() - 1 == background.size())
            dataBeginRow = 1;
        int backgroundDataBeginRowHere = backgroundDataBeginRow;  // copy, to preserve value of the initial guess
        if(backgroundFirstRowLength == backgroundSecondRowLength && background.size() - 1 == inputData.size())
            backgroundDataBeginRowHere = 1;

        if(inputData.size() - dataBeginRow == background.size() - backgroundDataBeginRowHere &&  // equal height
                std::abs(secondRowLength - backgroundSecondRowLength) <= 1)  // equal width (+-1)
        {
            int fromSecondColumn, backgroundFromSecondColumn;
            if(backgroundSecondRowLength > secondRowLength)
            {
                // if background has an extra first column, ignore it
                backgroundFromSecondColumn = 1;
                fromSecondColumn = 0;
            }
            else if(backgroundSecondRowLength < secondRowLength)
            {
                // if data has an extra first column, ignore it
                backgroundFromSecondColumn = 0;
                fromSecondColumn = 1;
            }
            else
            {
                // equal widths, check if they have first 'X' column and first 'Y' row to be ignored
                if(dataBeginRow == 1 || backgroundDataBeginRowHere == 1)
                {
                    backgroundFromSecondColumn = 1;
                    fromSecondColumn = 1;
                }
                else
                {
                    backgroundFromSecondColumn = 0;
                    fromSecondColumn = 0;
                }
            }

            // do the operation, ignoring first rows and first columns where applicable
            for(int i = 0; i < inputData.size() - dataBeginRow; i++)
            {
                for(int j = 0; j < secondRowLength - fromSecondColumn; j++)
                {
                    inputData[i + dataBeginRow][j + fromSecondColumn] =
                            operation(inputData[i + dataBeginRow][j + fromSecondColumn],
                                      background[i + backgroundDataBeginRow][j + backgroundFromSecondColumn]);
                }
            }
        }
    }
    else if(backgroundDataBeginRow == 0 && (backgroundFirstRowLength == 1 || backgroundFirstRowLength == 2) &&
            inputData.size() - dataBeginRow == background.size())
    {
        // if background is a single- or a two-column (XY) dataset, subtract background column from all data columns

        int notsingle = inputData.back().size() > 1;

        for(int i = dataBeginRow; i < inputData.size(); i++)
        {
            for(int j = notsingle; j < secondRowLength; j++)
            {
                inputData[i][j] = operation(inputData[i][j], background[i - dataBeginRow].last());
            }
        }
    }
    else if(background.size() == 1 && background[0].size() == 1)
    {
        // in this case background is a single value

        int notsingle = inputData.back().size() > 1;

        for(int i = dataBeginRow; i < inputData.size(); i++)
        {
            for(int j = notsingle; j < secondRowLength; j++)
            {
                inputData[i][j] = operation(inputData[i][j], background[0][0]);
            }
        }
    }
    else
        return;  // wrong shape, ignore and return

    writeToClipboard(inputData);
}

