// Written by Adrian Musceac YO8RZZ at gmail dot com, started October 2013.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef TELNETSERVER_H
#define TELNETSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QAbstractSocket>
#include <QVector>
#include <QHostAddress>
#include <QDebug>
#include <QCoreApplication>
#include "databaseapi.h"
#include "config_defines.h"
#include "ext/dec.h"
#include "ext/QRadioLink.pb.h"
#include "settings.h"

class TelnetServer : public QObject
{
    Q_OBJECT
public:
    explicit TelnetServer(Settings *settings, DatabaseApi *db, QObject *parent = 0);
    ~TelnetServer();
    void stop();

signals:
    void finished();
    void joinConference( int number, int id, int server_id);
    void leaveConference( int number, int id, int server_id);
public slots:


private slots:
    void getConnection();
    void connectionFailed(QAbstractSocket::SocketError error);
    void connectionSuccess();
    void processData();

private:
    QTcpServer *_server;
    QTcpSocket _socket;
    DatabaseApi *_db;
    int _status;
    bool _stop;
    QHostAddress _hostname;
    unsigned _listen_port;
    QVector<QTcpSocket*> _unconnected_clients;
    QVector<QTcpSocket*> _connected_clients;
    QByteArray processCommand(QByteArray data);

};

#endif // TELNETSERVER_H
