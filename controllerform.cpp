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

#include "controllerform.h"
#include "ui_controllerform.h"

ControllerForm::ControllerForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControllerForm)
{
    ui->setupUi(this);

    QPushButton *contactButtons[] = {ui->StimulationContact_E00,
                                     ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                     ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                     ui->StimulationContact_E03};
    for (int i = 0; i < 8; i++)
    {
        contactButtons[i]->setHidden(true);
    }
    ui->StimulationContact_GlobalCAN->setHidden(true);
    ui->StimulationContact_Ring01->setHidden(true);
    ui->StimulationContact_Ring02->setHidden(true);

    electrodeLayout = new QGridLayout();
    ui->ElectrodeLayoutWidget->setLayout(electrodeLayout);

    naturalButtonStyle = "QPushButton {";
    naturalButtonStyle += "border-style: outset;";
    naturalButtonStyle += "border-width: 2px;";
    naturalButtonStyle += "border-radius: [RADIUS]px;";
    naturalButtonStyle += "border-color: black;";
    naturalButtonStyle += "background-color: [COLOR];";
    naturalButtonStyle += "}";

    ui->SequenceDisplayTable->setVisible(false);
    ui->SequenceDisplayTable->setColumnWidth(0, 60);
    ui->SequenceDisplayTable->setColumnWidth(1, 90);
    ui->SequenceDisplayTable->setColumnWidth(2, 90);
    ui->SequenceDisplayTable->setColumnWidth(3, 120);

    stimulationStateTimer = new QTimer(this);

    this->applicationConfiguration = new QSettings(QDir::currentPath() + "/defaultSettings.ini", QSettings::IniFormat);

    if (!this->applicationConfiguration->value("LastNovelStimulation").isNull())
    {
        QFile file(this->applicationConfiguration->value("LastNovelStimulation").toString());
        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        char *data = file.readAll().data();
        int16 *stimulationVector = (int16*) data;
        QString wavename = QFileInfo(this->applicationConfiguration->value("LastNovelStimulation").toString()).fileName().split(".").first();

        if (file.size() % 88000 == 0)
        {
            int result = LoadWaveToEmbedded(stimulationVector, file.size() / 2, 1, (cChar*) wavename.toStdString().c_str());
            if (result == eAO_OK) this->waveformList.append(wavename);
        }
    }

    if (!this->applicationConfiguration->value("LastStimulationConfiguration").isNull())
    {
        QFile file(this->applicationConfiguration->value("LastStimulationConfiguration").toString());
        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        QByteArray sequenceData = file.readAll();
        QJsonDocument loadedDocument = QJsonDocument::fromJson(sequenceData);
        if (loadedDocument.isObject())
        {
            QJsonObject stimulationConfiguration = loadedDocument.object();
            if (stimulationConfiguration.contains("StimulationName"))
            {
                this->stimulationConfigurations = loadedDocument;
                loadAnalogWaveform(stimulationConfiguration["AnalogWaveforms"].toArray());
                ui->SequenceFilename->setText(this->stimulationConfigurations.object()["StimulationName"].toString());
            }
        }
    }
}

ControllerForm::~ControllerForm()
{
    delete ui;
}

// Initialize Patient Information for Loggings
void ControllerForm::controllerInitialization(string patientID, string diagnosis)
{
    // Patient ID initialization
    if (patientID == "") this->patientID = "Unnamed";
    else this->patientID = patientID;

    ui->patientID_label->setText(this->patientID.c_str());
    this->diagnosis = diagnosis;

    // JSON Object Loggings
    QJsonObject statusObject;
    statusObject["Name"] = QJsonValue(QString::fromStdString(this->patientID));
    statusObject["Diagnosis"] = QJsonValue(QString::fromStdString(this->diagnosis));
    QDateTime currentTime;
    statusObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));

    // Create JSON Storage File. This is stored in the SurgicalLogFolder defined in "defaultConfiguration.ini:
    jsonStorage = new JSONStorage(applicationConfiguration->value("SurgicalLogFolder").toString() + "\\", currentTime.currentDateTime().toString("[yyyyMMdd_HH-mm-ss]") + " " + statusObject["Name"].toString() + ".json");
    jsonStorage->addJSON(statusObject);
    sideEffectNotes = new QFile(applicationConfiguration->value("SurgicalLogFolder").toString() + "\\" + currentTime.currentDateTime().toString("[yyyyMMdd_HH-mm-ss]") + " " + statusObject["Name"].toString() + ".txt");
    sideEffectNotes->open(QIODevice::WriteOnly | QIODevice::Text);

    // Reset the filename to MER. This is because NeuroOmega actually keep track of the filename previously set.
    // If we get a new patient we should at least reset it once before actual program starting
    QString filename = "";
    filename = filename + currentTime.currentDateTime().toString("yyyyMMdd") + "_" + QString::fromStdString(this->diagnosis) + "_" + QString::fromStdString(this->patientID) + "_MER_";

    int result = SetSaveFileName((char*)filename.toStdString().c_str(), filename.length());
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // Status Timer for Periodic Connection Checks
    connectionCheck = new QTimer(this);
    connect(connectionCheck, &QTimer::timeout, this, &ControllerForm::checkStatus);
    connectionCheck->start(1000);

}

// Clean-up after Form is closed
void ControllerForm::closeEvent(QCloseEvent *event)
{
    // Clean-up Step 1: Rename all channel name to default.
    // Default ECOG HF channel name are ECOG HF 01 / 01 - Array / 01
    uint32 channelCount = 0;
    GetChannelsCount(&channelCount);
    SInformation channelsInfo[channelCount];
    GetAllChannels(channelsInfo, channelCount);
    for (unsigned i = 0; i < channelCount; i++)
    {
        if (channelsInfo[i].channelID >= 10272 && channelsInfo[i].channelID <= 10335)
        {
            int boxID = (channelsInfo[i].channelID - 10272) / 16;
            int channelID = (channelsInfo[i].channelID - 10272) % 16;
            QString defaultChannelName = "ECOG HF " + QString::number(boxID+1) + " / " + QStringLiteral("%1").arg(channelID+1, 2, 10, QLatin1Char('0')) + " - Array " + QString::number(boxID+1) + " / " + QStringLiteral("%1").arg(channelID+1, 2, 10, QLatin1Char('0'));
            SetChannelName(channelsInfo[i].channelID, (char*)defaultChannelName.toStdString().c_str(), defaultChannelName.length());
        }
    }

    // Clean-up Step 2: If recording is on-going, Stop recording
    if (recordingStatus) on_NeuroOmega_RecordingStop_clicked();

    // Clean-up Step 3: If stimulation is on-going, stop stimualtion.
    if (currentStimulationState) on_StimulationControl_Stop_clicked();

    // Clean-up Step 4: Save the JSON and Note File
    jsonStorage->saveJSON();
    sideEffectNotes->close();

    // Closing NeuroOmega Connection
    CloseConnection();
    emit connectionChanged();
}

// Generic Error Log Reporting Function to extract NeuroOmega error message for display
QString ControllerForm::getErrorLog()
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

// Standard error display using QMessageBox
void ControllerForm::displayError(int errorLevel, QString message)
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

