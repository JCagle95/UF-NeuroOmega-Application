#include "realtimestream.h"
#include "ui_realtimestream.h"

RealtimeStream::RealtimeStream(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RealtimeStream)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    elapsedTimer = new QElapsedTimer();
    elapsedTimer->start();

    // Setup Network Communication with Python Server
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &RealtimeStream::responseParser);


    // Status Timer for Periodic Connection Checks
    periodicDataRequest = new QTimer(this);
    connect(periodicDataRequest, &QTimer::timeout, this, &RealtimeStream::updateData);
    periodicDataRequest->start(1000);
}

RealtimeStream::~RealtimeStream()
{
    delete ui;
}

void RealtimeStream::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
}

void RealtimeStream::initializeElectrode(QString electrodeName, int electrodeConfiguration[8])
{
    if (electrodeName == "Microelectrode / Macroelectrode")
    {
        ui->ElectrodeNameLabel->setText(electrodeName);
        nContacts = 0;
        for (int i = 0; i < 8; i++)
        {
            this->electrodeIDs[i] = electrodeConfiguration[i];
            if (electrodeConfiguration[i] != 0) nContacts++;
        }

        // Setup Buffer to hold raw data
        QString legendItems[8] = {"Contact 0", "Contact 1.1", "Contact 1.2", "Contact 1.3", "Contact 2.1", "Contact 2.2", "Contact 2.3", "Contact 3"};
        if (nContacts == 4)
        {
            for (int i = 0; i < 4; i++) legendItems[i] = QString("Contact %1").arg(i);
        }

        dataBuffer.clear();
        psdLines.clear();
        for (int i = 0; i < nContacts; i++)
        {
            psdLines.append(new QLineSeries());
            psdLines[i]->setName(legendItems[i]);
            dataBuffer.append(CircularBuffer());
            dataBuffer[i].initiateBuffer(110000);
        }

        // Create Spectral Density Graph
        QChart *chart = new QChart();
        for (int i = 0; i < nContacts; i++)
        {
            chart->addSeries(psdLines[i]);
        }
        chart->createDefaultAxes();
        //ui->SpectralChartView->chart()->setAxisX()
        chart->axisX()->setRange(0, 100);
        chart->axisY()->setRange(0, 10);
        chart->setTitle("Simple line chart example");
        chart->setTitleFont(QFont("Lato", 24));

        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->legend()->setFont(QFont("Lato", 12));

        ui->SpectralChartView->setChart(chart);
        ui->SpectralChartView->setRenderHint(QPainter::Antialiasing);
        chart->setContentsMargins(0, 0, 0, 0);
        chart->setBackgroundRoundness(50);
    }
    else
    {
        ui->ElectrodeNameLabel->setText(electrodeName);
        nContacts = 0;
        for (int i = 0; i < 8; i++)
        {
            this->electrodeIDs[i] = electrodeConfiguration[i];
            if (electrodeConfiguration[i] != 0) nContacts++;
        }

        // Setup Buffer to hold raw data
        QString legendItems[8] = {"Contact 0", "Contact 1.1", "Contact 1.2", "Contact 1.3", "Contact 2.1", "Contact 2.2", "Contact 2.3", "Contact 3"};
        if (nContacts == 4)
        {
            for (int i = 0; i < 4; i++) legendItems[i] = QString("Contact %1").arg(i);
        }

        dataBuffer.clear();
        psdLines.clear();
        for (int i = 0; i < nContacts; i++)
        {
            psdLines.append(new QLineSeries());
            psdLines[i]->setName(legendItems[i]);
            dataBuffer.append(CircularBuffer());
            dataBuffer[i].initiateBuffer(110000);
        }

        // Create Spectral Density Graph
        QChart *chart = new QChart();
        for (int i = 0; i < nContacts; i++)
        {
            chart->addSeries(psdLines[i]);
        }
        chart->createDefaultAxes();
        //ui->SpectralChartView->chart()->setAxisX()
        chart->axisX()->setRange(0, 100);
        chart->axisY()->setRange(0, 10);
        chart->setTitle("Simple line chart example");
        chart->setTitleFont(QFont("Lato", 24));

        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->legend()->setFont(QFont("Lato", 12));

        ui->SpectralChartView->setChart(chart);
        ui->SpectralChartView->setRenderHint(QPainter::Antialiasing);
        chart->setContentsMargins(0, 0, 0, 0);
        chart->setBackgroundRoundness(50);
    }
}

void RealtimeStream::responseParser(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        QJsonDocument document = QJsonDocument::fromJson(response.data());

        if (document.object().contains("PSD") && document.object().contains("Frequency"))
        {
            QJsonArray psdArray = document.object()["PSD"].toArray();
            QJsonArray freqArray = document.object()["Frequency"].toArray();

            QVector<QPointF> psdVector;
            for (int i = 0; i < psdArray.count(); i++)
            {
                psdVector.append(QPointF(freqArray[i].toDouble(), psdArray[i].toDouble()));
            }

            psdLines[document.object()["Channel"].toInt()]->replace(psdVector);
        }
    }
    return;
}

void RealtimeStream::requestProcsesing(QString processingType, QString header, int16* pData, int size)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart dataPart;
    dataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"RawData\""));
    QByteArray DataPackage((char*)pData, size*2);
    dataPart.setBody(DataPackage.toBase64());
    multiPart->append(dataPart);

    QHttpPart headerPart;
    headerPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    headerPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"Header\""));
    headerPart.setBody(header.toStdString().c_str());
    multiPart->append(headerPart);

    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:8000/processLFP/" + processingType));
    QNetworkReply *reply = networkManager->post(request, multiPart);
    multiPart->setParent((QObject*)reply);
}

void RealtimeStream::updateData(void)
{
    int16 *pData = (int16*)malloc(sizeof(int16)*22000*nContacts);
    int numDataCaptured = 0;

    GetAlignedData(pData, 22000*nContacts, &numDataCaptured, this->electrodeIDs, nContacts, NULL);
    for (int i = 0; i < nContacts; i++)
    {
        dataBuffer[i].addBuffer(pData + (i*(numDataCaptured/nContacts)), numDataCaptured / nContacts);
    }
    free(pData);

    if (ui->GraphicTab->currentIndex() == 0)
    {
        elapsedTimer->restart();
        pData = (int16*)malloc(sizeof(int16)*110000);
        for (int i = 0; i < nContacts; i++)
        {
            if (dataBuffer[i].getBuffer(pData, 110000) == 0)
            {
                requestProcsesing("psd", QString("{'Channel': %1, 'SamplingRate': %2}").arg(i).arg(22000), pData, 110000);
            }
        }
        free(pData);
    }
}
