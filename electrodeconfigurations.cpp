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

#include "electrodeconfigurations.h"
#include "ui_electrodeconfigurations.h"

ElectrodeConfigurations::ElectrodeConfigurations(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ElectrodeConfigurations)
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
        if (documentObject.contains("ElectrodeDefinitions")) interfaceConfiguration = loadedDocument.object();
    }

    // Load Electrode Names
    availableElectrodeNames.append("None");
    QJsonArray electrodeDefinitions = interfaceConfiguration["ElectrodeDefinitions"].toArray();
    for (int i = 0; i < electrodeDefinitions.size(); i++)
    {
        availableElectrodeNames.append(electrodeDefinitions[i].toObject()["ElectrodeName"].toString());
        for (int j = 0; j < 2; j++) electrodeDefinitions[i].toObject()["layoutSize"].toArray()[j].toInt();
    }

    // Load Target Names
    QJsonArray targetDefinitions = interfaceConfiguration["TargetDefinitions"].toArray();
    for (int i = 0; i < targetDefinitions.size(); i++)
    {
        availableTargetNames.append(targetDefinitions[i].toString());
    }

    scrollAreaLayout = new QGridLayout();
    ui->ScrollAreaWidget->setLayout(scrollAreaLayout);

    QFont headerFont("Ubuntu", 14, QFont::Bold);
    QStringList headerText = {"ID", "Electrode Type", "Hemisphere", "Target", "Channels"};
    QLabel *label;

    // Headers
    for (int i = 0; i < 5; i++)
    {
        label = new QLabel();
        label->setText(headerText[i]);
        label->setFont(headerFont);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setMinimumHeight(30);
        label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
        scrollAreaLayout->addWidget(label, 0, i);
    }

    // Initial Buttons
    for (int i = 1; i < 5; i++)
    {
        addNewElectrodeRow(i);
    }

    scrollAreaLayout->setColumnStretch(0, 2);
    scrollAreaLayout->setColumnStretch(1, 3);
    scrollAreaLayout->setColumnStretch(2, 1);
    scrollAreaLayout->setColumnStretch(3, 2);
    scrollAreaLayout->setColumnStretch(4, 2);

}

ElectrodeConfigurations::~ElectrodeConfigurations()
{
    delete ui;
}

void ElectrodeConfigurations::displayError(int errorLevel, QString message)
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

void ElectrodeConfigurations::addNewElectrodeRow(int rowID)
{
    ElectrodeInformation information;
    ElectrodeConfigurationUIWidgets widgets;
    QFont standardFont("Microsoft YaHei UI", 12);

    widgets.leadName = new QLabel();
    widgets.leadName->setText(QString("Lead #%1").arg(rowID));
    widgets.leadName->setFont(standardFont);
    widgets.leadName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    widgets.leadName->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.leadName, rowID, 0);

    widgets.electrodeName = new QComboBox();
    widgets.electrodeName->addItems(availableElectrodeNames);
    widgets.electrodeName->setFont(standardFont);
    widgets.electrodeName->setMinimumHeight(30);
    widgets.electrodeName->setProperty("rowID", rowID);
    widgets.electrodeName->connect(widgets.electrodeName, &QComboBox::currentIndexChanged, this, &ElectrodeConfigurations::on_electrodeNameChanged);
    scrollAreaLayout->addWidget(widgets.electrodeName, rowID, 1);

    widgets.hemisphereDefinition = new QComboBox();
    widgets.hemisphereDefinition->addItem("Left");
    widgets.hemisphereDefinition->addItem("Right");
    widgets.hemisphereDefinition->setFont(standardFont);
    widgets.hemisphereDefinition->setEnabled(false);
    widgets.hemisphereDefinition->setMinimumHeight(30);
    widgets.hemisphereDefinition->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.hemisphereDefinition, rowID, 2);

    widgets.targetDefinition = new QComboBox();
    widgets.targetDefinition->addItems(availableTargetNames);
    widgets.targetDefinition->setFont(standardFont);
    widgets.targetDefinition->setEnabled(false);
    widgets.targetDefinition->setMinimumHeight(30);
    widgets.targetDefinition->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.targetDefinition, rowID, 3);

    widgets.channelsSelection = new QPushButton();
    widgets.channelsSelection->setFont(standardFont);
    widgets.channelsSelection->setText("...");
    widgets.channelsSelection->setEnabled(false);
    widgets.channelsSelection->setMinimumHeight(30);
    widgets.channelsSelection->setProperty("rowID", rowID);
    widgets.channelsSelection->connect(widgets.channelsSelection, &QPushButton::clicked, this, &ElectrodeConfigurations::on_channelConfiguration);
    scrollAreaLayout->addWidget(widgets.channelsSelection, rowID, 4);

    electrodeWidgetsCollection.append(widgets);
    electrodeInfoCollection.append(information);
}

void ElectrodeConfigurations::on_AddElectrodes_clicked()
{
    addNewElectrodeRow(electrodeWidgetsCollection.length());
}