// Periodic task that check NeuroOmega connectivity
void ControllerForm::checkStatus()
{
    // If NeuroOmega is closed, request closing of the current controller form.
    int result = isConnected();
    if (result != eAO_CONNECTED)
    {
        displayError(QMessageBox::Critical, "The connection with NeuroOmega is disconnected.");
        connectionCheck->stop();
        this->close();
        return;
    }

    // Status Message templates can be edited here.
    QString statusMessage;
    int connectionQuality = 0;
    real32 percentThroughput = 0;

    result = CheckQualityConnection(&connectionQuality, &percentThroughput);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
    }
    else
    {
        switch (connectionQuality)
        {
        case 1:
            statusMessage += "POOR, ";
            break;
        case 2:
            statusMessage += "MED , ";
            break;
        case 3:
            statusMessage += "GOOD, ";
            break;
        }
    }

    // Note is that NeuroOmega timestamp is reported as number of clock, divided by 44k to get actual seconds since NeuroOmega started.
    ulong timestamp = 0;
    result = GetLatestTimeStamp(&timestamp);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
    }
    else
    {
        statusMessage = statusMessage + QString::number(timestamp / 44000) + " sec.";
    }
    ui->NeuroOmega_StatusString->setText(statusMessage);

    if (this->recordingStatus) ui->RecordingDurationLabel->setText(QString::number(recordingElapsedTime.elapsed() / 1000) + " sec");

    QJsonObject jsonObject;
    jsonObject["ObjectType"] = QJsonValue("MotorStatus");

    QDateTime currentTime;
    jsonObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));

    int32 motorDepth = 0;
    result = GetDriveDepth(&motorDepth);
    if (result == eAO_OK)
    {
        jsonObject["CurrentDepth"] = QJsonValue((qint64)motorDepth);
        uint32 motorStopTimer = 0, motorStartTimer = 0;
        result = GetMoveMotorTS(&motorStartTimer);
        if (result == eAO_OK)
        {
            jsonObject["LastMoveTS"] = QJsonValue((qint64)motorStartTimer);
        }
        result = GetStopMotorTS(&motorStopTimer);
        if (result == eAO_OK)
        {
            jsonObject["LastStopTS"] = QJsonValue((qint64)motorStopTimer);
        }
        jsonStorage->addJSON(jsonObject);
    }
}

////////////////////////////////////
/// Electrode Dropbox Callbacks ////
////////////////////////////////////
// The main electrode loading process.
void ControllerForm::configureElectrodes(QList<ElectrodeInformation> electrodeInformations)
{
    electrodeConfigurations = electrodeInformations;

    // Configure the selected channel for recording. See detail function "configureRecordingChannels()" on how this process work.
    if (!configureRecordingChannels()) return;

    // UI-update to display channels for selection.
    ui->StimulationContact_GlobalCAN->setHidden(false);

    // JSON logging to record what electrode configurations are stored for this patient.
    QJsonObject jsonObject;
    jsonObject["ObjectType"] = QJsonValue("ElectrodeConfigurations");

    // First adding Microelectrode/Macroelectrode Selection
    ui->StimulationControl_Electrode->addItem("Microelectrode / Macroelectrode");

    // Include the 4 Predefined Channel
    for (int i = 0; i < electrodeConfigurations.size(); i++)
    {
        if (electrodeConfigurations[i].electrodeType != "None")
        {
            ui->StimulationControl_Electrode->addItem("Lead #" + QString::number(i+1) + " " + electrodeConfigurations[i].hemisphere + " " + electrodeConfigurations[i].target);
            jsonObject["Lead" + QString::number(i+1)] = QJsonValue(electrodeConfigurations[i].electrodeType + " " + electrodeConfigurations[i].hemisphere + " " + electrodeConfigurations[i].target);
        }
    }

    QDateTime currentTime;
    jsonObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    jsonStorage->addJSON(jsonObject);

    // UI udpates
    ui->StimulationControl_Electrode->setEnabled(true);
    ui->StimulationControl_Electrode->setCurrentIndex(0);
    setupElectrodeButtons(ui->StimulationControl_Electrode->currentText());
    ui->StimulationControl_Amplitude->setEnabled(true);
    ui->StimulationControl_Pulsewidth->setEnabled(true);
    ui->StimulationControl_Frequency->setEnabled(true);
    ui->StimulationControl_Duration->setEnabled(true);
    ui->StimulationControl_Start->setEnabled(true);
    ui->StimulationControl_PassiveRecharge->setEnabled(true);
    ui->NeuroOmega_RecordingStart->setEnabled(true);

    QPushButton *benefitBtns[] = {ui->TremorScale_1, ui->TremorScale_2, ui->TremorScale_3, ui->TremorScale_4, ui->TremorScale_5,
                                  ui->RigidScale_1, ui->RigidScale_2, ui->RigidScale_3, ui->RigidScale_4, ui->RigidScale_5,
                                  ui->BradyScale_1, ui->BradyScale_2, ui->BradyScale_3, ui->BradyScale_4, ui->BradyScale_5};
    for (int i = 0; i < 15; i++)
    {
        benefitBtns[i]->setEnabled(true);
    }

    QPushButton *sideEffectsBtns[] = {ui->SideEffectsLabels_1, ui->SideEffectsLabels_2, ui->SideEffectsLabels_3, ui->SideEffectsLabels_4,
                                      ui->SideEffectsLabels_5, ui->SideEffectsLabels_6, ui->SideEffectsLabels_7, ui->SideEffectsLabels_8,
                                      ui->PersistentSideEffectsLabel, ui->TransientSideEffectsLabel};
    for (int i = 0; i < 10; i++)
    {
        sideEffectsBtns[i]->setEnabled(true);
    }
}

