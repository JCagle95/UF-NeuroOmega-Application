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

#include "macaddressdialog.h"
#include "ui_macaddressdialog.h"

MACAddressDialog::MACAddressDialog(QWidget *parent, string address, string targetDirectory) :
    QDialog(parent),
    ui(new Ui::MACAddressDialog)
{
    ui->setupUi(this);
    ui->macAddressEdit->setText(address.c_str());
    ui->jsonStatusFolderEdit->setText(targetDirectory.c_str());
}

MACAddressDialog::~MACAddressDialog()
{
    delete ui;
}

void MACAddressDialog::on_AddressConfirm_clicked()
{
    emit addressUpdated(ui->macAddressEdit->text().toStdString(), ui->jsonStatusFolderEdit->text().toStdString());
    this->close();
}

void MACAddressDialog::on_AddressCancel_clicked()
{
    this->close();
}