void ElectrodeConfigurations::on_electrodeNameChanged()
{
    QComboBox *electrodeNameWidget = qobject_cast<QComboBox*>(sender());
    int channelID = electrodeNameWidget->property("rowID").toInt() - 1;
    electrodeInfoCollection[channelID].channelIDs.clear();
    electrodeInfoCollection[channelID].numContacts = 0;

    QJsonArray electrodeDefinitions = interfaceConfiguration["ElectrodeDefinitions"].toArray();
    for (int i = 0; i < electrodeDefinitions.size(); i++)
    {
        if (electrodeDefinitions[i].toObject()["ElectrodeName"].toString() == electrodeWidgetsCollection[channelID].electrodeName->currentText())
        {
            electrodeInfoCollection[channelID].numContacts = electrodeDefinitions[i].toObject()["ChannelCount"].toInt();
            if (electrodeWidgetsCollection[channelID].electrodeName->currentText().contains("ECoG") || electrodeWidgetsCollection[channelID].electrodeName->currentText().contains("EMG"))
            {
                for (int j = 0; j < 2; j++) electrodeInfoCollection[channelID].layoutSize[j] = electrodeDefinitions[i].toObject()["Arrange"].toArray()[j].toInt();
            }
        }
    }

    if (electrodeNameWidget->currentText() != "None")
    {
        electrodeWidgetsCollection[channelID].hemisphereDefinition->setEnabled(true);
        electrodeWidgetsCollection[channelID].targetDefinition->setEnabled(true);
        electrodeWidgetsCollection[channelID].channelsSelection->setEnabled(true);
    }
    else
    {
        electrodeWidgetsCollection[channelID].hemisphereDefinition->setEnabled(false);
        electrodeWidgetsCollection[channelID].targetDefinition->setEnabled(false);
        electrodeWidgetsCollection[channelID].channelsSelection->setEnabled(false);
    }
}

void ElectrodeConfigurations::on_channelConfiguration()
{
    QPushButton *channelSelector = qobject_cast<QPushButton*>(sender());
    int channelID = channelSelector->property("rowID").toInt() - 1;

    int *channelIDs = (int*)malloc(sizeof(int)*electrodeInfoCollection[channelID].numContacts);
    for (int i = 0; i < electrodeInfoCollection[channelID].numContacts; i++)
    {
        if (i < electrodeInfoCollection[channelID].channelIDs.size())
        {
            channelIDs[i] = electrodeInfoCollection[channelID].channelIDs[i];
        }
        else
        {
            channelIDs[i] = 0;
        }
    }

    ChannelSelectionDialog channelSelectionDialog;
    channelSelectionDialog.setFixedSize(channelSelectionDialog.size());
    channelSelectionDialog.configureContactNumbers(electrodeWidgetsCollection[channelID].electrodeName->currentText(), channelID, electrodeInfoCollection[channelID].numContacts);
    channelSelectionDialog.configurePredefinedChannels(channelIDs);
    connect(&channelSelectionDialog, &ChannelSelectionDialog::channelIDsUpdate, this, &ElectrodeConfigurations::updateChannelConfiguration);
    channelSelectionDialog.exec();

    free(channelIDs);
}

void ElectrodeConfigurations::updateChannelConfiguration(int *channelIDs, int len, int electrodeID)
{
    electrodeInfoCollection[electrodeID].channelIDs.clear();
    for (int i = 0; i < len; i++)
    {
        electrodeInfoCollection[electrodeID].channelIDs.append(channelIDs[i]);
    }
}

void ElectrodeConfigurations::on_ChannelConfirm_clicked()
{
    QList<int> allContacts;
    bool verification = true;

    for (int i = 0; i < electrodeInfoCollection.length(); i++)
    {
        electrodeInfoCollection[i].electrodeType = electrodeWidgetsCollection[i].electrodeName->currentText();
        if (electrodeWidgetsCollection[i].hemisphereDefinition->isEnabled())
        {
            electrodeInfoCollection[i].hemisphere = electrodeWidgetsCollection[i].hemisphereDefinition->currentText();
            electrodeInfoCollection[i].target = electrodeWidgetsCollection[i].targetDefinition->currentText();
        }
        else
        {
            electrodeInfoCollection[i].hemisphere = "";
            electrodeInfoCollection[i].target = "";
        }

        // Main veritification in this step is to check if the NeuroOmega recording channel is configured for all electrode contacts
        electrodeInfoCollection[i].verified = true;
        if (electrodeWidgetsCollection[i].hemisphereDefinition->isEnabled())
        {
            for (int j = 0; j < electrodeInfoCollection[i].numContacts; j++)
            {
                if (j >= electrodeInfoCollection[i].channelIDs.size())
                {
                    // Missing channel configuration error message is here.
                    displayError(QMessageBox::Warning, "Electrode #" + QString::number(i+1) + " Channels not configured.");
                    electrodeInfoCollection[i].verified = false;
                    verification = false;
                    break;
                }
                else
                {
                    // Duplication of channel error message is here.
                    int index = allContacts.indexOf(electrodeInfoCollection[i].channelIDs[j]);
                    if (index != -1 && electrodeInfoCollection[i].channelIDs[j] > 0)
                    {
                        displayError(QMessageBox::Warning, "Electrode #" + QString::number(i+1) + " Channels found previously defined channel (" + QString::number(electrodeInfoCollection[i].channelIDs[j] - 10271) + ").");
                        electrodeInfoCollection[i].verified = false;
                        verification = false;
                        break;
                    }
                    allContacts << electrodeInfoCollection[i].channelIDs[j];
                }
            }
        }
    }

    if (verification)
    {
        int i = 0;
        while (i < electrodeInfoCollection.size())
        {
            if (electrodeInfoCollection[i].channelIDs.size() == 0)
            {
                electrodeInfoCollection.removeAt(i);
                continue;
            }
            i++;
        }
        this->accept();
    }
}