////////////////////////////////////
////// Stimulation Callbacks ///////
////////////////////////////////////
// Initial Electrode Setup. This is a UI-updating function after electrode configuration is completed.
void ControllerForm::setupElectrodeButtons(QString electrodeName)
{
    ui->ElectrodeLayoutWidget->setStyleSheet("");

    noneStyle = naturalButtonStyle;
    cathodeStyle = naturalButtonStyle;
    anodeStyle = naturalButtonStyle;
    noneStyle.replace("[COLOR]","transparent");
    cathodeStyle.replace("[COLOR]","rgb(83, 175, 255)");
    anodeStyle.replace("[COLOR]","rgb(255, 88, 99)");

    for (int i = 0; i < displayChannelButtons.size(); i++) delete(displayChannelButtons[i]);
    displayChannelButtons.clear();

    // UI update, reset all contacts to hidden
    QPushButton *allButtons[] = {ui->StimulationContact_E00,
                                     ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                     ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                     ui->StimulationContact_E03};
    for (int i = 0; i < 8; i++)
    {
        allButtons[i]->setHidden(true);
    }

    // Default Display for Micro/Macro Electrode. We currently hard-coded the program to handle up to 2 Microelectrode Arrays.
    if (electrodeName == "Microelectrode / Macroelectrode")
    {
        this->currentElectrodeConfiguration = {0};
        int channelIndex = 0;
        ui->StimulationContact_Ring01->setHidden(true);
        ui->StimulationContact_Ring02->setHidden(true);

        // NeuroOmega Channel 10000 - MicroElectrode 01
        // If the channel is enabled. Display MicroElectrode 1 for Stimulation
        int saveState = 0;
        GetChannelSaveState(10000, &saveState);
        if (saveState != 0)
        {
            ui->StimulationContact_E01_1->setText("Micro\n01");
            ui->StimulationContact_E01_1->setProperty("ChannelID", 10000);
            ui->StimulationContact_E01_1->setStyleSheet(noneStyle);
            ui->StimulationContact_E01_1->setHidden(false);
            this->currentElectrodeConfiguration.channelIDs.append(10000);
            channelIndex++;
        }

        // NeuroOmega Channel 10005 - Macroelectrode 01
        // If the channel is enabled. Display Macroelectrode 1 for Stimulation
        GetChannelSaveState(10005, &saveState);
        if (saveState != 0)
        {
            ui->StimulationContact_E02_1->setText("Macro\n01");
            ui->StimulationContact_E02_1->setProperty("ChannelID", 10005);
            ui->StimulationContact_E02_1->setStyleSheet(noneStyle);
            ui->StimulationContact_E02_1->setHidden(false);
            this->currentElectrodeConfiguration.channelIDs.append(10005);
            channelIndex++;
        }

        // NeuroOmega Channel 10001 - MicroElectrode 02
        // If the channel is enabled. Display MicroElectrode 2 for Stimulation
        GetChannelSaveState(10001, &saveState);
        if (saveState != 0)
        {
            ui->StimulationContact_E01_3->setText("Micro\n02");
            ui->StimulationContact_E01_3->setProperty("ChannelID", 10001);
            ui->StimulationContact_E01_3->setStyleSheet(noneStyle);
            ui->StimulationContact_E01_3->setHidden(false);
            this->currentElectrodeConfiguration.channelIDs.append(10001);
            channelIndex++;
        }

        // NeuroOmega Channel 10006 - Macroelectrode 02
        // If the channel is enabled. Display Macroelectrode 2 for Stimulation
        GetChannelSaveState(10006, &saveState);
        if (saveState != 0)
        {
            ui->StimulationContact_E02_3->setText("Macro\n02");
            ui->StimulationContact_E02_3->setProperty("ChannelID", 10006);
            ui->StimulationContact_E02_3->setStyleSheet(noneStyle);
            ui->StimulationContact_E02_3->setHidden(false);
            this->currentElectrodeConfiguration.channelIDs.append(10006);
            channelIndex++;
        }
    }
    else
    {
        // Extract Electrode ID and Configuration from Electrode Text
        int electrodeID = 0;
        char side[20], target[20];
        if (electrodeName.contains("Lead"))
        {
            sscanf(electrodeName.toStdString().c_str(), "Lead #%d %s %s", &electrodeID, side, target);
            electrodeID--;
        }

        this->currentElectrodeConfiguration = this->electrodeConfigurations[electrodeID];

        // Handle Configuration for 4-contact electrodes
        if (this->currentElectrodeConfiguration.electrodeType.contains("ECoG") || this->currentElectrodeConfiguration.electrodeType.contains("EMG"))
        {
            ui->StimulationContact_Ring01->setHidden(true);
            ui->StimulationContact_Ring02->setHidden(true);
            ui->ElectrodeLayoutWidget->setStyleSheet("#ElectrodeLayoutWidget {background-image: url(" + QDir::currentPath() + "/resources/ECoG_Background.png);}");

            int estimatedHeight = 500/this->electrodeConfigurations[electrodeID].layoutSize[1]*0.8;

            noneStyle.replace("[RADIUS]",QString::number(estimatedHeight/2));
            cathodeStyle.replace("[RADIUS]",QString::number(estimatedHeight/2));
            anodeStyle.replace("[RADIUS]",QString::number(estimatedHeight/2));

            for (int i = 0; i < this->electrodeConfigurations[electrodeID].numContacts; i++)
            {
                QPushButton *button = new QPushButton();
                button->setFixedSize(estimatedHeight,estimatedHeight);
                button->setStyleSheet(noneStyle);
                button->setFlat(true);
                button->setText(QString::number(i));
                button->setProperty("ChannelID", this->electrodeConfigurations[electrodeID].channelIDs[i]);
                button->connect(button, &QPushButton::clicked, this, &ControllerForm::on_StimulationContactClicked);
                electrodeLayout->addWidget(button, i % this->electrodeConfigurations[electrodeID].layoutSize[1], i / this->electrodeConfigurations[electrodeID].layoutSize[1]);
                displayChannelButtons.append(button);
            }
            ui->StimulationContact_GlobalCAN->setStyleSheet(noneStyle);

        }
        else if (this->electrodeConfigurations[electrodeID].numContacts == 4)
        {
            ui->StimulationContact_Ring01->setHidden(true);
            ui->StimulationContact_Ring02->setHidden(true);
            QPushButton *contactButtons[] = {ui->StimulationContact_E00, ui->StimulationContact_E01_2, ui->StimulationContact_E02_2, ui->StimulationContact_E03};
            for (int i = 0; i < 4; i++)
            {
                contactButtons[i]->setText("Contact\n" + QString::number(i));
                contactButtons[i]->setProperty("ChannelID", this->electrodeConfigurations[electrodeID].channelIDs[i]);
                contactButtons[i]->setStyleSheet(noneStyle);
                contactButtons[i]->setHidden(false);
            }
        }
        // Handle Configuration for 8-contact electrodes
        else
        {
            // Ring Selector for easier selection.
            ui->StimulationContact_Ring01->setHidden(false);
            ui->StimulationContact_Ring02->setHidden(false);
            QPushButton *contactButtons[] = {ui->StimulationContact_E00, ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3, ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3, ui->StimulationContact_E03};
            for (int i = 0; i < 8; i++)
            {
                int ringID = ((i - 1) / 3) + 1;
                contactButtons[i]->setText("Contact\n" + QString::number(ringID) + "." + QString::number(i - (ringID - 1) * 3));
                contactButtons[i]->setProperty("ChannelID", this->electrodeConfigurations[electrodeID].channelIDs[i]);
                contactButtons[i]->setStyleSheet(noneStyle);
                contactButtons[i]->setHidden(false);
            }
            contactButtons[0]->setText("Contact\n" + QString::number(0));
            contactButtons[7]->setText("Contact\n" + QString::number(3));
        }
    }
}

// UI update for stimulation electrodes
void ControllerForm::on_StimulationControl_Electrode_currentTextChanged(const QString &electrodeName)
{
    setupElectrodeButtons(electrodeName);
    StimulationAnode.clear();
    StimulationCathode = 0;
    ui->StimulationContact_GlobalCAN->setStyleSheet(noneStyle);
}

// Quick reset for all electrode cathodes. This will be execute every time electrode selector changes.
void ControllerForm::resetCathodeSelection()
{
    if (displayChannelButtons.size() > 0)
    {
        for (int i = 0; i < displayChannelButtons.size(); i++)
        {
            if (displayChannelButtons[i]->styleSheet() == cathodeStyle)
            {
                displayChannelButtons[i]->setStyleSheet(noneStyle);
            }
        }
    }
    else
    {
        QPushButton *contactButtons[] = {ui->StimulationContact_E00, ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3, ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3, ui->StimulationContact_E03};
        for (int i = 0; i < 8; i++)
        {
            if (contactButtons[i]->styleSheet() == cathodeStyle)
            {
                contactButtons[i]->setStyleSheet(noneStyle);
            }
        }
    }
}

// UI updates
void ControllerForm::on_StimulationContact_GlobalCAN_clicked()
{
    if (StimulationCathode == -1)
    {
        StimulationCathode = 0;
        ui->StimulationContact_GlobalCAN->setStyleSheet(noneStyle);
    }
    else
    {
        StimulationCathode = -1;
        resetCathodeSelection();
        ui->StimulationContact_GlobalCAN->setStyleSheet(cathodeStyle);
    }
}

// UI updates for ring selection
void ControllerForm::on_StimulationContact_Ring01_clicked()
{
    if (ui->StimulationContact_E01_1->styleSheet() == anodeStyle && ui->StimulationContact_E01_2->styleSheet() == anodeStyle && ui->StimulationContact_E01_3->styleSheet() == anodeStyle)
    {
        int electrodeID = ui->StimulationContact_E01_1->property("ChannelID").toInt();
        ui->StimulationContact_E01_1->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));

        electrodeID = ui->StimulationContact_E01_2->property("ChannelID").toInt();
        ui->StimulationContact_E01_2->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));

        electrodeID = ui->StimulationContact_E01_3->property("ChannelID").toInt();
        ui->StimulationContact_E01_3->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));
        return;
    }

    QPushButton *allButtons[] = {ui->StimulationContact_E00,
                                 ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                 ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                 ui->StimulationContact_E03,
                                 ui->StimulationContact_GlobalCAN};

    for (int i = 0; i < 9; i++)
    {
        if (allButtons[i]->styleSheet() == anodeStyle)
        {
            allButtons[i]->setStyleSheet(noneStyle);
        }
    }
    ui->StimulationContact_E01_1->setStyleSheet(anodeStyle);
    ui->StimulationContact_E01_2->setStyleSheet(anodeStyle);
    ui->StimulationContact_E01_3->setStyleSheet(anodeStyle);

    StimulationAnode.clear();
    QPushButton *ring1Buttons[] = {ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3};
    for (int i = 0; i < 3; i++)
    {
        int electrodeID = ring1Buttons[i]->property("ChannelID").toInt();
        StimulationAnode << electrodeID;
    }
}

