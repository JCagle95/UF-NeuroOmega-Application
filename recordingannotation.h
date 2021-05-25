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

#ifndef RECORDINGANNOTATION_H
#define RECORDINGANNOTATION_H

#include <QDialog>

namespace Ui {
class RecordingAnnotation;
}

class RecordingAnnotation : public QDialog
{
    Q_OBJECT

public:
    explicit RecordingAnnotation(QWidget *parent = 0);
    void closeEvent();
    ~RecordingAnnotation();

private slots:
    void on_RecordingAnnotation_clicked();

signals:
    void channelIDsUpdate(QString annotation);

private:
    Ui::RecordingAnnotation *ui;

    QString annotation = "No Annotation";
};

#endif // RECORDINGANNOTATION_H
