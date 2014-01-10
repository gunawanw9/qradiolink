#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings
{
public:
    Settings();
    quint32 _id;
    quint8 _use_mumble;
    quint8 _mumble_tcp;
    quint8 _use_codec2;
    float _audio_treshhold;
    float _voice_activation;
    quint16 _voice_activation_timeout;
    quint16 _voice_server_port;
    quint16 _local_udp_port;
    quint16 _control_port;
    quint16 _opus_bitrate;
    quint16 _codec2_bitrate;
    quint8 _enable_vox;
    quint8 _enable_agc;
    quint16 _ident_time;

};

#endif // SETTINGS_H
