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

#ifndef NOVELSTIMULATIONCONFIGURATION_H
#define NOVELSTIMULATIONCONFIGURATION_H

#include <QFile>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "AOSystemAPI_TEST.h"

namespace Ui {
class NovelStimulationConfiguration;
}

class NovelStimulationConfiguration : public QDialog
{
    Q_OBJECT

public:
    explicit NovelStimulationConfiguration(QWidget *parent = 0);
    ~NovelStimulationConfiguration();
    void closeEvent(QCloseEvent *event);
    void setupDefault(QStringList waveformNames, int waveformID, QJsonDocument jsonDocument);

signals:
    void novelConfigurations(QStringList waveformNames, int waveformID, QJsonDocument jsonDocument);

private slots:
    void on_chooseNovelWaveform_clicked();
    void on_chooseStimulationSequence_clicked();

private:
    Ui::NovelStimulationConfiguration *ui;
    QString deploymentMode;
    QString novelWaveformURL;

    QJsonDocument stimulationJsonDocument;
    QString sequenceJsonName;
};

#endif // NOVELSTIMULATIONCONFIGURATION_H
