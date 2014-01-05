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

#include "mumbleclient.h"




MumbleClient::MumbleClient(QObject *parent) :
    QObject(parent)
{
    _telnet = new SSLClient;
    _crypt_state = new CryptState;
    _codec = new AudioEncoder;
    _encryption_set = false;
    _authenticated = false;
    _synchronized = false;
    _session_id = -1;
    _max_bandwidth = -1;
    _channel_id = -1;
    _temp_channel_name = "";
    int opus_error;
    _opus_encoder = opus_encoder_create(8000,1,OPUS_APPLICATION_VOIP,&opus_error);
    _opus_decoder = opus_decoder_create(8000,1,&opus_error);
    _sequence_number = 0;
}

MumbleClient::~MumbleClient()
{
    delete _telnet;
    delete _crypt_state;
    delete _codec;
    opus_encoder_destroy(_opus_encoder);
    opus_decoder_destroy(_opus_decoder);
}

void MumbleClient::connectToServer(QString address, unsigned port)
{
    _telnet->connectHost(address,port);
    QObject::connect(_telnet,SIGNAL(connectedToHost()),this,SLOT(sendVersion()));
    QObject::connect(_telnet,SIGNAL(haveMessage(QByteArray)),this,SLOT(processProtoMessage(QByteArray)));
    QObject::connect(_telnet,SIGNAL(haveUDPData(QByteArray)),this,SLOT(processUDPData(QByteArray)));
}

void MumbleClient::disconnectFromServer()
{
    if(_authenticated)
    {
        _telnet->disconnectHost();
    }
}

void MumbleClient::sendVersion()
{
    qDebug() << "sending version";
    MumbleProto::Version v;
    v.set_version(PROTOCOL_VERSION);
    v.set_release("QRadioLink 1");
    v.set_os("Linux");
    v.set_os_version("3.2");
    int size = v.ByteSize();
    quint8 data[size];
    v.SerializeToArray(data,size);
    quint16 type = 0;
    this->sendMessage(data,type,size);
    authenticate();
}

void MumbleClient::authenticate()
{
    qDebug() << "authenticating...";
    MumbleProto::Authenticate auth;
    int rand_len = 4;
    char rand[4];
    genRandomStr(rand,rand_len);
    QString username = "test_";
    username += QString::fromLocal8Bit(rand);
    auth.set_username(username.toStdString().c_str());
    auth.set_password("");
    auth.set_opus(true);
    int size = auth.ByteSize();
    quint8 data[size];
    auth.SerializeToArray(data,size);
    quint16 type = 2;
    this->sendMessage(data,type,size);
    _authenticated = true;
}

void MumbleClient::pingServer()
{

    if(!_synchronized)
        return;
    struct timeval now;

    gettimeofday(&now, NULL);
    quint64 ts=now.tv_sec*1000000+now.tv_usec;
    MumbleProto::Ping ping;
    ping.set_timestamp(ts);
    int size = ping.ByteSize();
    quint8 data[size];
    ping.SerializeToArray(data,size);
    this->sendMessage(data,3,size);
#ifndef NO_CRYPT
    sendUDPPing();
#endif
}


void MumbleClient::processProtoMessage(QByteArray data)
{

    int total_size = data.size();
    quint8 bin_data[total_size];
    unsigned char *temp = reinterpret_cast<unsigned char*>(data.data());
    memcpy(bin_data,temp,total_size);
    int type, len;
    getPreamble(bin_data,&type,&len);
    int message_size = total_size-6;
    quint8 message[message_size];
    memcpy(message,bin_data+6,message_size);

    switch(type)
    {
    case 15:
        setupEncryption(message,message_size);
        break;
    case 5: // ServerSync
        processServerSync(message,message_size);
        break;
    case 3: // ping
        //qDebug() << "pong";
        break;
    case 7: // ChannelState
        processChannelState(message, message_size);
        break;
    case 9: // UserState
        processUserState(message, message_size);
        break;
    case 1: // UDPTunnel
        processIncomingAudioPacket(message, message_size);
    default:
        break;
    }
}

