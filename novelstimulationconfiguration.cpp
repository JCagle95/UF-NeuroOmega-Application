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

#include "novelstimulationconfiguration.h"
#include "ui_novelstimulationconfiguration.h"

NovelStimulationConfiguration::NovelStimulationConfiguration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NovelStimulationConfiguration)
{
    ui->setupUi(this);
}

NovelStimulationConfiguration::~NovelStimulationConfiguration()
{
    delete ui;
}

void NovelStimulationConfiguration::setupDefault(QStringList waveformNames, int waveformID, QJsonDocument jsonDocument)
{
    ui->WaveformSelector->addItem("Disable");
    if (waveformNames.length() > 0)
    {
        for (int i = 0; i < waveformNames.length(); i++)
        {
            ui->WaveformSelector->addItem(waveformNames.at(i));
        }
    }
    ui->WaveformSelector->setCurrentIndex(waveformID + 1);

    if (!jsonDocument.isNull())
    {
        this->stimulationJsonDocument = jsonDocument;
        ui->SequenceFilename->setText(this->stimulationJsonDocument.object()["StimulationName"].toString());
    }
}


void NovelStimulationConfiguration::closeEvent(QCloseEvent *event)
{
    QStringList waveNames;
    if (ui->WaveformSelector->count() > 1)
    {
        for (int i = 1; i < ui->WaveformSelector->count(); i++)
        {
            waveNames.append(ui->WaveformSelector->itemText(i));
        }
    }
    emit novelConfigurations(waveNames, ui->WaveformSelector->currentIndex() - 1, stimulationJsonDocument);
}

void NovelStimulationConfiguration::on_chooseNovelWaveform_clicked()
{
    QFileDialog fileSelector(this);
    fileSelector.setFileMode(QFileDialog::ExistingFile);
    fileSelector.setNameFilter(tr("Waveform (*.bin)"));
    fileSelector.setViewMode(QFileDialog::Detail);

    QStringList fileName;
    if (fileSelector.exec())
    {
        fileName = fileSelector.selectedFiles();

        QFile file(fileName.first());
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, "Error", "File open failed", QMessageBox::Close);
            return;
        }

        if (file.size() % 88000 == 0)
        {
            QSettings *applicationConfiguration = new QSettings(QDir::currentPath() + "/defaultSettings.ini", QSettings::IniFormat);
            applicationConfiguration->setValue("LastNovelStimulation", fileName.first());

            char *data = file.readAll().data();
            int16 *stimulationVector = (int16*) data;
            QString wavename = QFileInfo(fileName.first()).fileName().split(".").first();

            int result = LoadWaveToEmbedded(stimulationVector, file.size() / 2, 1, (cChar*) wavename.toStdString().c_str());

            if (result == eAO_OK)
            {
                if (ui->WaveformSelector->count() > 1)
                {
                    ui->WaveformSelector->setItemText(1, wavename);
                }
                else
                {
                    ui->WaveformSelector->addItem(wavename);
                }
            }
            else
            {
                QMessageBox::critical(this, "Error", "Cannot upload waveform", QMessageBox::Close);
            }
        }

        file.close();
    }
}

void NovelStimulationConfiguration::displayNovelWaveform()
{

}

void NovelStimulationConfiguration::on_chooseStimulationSequence_clicked()
{
    QFileDialog fileSelector(this);
    fileSelector.setFileMode(QFileDialog::ExistingFile);
    fileSelector.setNameFilter(tr("Sequence (*.json)"));
    fileSelector.setViewMode(QFileDialog::Detail);

    QStringList fileName;
    if (fileSelector.exec())
    {
        fileName = fileSelector.selectedFiles();

        QFile file(fileName.first());
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, "Error", "File open failed", QMessageBox::Close);
            return;
        }

        QByteArray sequenceData = file.readAll();
        QJsonDocument loadedDocument = QJsonDocument::fromJson(sequenceData);
        if (loadedDocument.isObject())
        {
            QJsonObject stimulationConfiguration = loadedDocument.object();
            if (stimulationConfiguration.contains("StimulationName"))
            {
                QSettings *applicationConfiguration = new QSettings(QDir::currentPath() + "/defaultSettings.ini", QSettings::IniFormat);
                applicationConfiguration->setValue("LastStimulationConfiguration", fileName.first());

                stimulationJsonDocument = loadedDocument;
                ui->SequenceFilename->setText(this->stimulationJsonDocument.object()["StimulationName"].toString());
            }
            else
            {
                QMessageBox::critical(this, "Error", "The JSON file uploaded is not in the right format", QMessageBox::Close);
                return;
            }
        }
        else
        {
            QMessageBox::critical(this, "Error", "The selected file is not a Json Object", QMessageBox::Close);
            return;
        }
    }
}
