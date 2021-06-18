/*******************************************************************************
Copyright (c) 2021, Jackson Cagle, University of Floria

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*********************************************************************************/

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

    void initializeElectrode(QString electrodeName, QVector<int> electrodeConfiguration);
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
