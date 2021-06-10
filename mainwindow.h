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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>
#include <QMainWindow>
#include <QSettings>
#include <QDir>

#include <cstring>
#include <windows.h>

#include "macaddressdialog.h"
#include "electrodeconfigurations.h"
#include "controllerform.h"

#ifdef QT_DEBUG
#include "AOSystemAPI_TEST.h"
#else
#include "AOSystemAPI.h"
#endif
#include "AOTypes.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);

    string getErrorLog();
    void displayError(int errorLevel, const char *message);

    void onConnectionUpdate();
    void checkConnection();
    MAC_ADDR formMACAddress(string addressString);
    void updateAddresses(string address, string targetDirectory);

private slots:
    void on_NeuroOmega_BtnConnect_clicked();
    void on_SystemMacAddressEdit_clicked();
    void on_patientID_textEdit_textChanged(const QString &text);
    void on_diagnosisSelection_currentTextChanged(const QString &text);

private:
    ElectrodeConfigurations *configurationForm;
    ControllerForm *controllerForm;

    Ui::MainWindow *ui;
    QSettings *applicationConfiguration;

    bool connectionStatus;
    string patientID;
    string diagnosis;
};

#endif // MAINWINDOW_H
