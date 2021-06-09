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

#ifndef CONTROLLERFORM_H
#define CONTROLLERFORM_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QElapsedTimer>

#include <cstring>
#include <windows.h>
#include <ctime>

#ifdef QT_DEBUG
#include "AOSystemAPI_TEST.h"
#else
#include "AOSystemAPI.h"
#endif
#include "AOTypes.h"
#include "jsonstorage.h"
#include "channelselectiondialog.h"
#include "detailchannelslist.h"
#include "recordingannotation.h"
#include "manuallabelentry.h"
#include "novelstimulationconfiguration.h"
#include "realtimestream.h"

using namespace std;

namespace Ui {
class ControllerForm;
}

typedef struct ElectrodeInformation
{
    QString electrodeType = "";
    QString hemisphere = "";
    QString target = "";
    int channelID[8] = {0};
    int numContacts = 0;
    bool verified = false;
} ElectrodeInformation;

class ControllerForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControllerForm(QWidget *parent = 0);
    void controllerInitialization(string patientID, string diagnosis);
    void closeEvent(QCloseEvent *event);
    ~ControllerForm();

    QString getErrorLog();
    void displayError(int errorLevel, QString message);
    void checkStatus();

    void configureElectrodeConfigurationText(QString electrodeSelectorName, int type);

    void resetCathodeSelection();
    void setupElectrodeButtons(QString electrodeName);
    void updateChannelConfiguration(int *channelIDs, int len, int electrodeID);
    void stimulationContactPressed(QPushButton* button);
    void formatElectrodeConfiguration();

    void resetSaveStates();
    bool configureRecordingChannels();
    void stimulationStateUpdate();
    void sendLabelMessages(QString messages);
    void updateAnnotation(QString annotation);
    void recordingStateUpdate();

    void novelStimulationParametersUpdate(QStringList waveNames, int selectedWave, QJsonDocument stimulationJsonDocument);
    void startSequentialStimulation();

signals:
    void connectionChanged();

private slots:
    void on_ElectrodeTypeSelectionCurrentIndexChanged(QString electrode);
    void on_Electrodes_BtnConfiguration_clicked();
    void on_ElectrodeSensorSliderChanged(int value);
    void on_channelSelectionDialogClicked();

    void on_StimulationControl_Electrode_currentTextChanged(const QString &electrodeName);

    void on_StimulationContactClicked();
    void on_StimulationContact_GlobalCAN_clicked();
    void on_StimulationControl_Start_clicked();
    void on_StimulationControl_Stop_clicked();

    void on_NeuroOmega_RecordingStart_clicked();
    void on_NeuroOmega_RecordingStop_clicked();

    void on_Electrodes_AdvanceConfiguration_clicked();

    void on_StimulationContact_Ring01_clicked();
    void on_StimulationContact_Ring02_clicked();

    void on_RecordingLabelClicked();
    void on_CustomLabelClicked();

    void on_BenefitsClicked();
    void on_SideEffectsClicked();
    void writeSideEffectNotes(QString sideEffectType);

    void on_StimulationControlConfigurationChanged(double value);

    void on_StimulationControl_Novel_clicked();
    void on_StimulationControl_Novel_Start_clicked();


    void on_RealtimeStreamDisplay_clicked();
    void streamWindowClosed(void);

private:
    Ui::ControllerForm *ui;
    QSettings *applicationConfiguration;

    string patientID = "";
    string diagnosis = "";

    ElectrodeInformation electrodeConfiguration[4];
    QList<int> allContacts;

    // Cathode is "+", Anode is "-". The Global Return can only be cathode "+".
    // There can only be 1 return channel in this program to simply things.
    QList<int> StimulationAnode;
    int StimulationCathode = 0;

    // Pre-defined QPushButton Style for easier visualization of button color. Change the color to your prefer color
    QString anodeStyle = "QPushButton {background-color: rgb(255, 88, 99); border: none}";
    QString cathodeStyle = "QPushButton {background-color: rgb(83, 175, 255); border: none}";
    QString noneStyle = "";

    // Initialization of QTimer variables. These are used for periodic tasks.
    QTimer *connectionCheck;
    QTimer *stimulationStateTimer;
    QTimer *recordingStateTimer;

    // Elapsed Time timer to keep track of task durations.
    QElapsedTimer stimulationElapsedTime;
    QElapsedTimer recordingElapsedTime;
    bool infiniteRecording = false;
    bool recordingStatus = false;

    // JSON Storage Class for Loggings
    JSONStorage *jsonStorage;
    QFile *sideEffectNotes;

    // Novel Stimulation Parameters
    QJsonDocument stimulationConfigurations;
    bool currentStimulationState = false;
    int currentStimulationStage = 0;
    bool novelStimulationStatus = false;
    QString currentProgrammedFilename = "";
    QStringList waveformList;
    int currentWaveformID = -1;

    // Realtime Stream QT Form
    RealtimeStream *streamView;
    ElectrodeInformation currentElectrodeConfiguration;
};

#endif // CONTROLLERFORM_H