// UI updates for ring selection
void ControllerForm::on_StimulationContact_Ring02_clicked()
{
    if (ui->StimulationContact_E02_1->styleSheet() == anodeStyle && ui->StimulationContact_E02_2->styleSheet() == anodeStyle && ui->StimulationContact_E02_3->styleSheet() == anodeStyle)
    {
        int electrodeID = ui->StimulationContact_E02_1->property("ChannelID").toInt();
        ui->StimulationContact_E02_1->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));

        electrodeID = ui->StimulationContact_E02_2->property("ChannelID").toInt();
        ui->StimulationContact_E02_2->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));

        electrodeID = ui->StimulationContact_E02_3->property("ChannelID").toInt();
        ui->StimulationContact_E02_3->setStyleSheet(noneStyle);
        StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));
        return;
    }

    QPushButton *allButtons[] = {ui->StimulationContact_E00,
                                 ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                 ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                 ui->StimulationContact_E03,
                                 ui->StimulationContact_GlobalCAN};

    for (int i = 0; i < 9; i++)
    {
        if (allButtons[i]->styleSheet() == anodeStyle)
        {
            allButtons[i]->setStyleSheet(noneStyle);
        }
    }

    ui->StimulationContact_E02_1->setStyleSheet(anodeStyle);
    ui->StimulationContact_E02_2->setStyleSheet(anodeStyle);
    ui->StimulationContact_E02_3->setStyleSheet(anodeStyle);

    StimulationAnode.clear();
    QPushButton *ring1Buttons[] = {ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3};
    for (int i = 0; i < 3; i++)
    {
        int electrodeID = ring1Buttons[i]->property("ChannelID").toInt();
        StimulationAnode << electrodeID;
    }
}

// UI updates for contact selection
void ControllerForm::on_StimulationContactClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int electrodeID = button->property("ChannelID").toInt();

    if (button->styleSheet() == noneStyle)
    {
        button->setStyleSheet(anodeStyle);
        StimulationAnode << electrodeID;
    }
    else if (button->styleSheet() == anodeStyle)
    {
        if (ui->StimulationContact_GlobalCAN->styleSheet() == cathodeStyle)
        {
            StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));
            button->setStyleSheet(noneStyle);
        }
        else
        {
            StimulationAnode.removeAt(StimulationAnode.indexOf(electrodeID));
            StimulationCathode = electrodeID;
            resetCathodeSelection();
            button->setStyleSheet(cathodeStyle);
        }
    }
    else
    {
        StimulationCathode = 0;
        button->setStyleSheet(noneStyle);
    }

    if (ui->StimulationControl_Stop->isEnabled())
    {
        on_StimulationControl_Stop_clicked();
    }
}

// Standard Pipeline for handling Stimulation On
void ControllerForm::on_StimulationControl_Start_clicked()
{
    // First establish the timer to upate Stimulation State.
    connect(stimulationStateTimer, &QTimer::timeout, this, &ControllerForm::stimulationStateUpdate);

    // Setup the progress bar to display current process through the stimulation.
    // NeuroOmega uses fix duration stimulation, so once the pre-defined duration reach, stimulation will stop.
    ui->StimulationProgressBar->setMaximum(ui->StimulationControl_Duration->value());
    ui->StimulationProgressBar->setValue(0);

    // Extract Stimulation Parameter in the UI
    real32 amplitude = 0, pulsewidth = 0, duration = 0, rechargePulse = 0, rechargeAmplitude = 0;
    int frequency = 0;
    amplitude = -ui->StimulationControl_Amplitude->value();
    pulsewidth = ui->StimulationControl_Pulsewidth->value() / 1000.0;
    duration = ui->StimulationControl_Duration->value();
    frequency = ui->StimulationControl_Frequency->value();
    rechargePulse = 900 / (float)frequency - pulsewidth;
    rechargeAmplitude = (amplitude * pulsewidth) / rechargePulse;

    // Display error if no stimualtion contacts were selected, or return is not defined
    if (StimulationAnode.size() == 0 || StimulationCathode == 0)
    {
        displayError(QMessageBox::Warning, "No stimulation contacts selected, or the return contact is not defined");
        return;
    }

    // Setup JSON storage to handle loggings of current stimulation parameters
    QJsonObject stimulationObject;
    QJsonArray anodeArray;
    QJsonArray anodeArrayName;
    QPushButton *allButtons[] = {ui->StimulationContact_E00,
                                 ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                 ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                 ui->StimulationContact_E03,
                                 ui->StimulationContact_GlobalCAN};
    for (int i = 0; i < 9; i++)
    {
        if (allButtons[i]->styleSheet() == anodeStyle) anodeArrayName.append(QJsonValue(ui->StimulationControl_Electrode->currentText() + " " + allButtons[i]->text()));
        if (allButtons[i]->styleSheet() == cathodeStyle) stimulationObject["StimulationReturnName"] = QJsonValue(ui->StimulationControl_Electrode->currentText() + " " + allButtons[i]->text());
    }

    if (this->currentWaveformID != -1)
    {
        QMessageBox::critical(this, "Error", "Novel Waveform Selected. Special Stimulation Mode Start", QMessageBox::Close);
        stimulationObject["ObjectType"] = QJsonValue("Novel Stimulation");
        stimulationObject["WaveName"] = QJsonValue(this->waveformList.at(this->currentWaveformID));
    }
    else
    {
        stimulationObject["ObjectType"] = QJsonValue("StimulationOn");
        stimulationObject["Frequency"] = QJsonValue(frequency);
        stimulationObject["Amplitude"] = QJsonValue(amplitude);
        stimulationObject["PulseWidth"] = QJsonValue(pulsewidth);
    }

    // NeuroOmega Stimulation Setup: Each Anode is configured separately for multi-contact stimulation.
    for (int i = 0; i < StimulationAnode.size(); i++)
    {
        if (this->currentWaveformID == -1)
        {
            int result;
            if (!ui->StimulationControl_PassiveRecharge->isChecked())
            {
                result = SetStimulationParameters(amplitude / StimulationAnode.size(), pulsewidth, -amplitude / StimulationAnode.size(), pulsewidth, frequency, duration, StimulationCathode, StimulationAnode.at(i), 0, 0);
            }
            else
            {
                result = SetStimulationParameters(amplitude / StimulationAnode.size(), pulsewidth, 0, pulsewidth, frequency, duration, StimulationCathode, StimulationAnode.at(i), 0, 0);
            }
            if (result != eAO_OK)
            {
                QString messsage = getErrorLog();
                displayError(QMessageBox::Warning, messsage);
                return;
            }
            anodeArray.append(QJsonValue(StimulationAnode.at(i)));

        }
    }

    // A separate loop is used to start stimulation because "SetStimulationParameters" has a significant delay.
    // If we request start right after configure each contact, stimulations from all contaccts will not be aligned.
    for (int i = 0; i <StimulationAnode.size(); i++)
    {
        if (this->currentWaveformID == -1)
        {
            int result = StartStimulation(StimulationAnode.at(i));
            if (result != eAO_OK)
            {
                QString messsage = getErrorLog();
                displayError(QMessageBox::Warning, messsage);
                return;
            }
        }
        else
        {
            int result = StartAnalogStimulation(StimulationAnode.at(i), this->currentWaveformID, -1, duration, StimulationCathode);
            if (result != eAO_OK)
            {
                QString messsage = getErrorLog();
                displayError(QMessageBox::Warning, messsage);
                return;
            }
        }
    }
    currentStimulationState = true;

    // Configure the timer to call status update every 100ms.
    stimulationElapsedTime.restart();
    stimulationStateTimer->start(100);

    // Save all configurations to JSON
    stimulationObject["StimulationChannel"] = anodeArray;
    stimulationObject["StimulationChannelName"] = anodeArrayName;
    stimulationObject["StimulationReturn"] = QJsonValue(StimulationCathode);

    QDateTime currentTime;
    stimulationObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    jsonStorage->addJSON(stimulationObject);

    // UI update so user cannot start stimulation after stimulation already started.
    ui->StimulationControl_Start->setEnabled(false);
    ui->StimulationControl_Stop->setEnabled(true);
}

