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

void ChannelSelectionDialog::configureContactNumbers(QString electrodeType, int electrodeID, int channelCount)
{
    this->channelCount = channelCount;
    if (electrodeType == "EMG Pads" || electrodeType == "ECG Sensors")
    {
        channelCount = 4;
        this->electrodeType = "Sensor";
        ui->electrodeImages->setVisible(false);
        ui->ChannelSelection_Ch1->insertItem(0, "None");
        ui->ChannelSelection_Ch2_2->insertItem(0, "None");
        ui->ChannelSelection_Ch3_2->insertItem(0, "None");
        ui->ChannelSelection_Ch4->insertItem(0, "None");
        ui->ChangeAllChannels->setHidden(true);
    }
    else if (electrodeType == "Boston Segmented")
    {
        this->electrodeType = "Lead";
    }
    else if (electrodeType == "Medtronic Segmented")
    {
        this->electrodeType = "Lead";
    }
    else if (electrodeType == "Medtronic 3387")
    {
        this->electrodeType = "Lead";
    }
    else if (electrodeType == "Medtronic 3389")
    {
        this->electrodeType = "Lead";
    }

    if (channelCount == 8)
    {
        ui->label_7->setText("Contact #1.2: ");
        ui->label_8->setText("Contact #2.2: ");
    }
    else
    {
        ui->ECOGBoxSelection_Ch2_1->setVisible(false);
        ui->ECOGBoxSelection_Ch2_3->setVisible(false);
        ui->ECOGBoxSelection_Ch3_1->setVisible(false);
        ui->ECOGBoxSelection_Ch3_3->setVisible(false);
        ui->ChannelSelection_Ch2_1->setVisible(false);
        ui->ChannelSelection_Ch2_3->setVisible(false);
        ui->ChannelSelection_Ch3_1->setVisible(false);
        ui->ChannelSelection_Ch3_3->setVisible(false);
        ui->label_10->setVisible(false);
        ui->label_11->setVisible(false);
        ui->label_12->setVisible(false);
        ui->label_13->setVisible(false);
    }

    channelIDs = new int[channelCount];
    this->electrodeID = electrodeID;
}

void ChannelSelectionDialog::configurePredefinedChannels(int* channelIDs)
{
    if (channelCount == 4)
    {
        QComboBox *boxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch4};
        QComboBox *channelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch4};
        for (int i = 0; i < channelCount; i++)
        {
            if (channelIDs[i] <= 0)
            {
                boxSelection[i]->setCurrentIndex(0);
                if (this->electrodeType != "Sensor")
                {
                    channelSelection[i]->setCurrentIndex(i);
                }
                else
                {
                    if (i < 2)
                    {
                        channelSelection[i]->setCurrentIndex(i + 9);
                    }
                    else
                    {
                        channelSelection[i]->setCurrentIndex(0);
                    }
                }
            }
            else
            {
                int channelID = channelIDs[i] - 10272;
                boxSelection[i]->setCurrentIndex(channelID / 16);

                if (this->electrodeType != "Sensor")
                {
                    channelSelection[i]->setCurrentIndex(channelID % 16);
                }
                else
                {
                    channelSelection[i]->setCurrentIndex(channelID % 16 + 1);
                }
            }
        }
    }
    else
    {
        QComboBox *boxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_3, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch2_1, ui->ECOGBoxSelection_Ch3_3, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch3_1, ui->ECOGBoxSelection_Ch4};
        QComboBox *channelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_3, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch2_1, ui->ChannelSelection_Ch3_3, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch3_1, ui->ChannelSelection_Ch4};
        for (int i = 0; i < channelCount; i++)
        {
            if (channelIDs[i] <= 0)
            {
                boxSelection[i]->setCurrentIndex(0);
                channelSelection[i]->setCurrentIndex(i);
            }
            else
            {
                int channelID = channelIDs[i] - 10272;
                boxSelection[i]->setCurrentIndex(channelID / 16);
                channelSelection[i]->setCurrentIndex(channelID % 16);
            }
        }
    }
}

