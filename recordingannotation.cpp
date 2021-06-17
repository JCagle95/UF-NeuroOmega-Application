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

#include "recordingannotation.h"
#include "ui_recordingannotation.h"

RecordingAnnotation::RecordingAnnotation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordingAnnotation)
{
    ui->setupUi(this);

    QFile file;
    file.setFileName(QDir::currentPath() + "/InterfaceConfigurations.json");
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray configurationData = file.readAll();

    QJsonDocument loadedDocument = QJsonDocument::fromJson(configurationData);
    if (loadedDocument.isObject())
    {
        QJsonObject documentObject = loadedDocument.object();
        if (documentObject.contains("TaskDefinitions")) taskDefinitions = loadedDocument.object()["TaskDefinitions"].toObject();
    }

    if (!taskDefinitions.isEmpty())
    {
        QPushButton* buttons[6] = {ui->RecordingAnnotationSelect, ui->RecordingAnnotationSelect_2, ui->RecordingAnnotationSelect_3,
                                  ui->RecordingAnnotationSelect_4, ui->RecordingAnnotationSelect_5, ui->RecordingAnnotationSelect_6};
        for (int i = 0; i < 6; i++)
        {
            buttons[i]->setVisible(false);
            if (i < taskDefinitions.keys().size())
            {
                buttons[i]->setText(taskDefinitions.keys()[i]);
                buttons[i]->setVisible(true);
            }
        }
    }
}

void RecordingAnnotation::closeEvent()
{
    emit channelIDsUpdate(annotation, QJsonDocument());
}

RecordingAnnotation::~RecordingAnnotation()
{
    delete ui;
}

void RecordingAnnotation::on_RecordingAnnotation_clicked()
{
    QPushButton* buttonClicked = qobject_cast<QPushButton*>(sender());
    annotation = buttonClicked->text();

    QJsonDocument loadedDocument = QJsonDocument();
    if (!taskDefinitions.isEmpty())
    {
        QFile file;
        file.setFileName(QDir::currentPath() + "/" + taskDefinitions[annotation].toString());
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray configurationData = file.readAll();
            loadedDocument = QJsonDocument::fromJson(configurationData);
        }
    }

    emit channelIDsUpdate(annotation, loadedDocument);
    this->close();
}
