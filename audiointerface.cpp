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

#include "audiointerface.h"

AudioInterface::AudioInterface(QObject *parent, unsigned sample_rate, unsigned channels, int normal) :
    QObject(parent)
{
    _s_rec = NULL;
    _s_play = NULL;
    _s_short_play = NULL;
    _s_short_rec = NULL;
    _error=0;
    int rand_len = 4;
    char rand[4];
    genRandomStr(rand,rand_len);
    static QString instance_name = QString::fromLocal8Bit(rand);
    QString dtmf_rec = "qradiolink_dtmf_" + instance_name;
    QString q_play = "qradiolink_" + instance_name;
    QString audio_rec = "qradiolink_audio_" + instance_name;

    int dd;
    //pa_usec_to_bytes(20000, &dd);
    pa_buffer_attr attr;
    attr.fragsize = 1280;
    attr.maxlength = -1;
    attr.minreq = -1;
    attr.prebuf = -1;
    attr.tlength = 1280;

    if(!normal)
    {
        pa_sample_spec ss;
        //ss.format = PA_SAMPLE_S16LE;
        ss.format = PA_SAMPLE_FLOAT32LE;
        ss.rate = sample_rate;
        ss.channels = channels;

        if (!(_s_rec = pa_simple_new(NULL, dtmf_rec.toStdString().c_str(), PA_STREAM_RECORD, NULL, "record", &ss, NULL, &attr, &_error)))
        {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(_error));
        }
        if (!(_s_play = pa_simple_new(NULL, q_play.toStdString().c_str(), PA_STREAM_PLAYBACK, NULL, "play", &ss, NULL, &attr, &_error)))
        {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(_error));
        }
    }
    else
    {
        pa_sample_spec ss_short;
        //ss.format = PA_SAMPLE_S16LE;
        ss_short.format = PA_SAMPLE_S16LE;
        ss_short.rate = sample_rate;
        ss_short.channels = channels;

        if (!(_s_short_play = pa_simple_new(NULL, q_play.toStdString().c_str(), PA_STREAM_PLAYBACK, NULL, "play", &ss_short, NULL, &attr, &_error)))
        {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(_error));
        }
        if (!(_s_short_rec = pa_simple_new(NULL, audio_rec.toStdString().c_str(), PA_STREAM_RECORD, NULL, "record", &ss_short, NULL, &attr, &_error)))
        {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(_error));
        }
    }
}

AudioInterface::~AudioInterface()
{
    if (_s_rec)
      pa_simple_free(_s_rec);
    if (_s_play)
      pa_simple_free(_s_play);
    if (_s_short_play)
      pa_simple_free(_s_short_play);
    if (_s_short_rec)
      pa_simple_free(_s_short_rec);
}

int AudioInterface::read(float *buf, short bufsize)
{
    if(!_s_rec)
        return 1;
    if (pa_simple_read(_s_rec, buf, bufsize, &_error) < 0)
    {
        fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(_error));
        return 1;
    }
    return 0;
}

int AudioInterface::write(float *buf, short bufsize)
{
    if(!_s_play)
        return 1;
    if(pa_simple_write(_s_play, buf, bufsize, &_error) < 0)
    {
        fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(_error));
        return 1;
    }
    return 0;
}

int AudioInterface::write_short(short *buf, short bufsize)
{
    if(!_s_short_play)
        return 1;
    if(pa_simple_write(_s_short_play, buf, bufsize, &_error) < 0)
    {
        fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(_error));
        return 1;
    }
    return 0;
}

int AudioInterface::read_short(short *buf, short bufsize)
{
    if(!_s_short_rec)
        return 1;
    if(pa_simple_read(_s_short_rec, buf, bufsize, &_error) < 0)
    {
        fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(_error));
        return 1;
    }
    return 0;
}
