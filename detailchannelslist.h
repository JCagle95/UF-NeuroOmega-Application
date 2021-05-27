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

#ifndef DETAILCHANNELSLIST_H
#define DETAILCHANNELSLIST_H

#include <QDialog>
#include <QString>
#include <QtCore>
#include <QCheckBox>
#include <QMessageBox>

#include <cstring>

#include "AOSystemAPI_TEST.h"
#include "AOTypes.h"

using namespace std;

namespace Ui {
class DetailChannelsList;
}

class DetailChannelsList : public QDialog
{
    Q_OBJECT

public:
    explicit DetailChannelsList(QWidget *parent = 0);
    ~DetailChannelsList();

    QString getErrorLog();
    void displayError(int errorLevel, QString message);
    void setupChannels();
    void updateChannelInformation();

private slots:
    void on_UpdateChannelInformation_clicked();

    void on_ResetChannelInformation_clicked();

private:
    Ui::DetailChannelsList *ui;
    QList<int> channelsSaveStates;

    QSettings *applicationConfiguration;
    string deploymentMode;
};

#endif // DETAILCHANNELSLIST_H
