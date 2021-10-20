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

#include "jsonstorage.h"

JSONStorage::JSONStorage(QString path, QString name)
{
    filePath = path;
    fileName = name;

    QFileInfo fileInfo(filePath + fileName);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        QFile file(filePath + fileName);
        file.open(QFile::ReadOnly | QFile::Text);
        QJsonParseError JsonParseError;
        jsonDocument = QJsonDocument::fromJson(file.readAll(), &JsonParseError);
        file.close();

        jsonArray = jsonDocument.array();
    }
}

void JSONStorage::clearContent()
{
    while (jsonArray.count() > 0)
    {
        jsonArray.removeFirst();
    }
}

QJsonArray JSONStorage::getJsonArray()
{
    return jsonArray;
}

void JSONStorage::addJSON(QJsonObject newObject)
{
    jsonArray.append(newObject);
}

void JSONStorage::addObjectTimestamp(QString key, QString value)
{
    QJsonObject newObject;
    newObject[key] = QJsonValue(value);

    QDateTime currentTime;
    newObject["Time"] = QJsonValue(currentTime.currentDateTime().toString("yyyy/MM/dd HH:mm:ss"));
    jsonArray.append(newObject);
}

void JSONStorage::saveJSON()
{
    jsonDocument.setArray(jsonArray);

    QFile file(filePath + fileName);
    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    file.write(jsonDocument.toJson());
    file.close();
}
