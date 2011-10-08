/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EVENTBUILDERPLUGIN_H
#define EVENTBUILDERPLUGIN_H

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <iostream>
#include <algorithm>
#include <vector>

#include "baseplugin.h"

class BasePlugin;

class EventBuilderPlugin : public BasePlugin
{
    Q_OBJECT

protected:
    virtual void createSettings(QGridLayout*);
    QString makeFileName();

    QLabel* totalBytesWrittenLabel;
    QLabel* currentFileNameLabel;
    QLabel* currentBytesWrittenLabel;
    QLabel* bytesFreeOnDiskLabel;

    QVector<uint32_t> outData;
    Attributes attribs_;

    QFile outFile;
    QDir outDir;
    QTime lastWriteTime;

    QString filePrefix;

    uint64_t total_bytes_written;
    uint32_t current_bytes_written;
    uint32_t current_file_number;

    bool open_new_file;

public:
    EventBuilderPlugin(int _id, QString _name, const Attributes &_attrs);

    static AbstractPlugin *create (int id, const QString &name, const Attributes &attrs) {
        return new EventBuilderPlugin (id, name, attrs);
    }

    AttributeMap getAttributeMap () const;
    Attributes getAttributes () const;
    static AttributeMap getEventBuilderAttributeMap ();

    virtual void userProcess();
    virtual void applySettings(QSettings*);
    virtual void saveSettings(QSettings*);

public slots:
    void updateRunName();

};

#endif // EVENTBUILDERPLUGIN_H
