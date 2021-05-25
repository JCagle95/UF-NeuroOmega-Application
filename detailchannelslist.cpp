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

#include "detailchannelslist.h"
#include "ui_detailchannelslist.h"

DetailChannelsList::DetailChannelsList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DetailChannelsList)
{
    ui->setupUi(this);

    ui->AllChannelsTable->setColumnWidth(1, 250);
    ui->AllChannelsTable->setColumnWidth(2, 100);
    ui->AllChannelsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->AllChannelsTable->verticalHeader()->setHidden(true);

    applicationConfiguration = new QSettings(QDir::currentPath() + "/defaultSettings.ini", QSettings::IniFormat);
    this->deploymentMode = applicationConfiguration->value("DeploymentMode").toString().toStdString();
}

DetailChannelsList::~DetailChannelsList()
{
    delete ui;
}

void DetailChannelsList::updateChannelInformation()
{
    uint32 channelCount = 0;
    int result = GetChannelsCount(&channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    SInformation channelsInfo[channelCount];
    result = GetAllChannels(channelsInfo, channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    for (int i = 0; i < channelCount; i++)
    {
        if (ui->AllChannelsTable->item(i, 0)->text() != QString(channelsInfo[i].channelName))
        {
            SetChannelName(channelsInfo[i].channelID, (char*)ui->AllChannelsTable->item(i, 1)->text().toStdString().c_str(), ui->AllChannelsTable->item(i, 1)->text().length());
        }

        auto field = ui->AllChannelsTable->cellWidget(i, 2);
        bool checkState = qobject_cast<QCheckBox*> (field)->isChecked();
        if (checkState != channelsSaveStates[i])
        {
            SetChannelSaveState(channelsInfo[i].channelID, checkState);
        }
    }

    setupChannels();
}

QString DetailChannelsList::getErrorLog()
{
    char errorString[1000] = {0};
    int nErrorCount = 0;
    int result = ErrorHandlingfunc(&nErrorCount, errorString, 1000);

    switch (result)
    {
        case eAO_ARG_NULL:
            printf("ErrorHandlingfunc: NULL Argument Error\n");
            break;
        case eAO_BAD_ARG:
            printf("ErrorHandlingfunc: Bad Argument\n");
            break;
    }

    return QString(errorString);
}

void DetailChannelsList::displayError(int errorLevel, QString message)
{
    if (errorLevel == QMessageBox::Critical)
    {
        QMessageBox::critical(this, "Error", message, QMessageBox::Close);
    }
    else if (errorLevel == QMessageBox::Warning)
    {
        QMessageBox::warning(this, "Warning", message, QMessageBox::Close);
    }
}

void DetailChannelsList::setupChannels()
{

    uint32 channelCount;
    SInformation *channelsInfo;

    channelCount = 0;
    int result = GetChannelsCount(&channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    channelsInfo = new SInformation[channelCount];
    result = GetAllChannels(channelsInfo, channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    int rowCount = ui->AllChannelsTable->rowCount();
    for (int i = 0; i < rowCount; i++)
    {
        ui->AllChannelsTable->removeRow(0);
    }

    for (int i = 0; i < channelCount; i++)
    {
        ui->AllChannelsTable->insertRow(ui->AllChannelsTable->rowCount());

        ui->AllChannelsTable->setItem(ui->AllChannelsTable->rowCount() - 1, 0, new QTableWidgetItem(QString::number(channelsInfo[i].channelID)));
        ui->AllChannelsTable->item(ui->AllChannelsTable->rowCount() - 1, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->AllChannelsTable->item(ui->AllChannelsTable->rowCount() - 1, 0)->setTextAlignment(Qt::AlignHCenter);

        ui->AllChannelsTable->setItem(ui->AllChannelsTable->rowCount() - 1, 1, new QTableWidgetItem(channelsInfo[i].channelName));

        int saveState = 0;
        QCheckBox *checkbox = new QCheckBox();
        GetChannelSaveState(channelsInfo[i].channelID, &saveState);
        channelsSaveStates << saveState;
        checkbox->setChecked(saveState);

        ui->AllChannelsTable->setCellWidget(ui->AllChannelsTable->rowCount() - 1, 2, checkbox);
        ui->AllChannelsTable->cellWidget(ui->AllChannelsTable->rowCount() - 1, 2)->setStyleSheet("margin-left:40%; margin-right:60%;");
    }
}

void DetailChannelsList::on_UpdateChannelInformation_clicked()
{
    updateChannelInformation();
}

void DetailChannelsList::on_ResetChannelInformation_clicked()
{
    uint32 channelCount = 0;
    int result = GetChannelsCount(&channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    SInformation channelsInfo[channelCount];
    result = GetAllChannels(channelsInfo, channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    for (int i = 0; i < channelCount; i++)
    {
        if (channelsInfo[i].channelID >= 10272 && channelsInfo[i].channelID <= 10335)
        {
            int boxID = (channelsInfo[i].channelID - 10272) / 16;
            int channelID = (channelsInfo[i].channelID - 10272) % 16;
            QString defaultChannelName = "ECOG HF " + QString::number(boxID+1) + " / " + QStringLiteral("%1").arg(channelID+1, 2, 10, QLatin1Char('0')) + " - Array " + QString::number(boxID+1) + " / " + QStringLiteral("%1").arg(channelID+1, 2, 10, QLatin1Char('0'));
            SetChannelName(channelsInfo[i].channelID, (char*)defaultChannelName.toStdString().c_str(), defaultChannelName.length());
        }
    }

    setupChannels();
}