void MumbleClient::setupEncryption(quint8 *message, quint64 size)
{
    MumbleProto::CryptSetup crypt;
    crypt.ParseFromArray(message,size);
    _key = crypt.key();
    _client_nonce = crypt.client_nonce();
    _server_nonce = crypt.server_nonce();
    _crypt_state->setKey(reinterpret_cast<const unsigned char*>(_key.c_str()),
                         reinterpret_cast<const unsigned char*>(_client_nonce.c_str()),
                         reinterpret_cast<const unsigned char*>(_server_nonce.c_str()));
    _encryption_set = true;
    qDebug() << "Encryption setup ok ";
    pingServer();
}

void MumbleClient::processServerSync(quint8 *message, quint64 size)
{
    MumbleProto::ServerSync sync;
    sync.ParseFromArray(message,size);
    _session_id = sync.session();
    _max_bandwidth = sync.max_bandwidth();
    std::string welcome = sync.welcome_text();
    _synchronized = true;
    qDebug() << QString::fromStdString(welcome) << " max bandwidth: " << _max_bandwidth;
#ifndef NO_CRYPT
    createChannel();
#endif
}

void MumbleClient::processChannelState(quint8 *message, quint64 size)
{

    MumbleProto::ChannelState ch;
    ch.ParseFromArray(message,size);

    qDebug() << " Channel id: " << ch.channel_id() << QString::fromStdString(ch.name());
}

void MumbleClient::processUserState(quint8 *message, quint64 size)
{

    MumbleProto::UserState us;
    us.ParseFromArray(message,size);
    if(us.session() == _session_id)
    {
        _channel_id = us.channel_id();
        qDebug() << " Joined channel: " << _channel_id;
    }

}

void MumbleClient::joinChannel(int id)
{
    MumbleProto::UserState us;
    us.set_channel_id(id);
    int size = us.ByteSize();
    quint8 data[size];
    us.SerializeToArray(data,size);
    this->sendMessage(data,9,size);

}


QString MumbleClient::getChannelName()
{
    return _temp_channel_name;
}

int MumbleClient::getChannelId()
{
    return _channel_id;
}

QString MumbleClient::createChannel()
{
    int rand_len = 8;
    char rand[8];
    genRandomStr(rand,rand_len);
    QString name = QString::fromLocal8Bit(rand);
    _temp_channel_name = name;
    MumbleProto::ChannelState channel;
    channel.set_parent(0);
    channel.set_name(name.toStdString());
    channel.set_temporary(true);
    int size = channel.ByteSize();
    quint8 data[size];
    channel.SerializeToArray(data,size);
    quint16 type = 7;
    this->sendMessage(data,type,size);
    emit channelName(_temp_channel_name);
    return name;
}

void MumbleClient::processAudio(short *audiobuffer, short audiobuffersize)
{

    if(!_synchronized)
        return;
    /*
    int opus_bandwidth;
    opus_encoder_ctl(_opus_encoder, OPUS_SET_BITRATE(4000));
    opus_encoder_ctl(_opus_encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(_opus_encoder, OPUS_SET_COMPLEXITY(5));
    opus_encoder_ctl(_opus_encoder, OPUS_SET_DTX(1));
    opus_encoder_ctl(_opus_encoder, OPUS_GET_BANDWIDTH(&opus_bandwidth));
    opus_encoder_ctl(_opus_encoder, OPUS_SET_INBAND_FEC(1));

    unsigned char encoded_audio[opus_bandwidth];
    int packet_size = opus_encode(_opus_encoder, audiobuffer, audiobuffersize/sizeof(short), encoded_audio, opus_bandwidth);
    */
    int packet_size = 0;
#ifdef USE_CODEC2
    unsigned char *encoded_audio = _codec->encode_codec2(audiobuffer, audiobuffersize, packet_size);
#else
    unsigned char *encoded_audio = _codec->encode_opus(audiobuffer, audiobuffersize, packet_size);
#endif

    createVoicePacket(encoded_audio, packet_size);
    delete[] encoded_audio;
}