// Update Stimulation State and UIs
void ControllerForm::stimulationStateUpdate()
{
    // See documentation on how SequenceStimulation is handled.
    if (novelStimulationStatus) startSequentialStimulation();

    // UI-updates
    ui->StimulationProgressBar->setValue(stimulationElapsedTime.elapsed() / 1000);
    ui->StimulationProgressBar->repaint();

    if (ui->StimulationProgressBar->value() >= ui->StimulationProgressBar->maximum())
    {
        on_StimulationControl_Stop_clicked();
        ui->StimulationControl_Start->setEnabled(true);
        ui->StimulationControl_Stop->setEnabled(false);
    }
}

// Advance Stimulation Parameter based on a configuration JSON file.
// This function operate on a simple sequential logic for starting and stopping stimulation. The full process is lengthy to describe, please refer to the documentation.
void ControllerForm::startSequentialStimulation()
{
    QJsonArray stimulationSequences = stimulationConfigurations.object()["StimulationSequence"].toArray();

    double phaseTimer = 0;
    for (int i = 0; i < stimulationSequences.size(); i++)
    {
        if (this->currentStimulationStage == i)
        {
            if (!stimulationSequences[i].isObject())
            {
                displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Sequence is not JsonObject");
                on_StimulationControl_Stop_clicked();
                return;
            }

            if (!stimulationSequences[i].toObject().contains("StimulationType") ||
                    !stimulationSequences[i].toObject().contains("StimulationLead") ||
                    !stimulationSequences[i].toObject().contains("StimulationChannel") ||
                    !stimulationSequences[i].toObject().contains("StimulationReturn") ||
                    !stimulationSequences[i].toObject().contains("Duration"))
            {
                displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Missing Important Configurations");
                on_StimulationControl_Stop_clicked();
                return;
            }

            if (stimulationSequences[i].toObject()["StimulationLead"].toInt() < 0 || stimulationSequences[i].toObject()["StimulationLead"].toInt() > 3)
            {
                displayError(QMessageBox::Warning, QString("Lead # %1 Not Exist").arg(stimulationSequences[i].toObject()["StimulationLead"].toInt()+1));
                on_StimulationControl_Stop_clicked();
                return;
            }

            if (this->electrodeConfigurations[stimulationSequences[i].toObject()["StimulationLead"].toInt()].electrodeType == "None")
            {
                displayError(QMessageBox::Warning, QString("Lead # %1 Not Connected").arg(stimulationSequences[i].toObject()["StimulationLead"].toInt()+1));
                on_StimulationControl_Stop_clicked();
                return;
            }

            if (!this->currentStimulationState)
            {
                if (stimulationElapsedTime.elapsed() / 1000.0 >= phaseTimer)
                {
                    // Stimulation Contacts
                    QJsonArray stimulationContactArray = stimulationSequences[i].toObject()["StimulationChannel"].toArray();
                    QVector<int> electrodeContacts = this->electrodeConfigurations[stimulationSequences[i].toObject()["StimulationLead"].toInt()].channelIDs;

                    if (stimulationSequences[i].toObject()["StimulationReturn"].toInt() > this->electrodeConfigurations[stimulationSequences[i].toObject()["StimulationLead"].toInt()].numContacts ||
                        stimulationSequences[i].toObject()["StimulationReturn"].toInt() < -1)
                    {
                        displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Bad contacts");
                        on_StimulationControl_Stop_clicked();
                        return;
                    }

                    int returnContact = -1;
                    if (stimulationSequences[i].toObject()["StimulationReturn"].toInt() != -1)
                    {
                        returnContact = electrodeContacts[stimulationSequences[i].toObject()["StimulationReturn"].toInt()];
                    }

                    if (stimulationSequences[i].toObject()["RecordingFilename"].toString() != currentProgrammedFilename)
                    {
                        int result = StopSave();
                        if (result != eAO_OK)
                        {
                            QString messsage = getErrorLog();
                            displayError(QMessageBox::Warning, messsage);
                            return;
                        }

                        updateAnnotation("Research_" + stimulationSequences[i].toObject()["RecordingFilename"].toString(), QJsonDocument());
                        currentProgrammedFilename = stimulationSequences[i].toObject()["RecordingFilename"].toString();

                        result = StartSave();
                        if (result != eAO_OK)
                        {
                            QString messsage = getErrorLog();
                            displayError(QMessageBox::Warning, messsage);
                            return;
                        }
                    }

                    if (stimulationSequences[i].toObject()["StimulationType"].toString().contains("Novel"))
                    {

                        int16_t* stimulationVector = this->preloadedAnalogWaveforms[stimulationSequences[i].toObject()["StimulationIndex"].toInt()];
                        AnalogWaveformDescriptor overview = analogWaveformDescriptor[stimulationSequences[i].toObject()["StimulationIndex"].toInt()];

                        int result = LoadWaveToEmbedded(stimulationVector, overview.filesize, 1, (cChar*)overview.wavename.toStdString().c_str());
                        if (result != eAO_OK)
                        {
                            QString messsage = getErrorLog();
                            displayError(QMessageBox::Warning, messsage);
                            on_StimulationControl_Stop_clicked();
                            return;
                        }
                        else
                        {
                            this->waveformList.clear();
                            this->waveformList.append(overview.wavename);
                            this->currentWaveformID = 0;
                        }

                        QElapsedTimer executionTimer = QElapsedTimer();
                        for (int j = 0; j < stimulationContactArray.size(); j++)
                        {
                            if (stimulationContactArray[j].toInt() >= this->electrodeConfigurations[stimulationSequences[i].toObject()["StimulationLead"].toInt()].numContacts ||
                                stimulationContactArray[j].toInt() < 0)
                            {
                                displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Bad contacts");
                                on_StimulationControl_Stop_clicked();
                                return;
                            }

                            int result = StartAnalogStimulation(electrodeContacts[stimulationContactArray[j].toInt()], this->currentWaveformID, -1, stimulationSequences[i].toObject()["Duration"].toInt(), returnContact);
                            executionTimer.restart();
                            while (executionTimer.elapsed() < 5000 && result != eAO_OK)
                            {
                                result = StartAnalogStimulation(electrodeContacts[stimulationContactArray[j].toInt()], this->currentWaveformID, -1, stimulationSequences[i].toObject()["Duration"].toInt(), returnContact);
                            }
                            if (result != eAO_OK)
                            {
                                QString messsage = getErrorLog();
                                displayError(QMessageBox::Warning, messsage);
                                on_StimulationControl_Stop_clicked();
                                return;
                            }
                        }
                    }
                    else if (stimulationSequences[i].toObject()["StimulationType"].toString().contains("Standard"))
                    {
                        if (!stimulationSequences[i].toObject().contains("Amplitude") ||
                                !stimulationSequences[i].toObject().contains("Pulsewidth") ||
                                !stimulationSequences[i].toObject().contains("Frequency"))
                        {
                            displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Missing Important Standard Stim Configurations");
                            on_StimulationControl_Stop_clicked();
                            return;
                        }

                        for (int j = 0; j < stimulationContactArray.size(); j++)
                        {
                            if (stimulationContactArray[j].toInt() >= this->electrodeConfigurations[stimulationSequences[i].toObject()["StimulationLead"].toInt()].numContacts ||
                                stimulationContactArray[j].toInt() < 0)
                            {
                                displayError(QMessageBox::Warning, "Bad Stimulation Configuration, Bad contacts");
                                on_StimulationControl_Stop_clicked();
                                return;
                            }

                            int result = SetStimulationParameters(-stimulationSequences[i].toObject()["Amplitude"].toDouble() / stimulationContactArray.size(),
                                                                  stimulationSequences[i].toObject()["Pulsewidth"].toDouble() / 1000.0,
                                                                  stimulationSequences[i].toObject()["Amplitude"].toDouble() / stimulationContactArray.size(),
                                                                  stimulationSequences[i].toObject()["Pulsewidth"].toDouble() / 1000.0,
                                                                  stimulationSequences[i].toObject()["Frequency"].toInt(),
                                                                  stimulationSequences[i].toObject()["Duration"].toInt(),
                                                                  returnContact,
                                                                  electrodeContacts[stimulationContactArray[j].toInt()], 0, 0);
                            if (result != eAO_OK)
                            {
                                QString messsage = getErrorLog();
                                displayError(QMessageBox::Warning, messsage);
                                on_StimulationControl_Stop_clicked();
                                return;
                            }
                        }


                        for (int j = 0; j < stimulationContactArray.size(); j++)
                        {
                            int result = StartStimulation(electrodeContacts[stimulationContactArray[j].toInt()]);
                            if (result != eAO_OK)
                            {
                                QString messsage = getErrorLog();
                                displayError(QMessageBox::Warning, messsage);
                                on_StimulationControl_Stop_clicked();
                                return;
                            }
                        }

                    }
                    this->currentStimulationState = true;
                }
            }
            else
            {
                if (stimulationElapsedTime.elapsed() / 1000.0 >= phaseTimer + stimulationSequences[i].toObject()["Duration"].toDouble())
                {
                    int result = StopStimulation(-1);
                    if (result != eAO_OK)
                    {
                        QString messsage = getErrorLog();
                        displayError(QMessageBox::Warning, messsage);
                        on_StimulationControl_Stop_clicked();
                        return;
                    }

                    ui->SequenceDisplayTable->setRowHidden(this->currentStimulationStage, true);
                    this->currentStimulationState = false;
                    this->currentStimulationStage = i+1;
                }
            }
        }
        phaseTimer += stimulationSequences[i].toObject()["Duration"].toDouble();
    }
    ui->StimulationProgressBar->setMaximum(phaseTimer);

    if (this->currentStimulationStage == stimulationSequences.size() && !this->currentStimulationState)
    {
        on_StimulationControl_Stop_clicked();
        ui->SequenceDisplayTable->setVisible(false);
    }
}

