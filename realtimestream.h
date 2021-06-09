#ifndef REALTIMESTREAM_H
#define REALTIMESTREAM_H

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>

#include "streamdatahandler.h"

#ifdef QT_DEBUG
#include "AOSystemAPI_TEST.h"
#else
#include "AOSystemAPI.h"
#endif
#include "AOTypes.h"

namespace Ui {
class RealtimeStream;
}

class RealtimeStream : public QMainWindow
{
    Q_OBJECT

public:
    explicit RealtimeStream(QWidget *parent = nullptr);
    ~RealtimeStream();
    void closeEvent(QCloseEvent *event);

    void initializeElectrode(QString electrodeName, int electrodeConfiguration[8]);
    void requestProcsesing(QString processingType, QString header, int16* pData, int size);
    void responseParser(QNetworkReply *reply);
    void updateData(void);

signals:
    void windowClosed(void);

private:
    Ui::RealtimeStream *ui;

    QTimer *periodicDataRequest;
    QElapsedTimer *elapsedTimer;

    QList<QLineSeries*> psdLines;
    QList<CircularBuffer> dataBuffer;

    QNetworkAccessManager *networkManager;
    int electrodeIDs[8];
    int nContacts = 0;
};

#endif // REALTIMESTREAM_H