void MumbleClient::createVoicePacket(unsigned char *encoded_audio, int packet_size)
{
    int type = 0;
    type |= (4 << 5);

    int data_size = 1024;
    char data[data_size];
    data[0] = static_cast<unsigned char>(type);
    PacketDataStream pds(data + 1, data_size-1);
    int nr_of_frames = opus_packet_get_nb_frames(encoded_audio,packet_size);

    // sequence?

    pds << _sequence_number;
    int real_packet_size = packet_size;
    _sequence_number +=nr_of_frames;
    //packet_size |= 1 << 13;
    pds << packet_size;

    char *audio_packet = reinterpret_cast<char*>(encoded_audio);

    pds.append(audio_packet,real_packet_size);

    unsigned char *bin_data = reinterpret_cast<unsigned char*>(data);
    if(MUMBLE_TCP_AUDIO) // TCP tunnel
    {
        this->sendMessage(bin_data,1,pds.size()+1);
    }
    else // Use UDP
    {
        this->sendUDPMessage(bin_data,pds.size()+1);
    }
}

void MumbleClient::processIncomingAudioPacket(quint8 *data, quint64 size)
{
    PacketDataStream pds(data+1, size-1);
    quint64 seq_number;
    quint64 session;
    quint8 audio_head;
    pds >> session;
    pds >> seq_number;
    pds >> audio_head;
    audio_head &= 0x1fff;
    int audio_size = pds.left();
    QByteArray qba = pds.dataBlock(pds.left());
    unsigned char *encoded_audio = reinterpret_cast<unsigned char*>(qba.data());

    decodeAudio(encoded_audio,audio_size);

}

void MumbleClient::processUDPData(QByteArray data)
{
    if(!_encryption_set)
        return;

    unsigned char *encrypted = reinterpret_cast<unsigned char*>(data.data());
#ifndef NO_CRYPT
    unsigned char decrypted[1024];
    _crypt_state->decrypt(encrypted, decrypted, data.size());
    int decrypted_length = data.size() - 4;

    processIncomingAudioPacket(decrypted, decrypted_length);
#else
    processIncomingAudioPacket(encrypted, data.size());
#endif
}

void MumbleClient::decodeAudio(unsigned char *audiobuffer, short audiobuffersize)
{
    /*
    int nr_of_frames = opus_packet_get_nb_frames(audiobuffer,audiobuffersize);
    int fs = 960 * nr_of_frames;
    short pcm[fs*sizeof(short)];

    int samples = opus_decode(_opus_decoder,audiobuffer,audiobuffersize, pcm, fs, 0);
    */
    int samples =0;
#ifdef USE_CODEC2
    short *pcm = _codec->decode_codec2(audiobuffer,audiobuffersize, samples);
#else
    short *pcm = _codec->decode_opus(audiobuffer,audiobuffersize, samples);
#endif

    emit pcmAudio(pcm, samples);
}



void MumbleClient::sendMessage(quint8 *message, quint16 type, int size)
{

    int new_size = size+6;

    quint8 bin_data[new_size];
    //qToBigEndian<quint16>(type, & bin_data[0]);
    //qToBigEndian<quint32>(len, & bin_data[2]);

    memcpy(bin_data+6,message,size);
    addPreamble(bin_data,type,size);
    _telnet->sendBin(bin_data,new_size);
}

void MumbleClient::sendUDPMessage(quint8 *message, int size)
{


#ifndef NO_CRYPT
    if(_encryption_set)
    {
        int new_size = size+4;
        quint8 bin_data[new_size];
        _crypt_state->encrypt(message,bin_data,size);
        _telnet->sendUDP(bin_data,new_size);
    }
#else
     _telnet->sendUDP(message,size);
#endif
}

void MumbleClient::sendUDPPing()
{
    if((!_synchronized) || (!_encryption_set))
        return;
    quint8 head = 32;
    struct timeval now;

    gettimeofday(&now, NULL);
    quint64 ts=now.tv_sec*1000000+now.tv_usec;
    quint8 message[sizeof(quint8) + sizeof(quint64)];
    memcpy(message,&head,sizeof(quint8));
    memcpy(message+sizeof(quint8),&ts,sizeof(quint64));
    quint8 encrypted[sizeof(quint8) + sizeof(quint64)+4];
    _crypt_state->encrypt(message,encrypted,sizeof(quint8) + sizeof(quint64));

    _telnet->sendUDP(message,sizeof(quint8) + sizeof(quint64)+4);
}


