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

#ifndef ELECTRODECONFIGURATIONS_H
#define ELECTRODECONFIGURATIONS_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLayout>

#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "jsonstorage.h"
#include "channelselectiondialog.h"

namespace Ui {
class ElectrodeConfigurations;
}

typedef struct ElectrodeConfigurationUIWidgets
{
    QLabel *leadName;
    QComboBox *electrodeName;
    QComboBox *hemisphereDefinition;
    QComboBox *targetDefinition;
    QPushButton *channelsSelection;
} ElectrodeConfigurationUIWidgets;

typedef struct ElectrodeInformation
{
    QString electrodeType = "";
    QString hemisphere = "";
    QString target = "";
    QVector<int> channelIDs;
    int layoutSize[2] = {0};
    int numContacts = 0;
    bool verified = false;
} ElectrodeInformation;

class ElectrodeConfigurations : public QDialog
{
    Q_OBJECT

public:
    explicit ElectrodeConfigurations(QWidget *parent = nullptr);
    ~ElectrodeConfigurations();

    void addNewElectrodeRow(int rowID);
    void displayError(int errorLevel, QString message);

    QList<ElectrodeInformation> electrodeInfoCollection;

private slots:
    void on_AddElectrodes_clicked();

    void on_electrodeNameChanged();
    void on_channelConfiguration();
    void updateChannelConfiguration(int *channelIDs, int len, int electrodeID);

    bool verifyConfigurations();
    void on_ChannelConfirm_clicked();

    void on_SaveChannelConfig_clicked();

    void on_LoadChannelConfig_clicked();

private:
    Ui::ElectrodeConfigurations *ui;
    QGridLayout *scrollAreaLayout;
    QList<ElectrodeConfigurationUIWidgets> electrodeWidgetsCollection;

    QJsonObject interfaceConfiguration;
    QStringList availableElectrodeNames;
    QStringList availableTargetNames;
};

#endif // ELECTRODECONFIGURATIONS_H
