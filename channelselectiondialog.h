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

#ifndef CHANNELSELECTIONDIALOG_H
#define CHANNELSELECTIONDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QString>
#include <QLayout>
#include <QTableWidget>

namespace Ui {
class ChannelSelectionDialog;
}

typedef struct InputChannelUIWidgets
{
    QTableWidgetItem *inputName;
    QComboBox *boxName;
    QComboBox *channelName;
} InputChannelUIWidgets;

class ChannelSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelSelectionDialog(QWidget *parent = 0);
    ~ChannelSelectionDialog();

    void displayError(int errorLevel, QString message);
    void addNewInputChannel(int rowID);
    void configureContactNumbers(QString electrodeType, int electrodeID, int channelCount);
    void configurePredefinedChannels(int* channelIDs);

signals:
    void channelIDsUpdate(int *newChannelIDs, int len, int electrodeID);

private slots:
    void on_ChannelConfirm_clicked();
    void on_ChangeAllChannels_clicked();
    void batchSelectionChanged();
    void batchChangeECoGBox(int index);

private:
    Ui::ChannelSelectionDialog *ui;
    QList<InputChannelUIWidgets> inputChannelWidgetsCollection;
    QList<int> selectedRows;

    QString electrodeType;
    int electrodeID = 0;
    int channelCount = 4;
    int *channelIDs;
};

#endif // CHANNELSELECTIONDIALOG_H