// Stopping Stimulation
void ControllerForm::on_StimulationControl_Stop_clicked()
{
    // Request Stimulation Stop. One function will handle all multi-contact stimulations
    int result = StopStimulation(-1);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // If we are doing pre-programmed stimulation from Annotation, this will also stop the recording unless we set it to be infiniteRecording.
    if (novelStimulationStatus && !infiniteRecording)
    {
        novelStimulationStatus = false;
        on_NeuroOmega_RecordingStop_clicked();
    }
    currentStimulationState = false;

    // JSON storage
    QJsonObject stimulationObject;
    stimulationObject["ObjectType"] = QJsonValue("StimulationOff");

    QDateTime currentTime;
    stimulationObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    jsonStorage->addJSON(stimulationObject);

    // Finally, stop the callbacks
    if (stimulationStateTimer->isActive())
    {
        stimulationStateTimer->stop();
    }
    ui->StimulationControl_Start->setEnabled(true);
    ui->StimulationControl_Stop->setEnabled(false);

}

// A special case of stimulation start. This is to start stimulation in sequential mode described in documentation.
void ControllerForm::on_StimulationControl_Novel_Start_clicked()
{
    on_StimulationControl_Stop_clicked();

    if (this->waveformList.size() == 0)
    {
        displayError(QMessageBox::Warning, "Novel Waveform not yet loaded");
        return;
    }

    if (!stimulationConfigurations.isObject())
    {
        displayError(QMessageBox::Warning, "Stimulation Configuration not imported");
        return;
    }

    if (!stimulationConfigurations.object().contains("StimulationSequence"))
    {
        displayError(QMessageBox::Warning, "Bad Stimulation Configuration");
        return;
    }

    stimulationElapsedTime.restart();
    stimulationStateTimer->start(100);
    connect(stimulationStateTimer, &QTimer::timeout, this, &ControllerForm::stimulationStateUpdate);

    ui->StimulationControl_Start->setEnabled(false);
    ui->StimulationControl_Stop->setEnabled(true);

    novelStimulationStatus = true;
    startSequentialStimulation();
}

// Stop stimulation if any of the parameter changed.
void ControllerForm::on_StimulationControlConfigurationChanged(double value)
{
    // Suppress Unsed Parameter Warning. This Slot is used to turn off stimulation only.
    // Potential use of this slot is to update stimulation in real-time, by turning off stimulation -> change stimulation setting -> turn on stimulation.
    (void)value;
    if (ui->StimulationControl_Stop->isEnabled()) on_StimulationControl_Stop_clicked();
}

////////////////////////////////////
/////// Recording Callbacks ////////
////////////////////////////////////

// Reset all NeuroOmega Channels to disable saving to minimize storage. Only record the channels used in the surgery.
void ControllerForm::resetSaveStates()
{
    // Get all channels available on NeuroOmega
    uint32 channelCount = 0;
    int result = GetChannelsCount(&channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // Loading channel information
    SInformation channelsInfo[channelCount];
    result = GetAllChannels(channelsInfo, channelCount);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // Turn off all saving states.
    for (int i = 0; i < channelCount; i++)
    {
        if (channelsInfo[i].channelID >= 10272 && channelsInfo[i].channelID <= 10335)
        result = SetChannelSaveState(channelsInfo[i].channelID, false);
        if (result != eAO_OK)
        {
            QString messsage = getErrorLog();
            displayError(QMessageBox::Warning, messsage);
            return;
        }
    }
}

bool ControllerForm::configureRecordingChannels()
{
    // Turn Save State to False for all channels
    resetSaveStates();

    // Given the 4 existing channels.
    for (int i = 0; i < this->electrodeConfigurations.size(); i++)
    {
        for (int j = 0; j < this->electrodeConfigurations[i].numContacts; j++)
        {
            if (this->electrodeConfigurations[i].channelIDs[j] > 0)
            {
                int result = SetChannelName(this->electrodeConfigurations[i].channelIDs[j], "temp", 4);
                if (result != eAO_OK)
                {
                    QString messsage = getErrorLog();
                    displayError(QMessageBox::Warning, messsage);
                    return false;
                }

                QString channelName = "Lead_" + QString::number(i+1) + "_" + this->electrodeConfigurations[i].hemisphere + "_" + this->electrodeConfigurations[i].target + "_" + QString::number(j);

                result = SetChannelName(this->electrodeConfigurations[i].channelIDs[j], (char*)channelName.toStdString().c_str(), channelName.length());
                if (result != eAO_OK)
                {
                    QString messsage = getErrorLog();
                    displayError(QMessageBox::Warning, messsage);
                    return false;
                }

                result = SetChannelSaveState(this->electrodeConfigurations[i].channelIDs[j], true);
                if (result != eAO_OK)
                {
                    QString messsage = getErrorLog();
                    displayError(QMessageBox::Warning, messsage);
                    return false;
                }
            }
        }
    }

    // Stim Marker Channel
    int result = SetChannelSaveState(11221, true);
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return false;
    }

    return true;
}

