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

#include "channelselectiondialog.h"
#include "ui_channelselectiondialog.h"

ChannelSelectionDialog::ChannelSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChannelSelectionDialog)
{
    ui->setupUi(this);

    scrollAreaLayout = new QGridLayout();
    ui->ScrollAreaWidget->setLayout(scrollAreaLayout);

    scrollAreaLayout->setColumnStretch(0, 2);
    scrollAreaLayout->setColumnStretch(1, 2);
    scrollAreaLayout->setColumnStretch(2, 3);
}

ChannelSelectionDialog::~ChannelSelectionDialog()
{
    delete ui;
}

void ChannelSelectionDialog::displayError(int errorLevel, QString message)
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


void ChannelSelectionDialog::addNewInputChannel(int rowID)
{
    InputChannelUIWidgets widgets;
    QFont standardFont("Microsoft YaHei UI", 12);

    widgets.inputName = new QLabel();
    widgets.inputName->setText(QString("Contact #%1").arg(rowID));
    widgets.inputName->setFont(standardFont);
    widgets.inputName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    widgets.inputName->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.inputName, rowID, 0);

    widgets.boxName = new QComboBox();
    widgets.boxName->addItem("ECoG Box 01");
    widgets.boxName->addItem("ECoG Box 02");
    widgets.boxName->addItem("ECoG Box 03");
    widgets.boxName->addItem("ECoG Box 04");
    widgets.boxName->setFont(standardFont);
    widgets.boxName->setMinimumHeight(30);
    widgets.boxName->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.boxName, rowID, 1);

    widgets.channelName = new QComboBox();
    for (int i = 0; i < 16; i++)
    {
        widgets.channelName->addItem(QString("Channel %1").arg(i+1));
    }
    widgets.channelName->setFont(standardFont);
    widgets.channelName->setMinimumHeight(30);
    widgets.channelName->setProperty("rowID", rowID);
    scrollAreaLayout->addWidget(widgets.channelName, rowID, 2);

    inputChannelWidgetsCollection.append(widgets);
}


void ChannelSelectionDialog::configureContactNumbers(QString electrodeType, int electrodeID, int channelCount)
{
    this->channelCount = channelCount;
    for (int i = 0; i < channelCount; i++) addNewInputChannel(i);
    channelIDs = new int[channelCount];
    this->electrodeID = electrodeID;
}

void ChannelSelectionDialog::configurePredefinedChannels(int* channelIDs)
{
    for (int i = 0; i < channelCount; i++)
    {
        if (channelIDs[i] <= 0)
        {
            inputChannelWidgetsCollection[i].boxName->setCurrentIndex(0);
            inputChannelWidgetsCollection[i].channelName->setCurrentIndex(i);
        }
        else
        {
            int channelID = channelIDs[i] - 10272;
            inputChannelWidgetsCollection[i].boxName->setCurrentIndex(channelID / 16);
            inputChannelWidgetsCollection[i].channelName->setCurrentIndex(channelID % 16);
        }
    }
}

void ChannelSelectionDialog::on_ChannelConfirm_clicked()
{
    for (int i = 0; i < channelCount - 1; i++)
    {
        for (int j = i + 1; j < channelCount; j++)
        {
            if (inputChannelWidgetsCollection[i].boxName->currentIndex() == inputChannelWidgetsCollection[j].boxName->currentIndex() &&
                inputChannelWidgetsCollection[i].channelName->currentIndex() == inputChannelWidgetsCollection[j].channelName->currentIndex())
            {
                displayError(QMessageBox::Critical, inputChannelWidgetsCollection[i].boxName->currentText() + " " + inputChannelWidgetsCollection[i].channelName->currentText() + " is assigned to more than 1 contacts");
                return;
            }
        }
    }

    for (int i = 0; i < channelCount; i++)
    {
        channelIDs[i] = 10272 + (inputChannelWidgetsCollection[i].boxName->currentIndex() * 16 ) + inputChannelWidgetsCollection[i].channelName->currentIndex();
    }

    emit channelIDsUpdate(channelIDs, channelCount, electrodeID);
    this->close();
}

void ChannelSelectionDialog::on_ChangeAllChannels_clicked()
{
    for (int i = 0; i < channelCount; i++)
    {
        inputChannelWidgetsCollection[i].boxName->setCurrentIndex(inputChannelWidgetsCollection[0].boxName->currentIndex());
        inputChannelWidgetsCollection[i].channelName->setCurrentIndex(inputChannelWidgetsCollection[0].channelName->currentIndex() + i);
    }
}
