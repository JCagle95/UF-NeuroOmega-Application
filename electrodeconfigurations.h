#ifndef ELECTRODECONFIGURATIONS_H
#define ELECTRODECONFIGURATIONS_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLayout>

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
    int channelID[8] = {0};
    QVector<int> channelIDs;
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

    void on_ChannelConfirm_clicked();

private:
    Ui::ElectrodeConfigurations *ui;
    QGridLayout *scrollAreaLayout;
    QList<ElectrodeConfigurationUIWidgets> electrodeWidgetsCollection;

    QJsonObject interfaceConfiguration;
    QStringList availableElectrodeNames;
    QStringList availableTargetNames;
};

#endif // ELECTRODECONFIGURATIONS_H
