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

#ifndef JSONSTORAGE_H
#define JSONSTORAGE_H

#include <QtCore>
#include <QString>

class JSONStorage
{
public:
    JSONStorage(QString path, QString name);

    void addJSON(QJsonObject newObject);
    void addObjectTimestamp(QString key, QString value);
    void saveJSON();

private:
    QString filePath;
    QString fileName;

    QJsonDocument jsonDocument;
    QJsonObject baseObject;
    QJsonArray jsonArray;
};



#endif // JSONSTORAGE_H
