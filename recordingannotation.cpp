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

#include "recordingannotation.h"
#include "ui_recordingannotation.h"

RecordingAnnotation::RecordingAnnotation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordingAnnotation)
{
    ui->setupUi(this);
}

void RecordingAnnotation::closeEvent()
{
    emit channelIDsUpdate(annotation);
}

RecordingAnnotation::~RecordingAnnotation()
{
    delete ui;
}

void RecordingAnnotation::on_RecordingAnnotation_clicked()
{
    QPushButton* buttonClicked = qobject_cast<QPushButton*>(sender());
    annotation = buttonClicked->text();
    emit channelIDsUpdate(annotation);
    this->close();
}
