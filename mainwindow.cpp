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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connectionStatus = false;
    diagnosis = "PD";
    patientID = "";

    applicationConfiguration = new QSettings(QDir::currentPath() + "/defaultSettings.ini", QSettings::IniFormat);
    updateAddresses(applicationConfiguration->value("SystemMACAddress").toString().toStdString(), applicationConfiguration->value("SurgicalLogFolder").toString().toStdString());

    this->setFixedSize(this->size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    AO_Exit();
}

string MainWindow::getErrorLog()
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

    return string(errorString);
}

void MainWindow::displayError(int errorLevel, const char* message)
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

void MainWindow::checkConnection()
{
    int result = isConnected();
    switch (result)
    {
        case eAO_DISCONNECTED:
            connectionStatus = false;
            break;
        case eAO_CONNECTED:
            connectionStatus = true;
            break;
    }
}

void MainWindow::onConnectionUpdate()
{
    checkConnection();
    if (!connectionStatus)
    {
        ui->NeuroOmega_BtnConnect->setEnabled(true);
        ui->patientID_textEdit->setEnabled(true);
        ui->diagnosisSelection->setEnabled(true);
    }
}

MAC_ADDR MainWindow::formMACAddress(string addressString)
{
    MAC_ADDR macAddress = {0};
    sscanf(addressString.c_str(), "%x:%x:%x:%x:%x:%x",
        &macAddress.addr[0], &macAddress.addr[1], &macAddress.addr[2], &macAddress.addr[3], &macAddress.addr[4], &macAddress.addr[5]);
    return macAddress;
}

void MainWindow::on_NeuroOmega_BtnConnect_clicked()
{
    checkConnection();

    if (!connectionStatus)
    {
        MAC_ADDR sysMACAddress = formMACAddress(applicationConfiguration->value("SystemMACAddress").toString().toStdString());

        int result = DefaultStartConnection(&sysMACAddress, NULL);
        if (result != eAO_OK)
        {
            displayError(QMessageBox::Critical, getErrorLog().c_str());
            return;
        }

        connectionStatus = true;
        int currentStatus = 0;
        for (int i = 0; i < 10; i++)
        {
            currentStatus = isConnected();
            if (currentStatus == eAO_DISCONNECTED)
            {
                displayError(QMessageBox::Critical, getErrorLog().c_str());
                return;
            }
            else if (currentStatus == eAO_CONNECTED)
            {
                connectionStatus = true;
                break;
            }
            if (i == 0)
            {
                connectionStatus = true;
                break;
            }
            Sleep(1000);
        }

        if (connectionStatus)
        {
            configurationForm = new ElectrodeConfigurations();
            configurationForm->setFixedSize(configurationForm->size());

            if (configurationForm->exec() == configurationForm->Accepted)
            {
                ui->NeuroOmega_BtnConnect->setEnabled(false);
                ui->patientID_textEdit->setEnabled(false);
                ui->diagnosisSelection->setEnabled(false);
                ui->SystemMacAddressEdit->setEnabled(false);

                controllerForm = new ControllerForm();
                controllerForm->controllerInitialization(this->patientID, this->diagnosis);
                controllerForm->configureElectrodes(configurationForm->electrodeInfoCollection);
                controllerForm->setFixedSize(controllerForm->size());
                connect(controllerForm, &ControllerForm::connectionChanged, this, &MainWindow::onConnectionUpdate);
                controllerForm->show();
            }
            else
            {
                CloseConnection();
            }

        }
        else
        {
            displayError(QMessageBox::Warning, "NeuroOmega has not responded in 10 seconds. Please check if the NeuroOmega Application is running.");
        }
    }
    else
    {
        displayError(QMessageBox::Warning, "NeuroOmega is already connected.");
    }
}

void MainWindow::updateAddresses(string address, string targetDirectory)
{
    string labelText = string("System MAC Address - ") + address;
    ui->MAC_AddressLabel->setText(labelText.c_str());
    if (address != applicationConfiguration->value("SystemMACAddress").toString().toStdString())
    {
        applicationConfiguration->setValue("SystemMACAddress", address.c_str());
    }
    applicationConfiguration->setValue("SurgicalLogFolder", targetDirectory.c_str());
}

void MainWindow::on_SystemMacAddressEdit_clicked()
{
    string addressString = applicationConfiguration->value("SystemMACAddress").toString().toStdString();
    string targetDirectory = applicationConfiguration->value("SurgicalLogFolder").toString().toStdString();

    MACAddressDialog addressDialog(this, addressString, targetDirectory);
    connect(&addressDialog, &MACAddressDialog::addressUpdated, this, &MainWindow::updateAddresses);
    addressDialog.setFixedSize(addressDialog.size());
    addressDialog.exec();
}

void MainWindow::on_patientID_textEdit_textChanged(const QString &text)
{
    this->patientID = text.toStdString();
}

void MainWindow::on_diagnosisSelection_currentTextChanged(const QString &text)
{
    this->diagnosis = text.toStdString();
}

