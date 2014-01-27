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

#ifndef MUMBLECLIENT_H
#define MUMBLECLIENT_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QtEndian>
#include <QCoreApplication>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include "ext/Mumble.pb.h"
#include "ext/CryptState.h"
#include "ext/PacketDataStream.h"
#include "ext/utils.h"
#include "sslclient.h"
#include "audiointerface.h"
#include "audioencoder.h"
#include "opus/opus.h"
#include "config_defines.h"
#include "settings.h"
#include "station.h"

typedef QVector<Station> StationList;
class MumbleClient : public QObject
{
    Q_OBJECT
public:
    explicit MumbleClient(Settings *settings, QObject *parent = 0);
    ~MumbleClient();
    void connectToServer(QString address, unsigned port);
    void disconnectFromServer();

    QString getChannelName();
    int getChannelId();
    QString createChannel(QString channel_name="");
    void joinChannel(int id);
    int callStation(QString radio_id);
    void disconnectFromCall();
    int disconnectStation(QString radio_id);
    void disconnectAllStations();
    void setMute(bool mute);
signals:
    void channelName(QString name);
    void pcmAudio(short *pcm, short size);
    void onlineStations(StationList);
    void newStation(Station*);
    void leftStation(Station*);
    void channelReady(int chan_number);
    
public slots:
    void sendVersion();
    void authenticate();
    void pingServer();
    void processProtoMessage(QByteArray data);
    void processUDPData(QByteArray data);
    void sendUDPPing();
    void processAudio(short *audiobuffer, short audiobuffersize);

private:
    void sendUDPMessage(quint8 *message, int size);
    void sendMessage(quint8 *message, quint16 type, int size);
    void setupEncryption(quint8 *message, quint64 size);

    void processServerSync(quint8 *message, quint64 size);
    void processChannelState(quint8 *message, quint64 size);
    void processUserState(quint8 *message, quint64 size);
    void processUserRemove(quint8 *message, quint64 size);
    void createVoicePacket(unsigned char *encoded_audio, int packet_size);
    void processIncomingAudioPacket(quint8 *data, quint64 size, quint8 type);
    void decodeAudio(unsigned char *audiobuffer, short audiobuffersize, quint8 type);

    SSLClient *_telnet;
    CryptState *_crypt_state;
    std::string _key;
    std::string _client_nonce;
    std::string _server_nonce;
    QString _temp_channel_name;
    bool _encryption_set;
    bool _synchronized;
    bool _authenticated;
    int _session_id;
    int _max_bandwidth;
    int _channel_id;
    OpusEncoder *_opus_encoder;
    OpusDecoder *_opus_decoder;
    AudioEncoder *_codec;
    Settings *_settings;
    quint64 _sequence_number;
    QVector<Station*> _stations;

};


#endif // MUMBLECLIENT_H
