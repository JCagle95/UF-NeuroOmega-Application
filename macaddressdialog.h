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

#ifndef MACADDRESSDIALOG_H
#define MACADDRESSDIALOG_H

#include <cstring>
#include <QDialog>

using namespace std;

namespace Ui {
class MACAddressDialog;
}

class MACAddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MACAddressDialog(QWidget *parent = 0, string address = "", string targetDirectory = "");
    ~MACAddressDialog();

signals:
    void addressUpdated(string address, string targetDirectory);

private slots:
    void on_AddressConfirm_clicked();
    void on_AddressCancel_clicked();

private:
    Ui::MACAddressDialog *ui;
};

#endif // MACADDRESSDIALOG_H