// Perform different recording task based on the annotation.
//      This function is customizable. Please include your own pipeline to handle different pre-defined task.
//      As an example, we used "4 Contacts\nResearch Stim" and "8 Contacts\nResearch Stim" to start the NovelStimulation.
void ControllerForm::updateAnnotation(QString annotations, QJsonDocument loadedDocument)
{
    // 4-Contact Electrode (i.e. Medtronic 3387, 3389)
    if (!loadedDocument.isEmpty())
    {
        // Stop Current Stimulation in order to start the novel stimulation
        on_StimulationControl_Stop_clicked();

        // Parsing Stimulation JSON file. Check to see if the JSON is valid here.
        if (loadedDocument.isObject())
        {
            QJsonObject stimulationConfiguration = loadedDocument.object();
            if (stimulationConfiguration.contains("StimulationName")) this->stimulationConfigurations = loadedDocument;
        }

        // Notify user if stimulation JSON file is not valid.
        if (!stimulationConfigurations.isObject())
        {
            displayError(QMessageBox::Warning, "Stimulation Configuration not imported");
            return;
        }

        // Notify user if the JSON is not in the correct format
        if (!stimulationConfigurations.object().contains("StimulationSequence"))
        {
            displayError(QMessageBox::Warning, "Bad Stimulation Configuration");
            return;
        }

        bool analogWaveformNeeded = false;
        QJsonArray stimulationSequences = stimulationConfigurations.object()["StimulationSequence"].toArray();
        while(ui->SequenceDisplayTable->rowCount() > 0)
        {
            ui->SequenceDisplayTable->removeRow(0);
        }
        for (int i = 0; i < stimulationSequences.size(); i++)
        {
            ui->SequenceDisplayTable->insertRow(i);
            ui->SequenceDisplayTable->setItem(i, 0, new QTableWidgetItem(stimulationSequences[i].toObject()["StimulationType"].toString()));

            ui->SequenceDisplayTable->setItem(i, 1, new QTableWidgetItem(stimulationSequences[i].toObject()["RecordingFilename"].toString()));
            ui->SequenceDisplayTable->item(i,1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

            QString recordingDuration = QString::number(stimulationSequences[i].toObject()["Duration"].toInt()) + " sec";
            ui->SequenceDisplayTable->setItem(i, 2, new QTableWidgetItem(recordingDuration));
            ui->SequenceDisplayTable->item(i,2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

            if (ui->SequenceDisplayTable->item(i,0)->text() == "Novel")
            {
                analogWaveformNeeded = true;
            }

            if (ui->SequenceDisplayTable->item(i,0)->text() != "Baseline")
            {
                QString leadName;
                ui->StimulationControl_Electrode->count();
                int leadID = stimulationSequences[i].toObject()["StimulationLead"].toInt();
                if (leadID == -1)
                {
                    ui->SequenceDisplayTable->setItem(i, 3, new QTableWidgetItem(ui->StimulationControl_Electrode->currentText()));
                }
                else
                {
                    if (leadID < ui->StimulationControl_Electrode->count() - 1)
                    {
                        ui->SequenceDisplayTable->setItem(i, 3, new QTableWidgetItem(ui->StimulationControl_Electrode->itemText(leadID+1)));
                    }
                    else
                    {
                        displayError(QMessageBox::Critical, "Bad definition on Lead ID");
                        return;
                    }
                }

                QString stimulationContactsText;
                QJsonArray stimulationContacts = stimulationSequences[i].toObject()["StimulationChannel"].toArray();
                for (int j = 0; j < stimulationContacts.size(); j++)
                {
                    stimulationContactsText += "-E" + QString::number(stimulationContacts[j].toInt()) + " ";
                }
                if (stimulationSequences[i].toObject()["StimulationReturn"].toInt() >= 0)
                {
                    stimulationContactsText += "+E" + QString::number(stimulationSequences[i].toObject()["StimulationReturn"].toInt()) + " ";
                }
                else
                {
                    stimulationContactsText += "+CAN";
                }
                ui->SequenceDisplayTable->setItem(i, 4, new QTableWidgetItem(stimulationContactsText));

                if (ui->SequenceDisplayTable->item(i,0)->text() == "Standard")
                {
                    ui->SequenceDisplayTable->setItem(i, 5, new QTableWidgetItem(QString("%1 mA").arg(stimulationSequences[i].toObject()["Amplitude"].toInt())));
                    ui->SequenceDisplayTable->setItem(i, 6, new QTableWidgetItem(QString("%1 µS").arg(stimulationSequences[i].toObject()["Pulsewidth"].toInt())));
                    ui->SequenceDisplayTable->setItem(i, 7, new QTableWidgetItem(QString("%1 Hz").arg(stimulationSequences[i].toObject()["Frequency"].toInt())));
                }
            }
        }
        ui->SequenceDisplayTable->verticalScrollBar()->setValue(0);
        ui->SequenceDisplayTable->horizontalScrollBar()->setValue(0);

        if (analogWaveformNeeded)
        {
            if (stimulationConfigurations.object().contains("AnalogWaveforms"))
            {
                loadAnalogWaveform(stimulationConfigurations.object()["AnalogWaveforms"].toArray());
                for (int i = 0; i < stimulationSequences.size(); i++)
                {
                    if (stimulationSequences[i].toObject()["StimulationType"].toString() == "Novel")
                    {
                        ui->SequenceDisplayTable->setItem(i, 0, new QTableWidgetItem(analogWaveformDescriptor[stimulationSequences[i].toObject()["StimulationIndex"].toInt()].wavename));
                    }
                }
            }
            else
            {
                displayError(QMessageBox::Warning, "Novel Waveform not in configurations");
                return;
            }
        }

        // Notify user if no novel waveform loaded.
        if (analogWaveformNeeded && this->preloadedAnalogWaveforms.count() == 0)
        {
            displayError(QMessageBox::Warning, "Novel Waveform not yet loaded");
            return;
        }

        ui->SequenceFilename->setText(this->stimulationConfigurations.object()["StimulationName"].toString());

        // Start Stimlation Elapsed Time timer. Background UI-updater operate every 100ms to update the progress bar.
        // The stimulationStateUpdate function will initiate stimulation as needed according to the JSON file.
        stimulationElapsedTime.restart();
        stimulationStateTimer->start(100);
        connect(stimulationStateTimer, &QTimer::timeout, this, &ControllerForm::stimulationStateUpdate);

        // Disable Start button before we start the sequential stimulation programmatically.
        ui->StimulationControl_Start->setEnabled(false);
        ui->StimulationControl_Stop->setEnabled(true);
        this->currentStimulationStage = 0;
        novelStimulationStatus = true;
        startSequentialStimulation();
        ui->SequenceDisplayTable->setVisible(true);
    }
    else
    {
        // This is the generic recording starting pipeline. It does not configure stimulation nor other information.
        // We will attempt to update NeuroOmega SaveFile name and request saving start.

        // Update Filename. Filename format is yyyyMMdd_diagnosis_patientID_annotation.
        // Modify this to make your own file-naming convention.
        QString filename = "";
        QDateTime currentTime;
        filename = filename + currentTime.currentDateTime().toString("yyyyMMdd") + "_" + QString::fromStdString(this->diagnosis) + "_" + QString::fromStdString(this->patientID) + "_" + annotations;

        // Update NeuroOmega SaveFile name.
        //      IMPORTANT: This will not work if you set NeuroOmega to automatically update filename. We cannot overwrite configuration in NeuroOmega Application Interface.
        int result = SetSaveFileName((char*)filename.toStdString().c_str(), filename.length());
        if (result != eAO_OK)
        {
            QString messsage = getErrorLog();
            displayError(QMessageBox::Warning, messsage);
            return;
        }

        // Making this recording infinite recording. This is configured to prevent "Stop Stimulation" from turning off recording.
        if (!novelStimulationStatus) infiniteRecording = true;
    }
    recordingStatus = true;
}

void ControllerForm::on_NeuroOmega_RecordingStart_clicked()
{
    // Pop-up dialog to request user to indicate which type of recording is this. Theoretical sequential program to run next is UpdateAnnotation().
    RecordingAnnotation annotationSelection;
    annotationSelection.setFixedSize(annotationSelection.size());
    connect(&annotationSelection, &RecordingAnnotation::channelIDsUpdate, this, &ControllerForm::updateAnnotation);
    annotationSelection.exec();

    // If we closed the annotation window without choosing recording type, or there are errors in the configuration, the recordingStatus will not be set to True.
    // Wait for 3 seconds, or if recordingStatus is true, we immediately proceed to the new
    recordingElapsedTime.restart();
    for (int i = 0; i < 3; i++)
    {
        if (!recordingStatus) Sleep(1000);
        else break;
    }

    // if recordingStatus is not true, we immediately return.
    if (!recordingStatus)
    {
        displayError(QMessageBox::Critical, "Recording cannot start in 10 seconds. Check NeuroOmega Connection.");
        return;
    }

    // Request NeuroOmega to start saving files
    int result = StartSave();
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // Update UIs
    recordingElapsedTime.restart();
    ui->NeuroOmega_RecordingStart->setEnabled(false);
    ui->NeuroOmega_RecordingStop->setEnabled(true);
    ui->RecordingLabels_1->setEnabled(true);
    ui->RecordingLabels_2->setEnabled(true);
    ui->RecordingLabels_3->setEnabled(true);
    ui->RecordingLabels_4->setEnabled(true);
    ui->RecordingLabels_5->setEnabled(true);
    ui->RecordingLabels_6->setEnabled(true);
}


void ControllerForm::on_NeuroOmega_RecordingStop_clicked()
{
    // Request NeuroOmega to stop recording. Notify user if failed.
    int result = StopSave();
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // If this is a NovelStimulation Recording. Stop the NovelStimulation as well (unless it is infinite streaming)
    if (novelStimulationStatus && !infiniteRecording) on_StimulationControl_Stop_clicked();

    // UI Update if Stopping successfully.
    // recordingElapsedTime.restart();
    ui->RecordingDurationLabel->setText("STOPPED");
    recordingStatus = false;
    infiniteRecording = false;
    ui->NeuroOmega_RecordingStart->setEnabled(true);
    ui->NeuroOmega_RecordingStop->setEnabled(false);
    ui->RecordingLabels_1->setEnabled(false);
    ui->RecordingLabels_2->setEnabled(false);
    ui->RecordingLabels_3->setEnabled(false);
    ui->RecordingLabels_4->setEnabled(false);
    ui->RecordingLabels_5->setEnabled(false);
    ui->RecordingLabels_6->setEnabled(false);
    ui->SequenceDisplayTable->setVisible(false);
}

// General Slot() for clicking recording labels. Update UI file to make them your favourite texts to label during recording.
void ControllerForm::on_RecordingLabelClicked()
{
    QPushButton* buttonClicked = qobject_cast<QPushButton*>(sender());
    sendLabelMessages(buttonClicked->text());
}

// Or just use the custom label button to put your own texts.
void ControllerForm::on_CustomLabelClicked()
{
    ManualLabelEntry labelEntryDialog;
    labelEntryDialog.setFixedSize(labelEntryDialog.size());
    connect(&labelEntryDialog, &ManualLabelEntry::labelComplete, this, &ControllerForm::sendLabelMessages);
    labelEntryDialog.exec();
}

void ControllerForm::sendLabelMessages(QString messages)
{
    // Empty message can be due to clicking "Cancel" on ManualLabelEntry dialog
    if (messages == "") return;

    // Attempt sending message to NeuroOmega, notify user if failed.
    int result = SendText((char*)messages.toStdString().c_str(), messages.length());
    if (result != eAO_OK)
    {
        QString messsage = getErrorLog();
        displayError(QMessageBox::Warning, messsage);
        return;
    }

    // If not failed, log the label externally as text for reading
    QJsonObject jsonObject;
    jsonObject["ObjectType"] = QJsonValue("Label");
    jsonObject["LabelText"] = QJsonValue(messages);
    QDateTime currentTime;
    jsonObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    jsonStorage->addJSON(jsonObject);
}

////////////////////////////////////////////////////////
////// Extra JSON Loggings and Text Note Buttons ///////
////////////////////////////////////////////////////////
// We send benefit scores to NeuroOmega log. Note that the message we send are based on Object Names.
void ControllerForm::on_BenefitsClicked()
{
    QPushButton* buttonClicked = qobject_cast<QPushButton*>(sender());
    sendLabelMessages(buttonClicked->objectName());
}

// Similarly, we send side effects to NeuroOmega log, but this time we send them based on Object Text.
void ControllerForm::on_SideEffectsClicked()
{
    QPushButton* buttonClicked = qobject_cast<QPushButton*>(sender());
    sendLabelMessages(buttonClicked->text());
    if (buttonClicked->text() == "Persistent" || buttonClicked->text() == "Transient") writeSideEffectNotes(buttonClicked->text());
}

// Text writer for side effect labels
void ControllerForm::writeSideEffectNotes(QString sideEffectType)
{
    // Writer Class
    QTextStream textOutstream(this->sideEffectNotes);

    QPushButton *allButtons[] = {ui->StimulationContact_E00,
                                 ui->StimulationContact_E01_1, ui->StimulationContact_E01_2, ui->StimulationContact_E01_3,
                                 ui->StimulationContact_E02_1, ui->StimulationContact_E02_2, ui->StimulationContact_E02_3,
                                 ui->StimulationContact_E03,
                                 ui->StimulationContact_GlobalCAN};
    textOutstream << ui->StimulationControl_Electrode->currentText() << "\n";
    for (int i = 0; i < 9; i++)
    {
        if (allButtons[i]->styleSheet() == anodeStyle)
        {
            QString str = allButtons[i]->text();
            str = str.replace("Contact\n","");
            textOutstream << str << "- ";
        }
        if (allButtons[i]->styleSheet() == cathodeStyle)
        {
            QString str = allButtons[i]->text();
            str = str.replace("Contact\n","");
            textOutstream << allButtons[i]->text() << "+ ";
        }
    }
    textOutstream << "\n" << sideEffectType << " side effects\n";

    textOutstream << "\t" << ui->StimulationControl_Amplitude->value() << " mA\n";
    textOutstream << "\t" << ui->StimulationControl_Pulsewidth->value() << " uS\n";
    textOutstream << "\t" << ui->StimulationControl_Frequency->value() << " Hz\n\n";
}

////////////////////////////////////////////////////////////////////
////// Advance Configuration Interfaces for Research Purpose ///////
////////////////////////////////////////////////////////////////////
// Novel Stimulation Configuration Window
void ControllerForm::on_StimulationControl_Novel_clicked()
{
    NovelStimulationConfiguration configurationWindow;
    configurationWindow.setupDefault(this->waveformList, this->currentWaveformID, this->stimulationConfigurations);
    configurationWindow.setFixedSize(configurationWindow.size());
    connect(&configurationWindow, &NovelStimulationConfiguration::novelConfigurations, this, &ControllerForm::novelStimulationParametersUpdate);
    configurationWindow.exec();
}

void ControllerForm::novelStimulationParametersUpdate(QStringList waveformList, int selectedWave, QJsonDocument stimulationJsonDocument)
{
    this->currentWaveformID = selectedWave;
    this->waveformList = waveformList;
    this->stimulationConfigurations = stimulationJsonDocument;
    ui->SequenceFilename->setText(this->stimulationConfigurations.object()["StimulationName"].toString());
}

// Pull up advance configuration for channel lists
void ControllerForm::on_Electrodes_AdvanceConfiguration_clicked()
{
    DetailChannelsList channelListView;
    channelListView.setFixedSize(channelListView.size());
    channelListView.setupChannels();
    channelListView.exec();
}

void ControllerForm::loadAnalogWaveform(QJsonArray filenameArray)
{
    if (!this->preloadedAnalogWaveforms.isEmpty())
    {
        for (int i = 0; i < this->preloadedAnalogWaveforms.count(); i++)
        {
            free(this->preloadedAnalogWaveforms[i]);
        }
        this->preloadedAnalogWaveforms.clear();
    }

    try
    {
        for (int i = 0; i < filenameArray.count(); i++)
        {
            QFile file(qApp->applicationDirPath() + "/" + filenameArray[i].toString());
            if (!file.open(QIODevice::ReadOnly))
            {
                throw -1;
            }

            QByteArray byteArray = file.readAll();
            char *data = byteArray.data();

            AnalogWaveformDescriptor overview;
            overview.filesize = file.size() / 2;
            overview.wavename = QFileInfo(qApp->applicationDirPath() + "/" + filenameArray[i].toString()).fileName().split(".").first();
            analogWaveformDescriptor.append(overview);

            int16 *stimulationVector = (int16*)malloc(file.size());
            memcpy(stimulationVector, data, file.size());
            this->preloadedAnalogWaveforms.append(stimulationVector);

        }
    }  catch (int errorCode) {
        qDebug() << "Error Occurred";
        if (!this->preloadedAnalogWaveforms.isEmpty())
        {
            for (int i = 0; i < this->preloadedAnalogWaveforms.count(); i++)
            {
                free(this->preloadedAnalogWaveforms[i]);
            }
            this->preloadedAnalogWaveforms.clear();
        }
    }

}