void ChannelSelectionDialog::on_ChannelConfirm_clicked()
{
    if (channelCount == 4)
    {
        QComboBox *ECOGBoxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch4};
        QComboBox *ChannelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch4};

        for (int i = 0; i < channelCount - 1; i++)
        {
            if (this->electrodeType == "Sensor" && ChannelSelection[i]->currentIndex() == 0)
            {
                continue;
            }

            for (int j = i + 1; j < channelCount; j++)
            {
                if (ECOGBoxSelection[i]->currentIndex() == ECOGBoxSelection[j]->currentIndex() && ChannelSelection[i]->currentIndex() == ChannelSelection[j]->currentIndex())
                {
                    displayError(QMessageBox::Critical, ECOGBoxSelection[j]->currentText() + " " + ChannelSelection[i]->currentText() + " is assigned to more than 1 contacts");
                    return;
                }
            }
        }

        for (int i = 0; i < channelCount; i++)
        {
            if (this->electrodeType == "Lead")
            {
                channelIDs[i] = 10272 + (ECOGBoxSelection[i]->currentIndex() * 16 ) + ChannelSelection[i]->currentIndex();
            }
            else
            {
                if (ChannelSelection[i]->currentIndex() == 0)
                {
                    channelIDs[i] = -1;
                }
                else
                {
                    channelIDs[i] = 10272 + (ECOGBoxSelection[i]->currentIndex() * 16 ) + ChannelSelection[i]->currentIndex() - 1;
                }
            }
        }
    }
    else
    {
        QComboBox *ECOGBoxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_3, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch2_1, ui->ECOGBoxSelection_Ch3_3, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch3_1, ui->ECOGBoxSelection_Ch4};
        QComboBox *ChannelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_3, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch2_1, ui->ChannelSelection_Ch3_3, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch3_1, ui->ChannelSelection_Ch4};
        for (int i = 0; i < channelCount - 1; i++)
        {
            for (int j = i + 1; j < channelCount; j++)
            {
                if (ECOGBoxSelection[i]->currentIndex() == ECOGBoxSelection[j]->currentIndex() && ChannelSelection[i]->currentIndex() == ChannelSelection[j]->currentIndex())
                {
                    displayError(QMessageBox::Critical, ECOGBoxSelection[j]->currentText() + " " + ChannelSelection[i]->currentText() + " is assigned to more than 1 contacts");
                    return;
                }
            }
        }

        for (int i = 0; i < channelCount; i++)
        {
            channelIDs[i] = 10272 + (ECOGBoxSelection[i]->currentIndex() * 16) + ChannelSelection[i]->currentIndex();
        }
    }

    emit channelIDsUpdate(channelIDs, channelCount, electrodeID);
    this->close();
}

void ChannelSelectionDialog::on_ChangeAllChannels_clicked()
{
    if (channelCount == 4)
    {
        QComboBox *ECOGBoxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch4};
        QComboBox *ChannelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch4};

        for (int i = 1; i < channelCount; i++)
        {
            ECOGBoxSelection[i]->setCurrentIndex(ECOGBoxSelection[0]->currentIndex());
            ChannelSelection[i]->setCurrentIndex(ChannelSelection[0]->currentIndex() + i);
        }
    }
    else
    {
        QComboBox *ECOGBoxSelection[] = {ui->ECOGBoxSelection_Ch1, ui->ECOGBoxSelection_Ch2_3, ui->ECOGBoxSelection_Ch2_2, ui->ECOGBoxSelection_Ch2_1, ui->ECOGBoxSelection_Ch3_3, ui->ECOGBoxSelection_Ch3_2, ui->ECOGBoxSelection_Ch3_1, ui->ECOGBoxSelection_Ch4};
        QComboBox *ChannelSelection[] = {ui->ChannelSelection_Ch1, ui->ChannelSelection_Ch2_3, ui->ChannelSelection_Ch2_2, ui->ChannelSelection_Ch2_1, ui->ChannelSelection_Ch3_3, ui->ChannelSelection_Ch3_2, ui->ChannelSelection_Ch3_1, ui->ChannelSelection_Ch4};
        for (int i = 1; i < channelCount; i++)
        {
            ECOGBoxSelection[i]->setCurrentIndex(ECOGBoxSelection[0]->currentIndex());
            ChannelSelection[i]->setCurrentIndex(ChannelSelection[0]->currentIndex() + i);
        }
    }
}
