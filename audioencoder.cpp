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

#include "audioencoder.h"

AudioEncoder::AudioEncoder()
{
    int error, error1;
    _enc = opus_encoder_create(8000,1,OPUS_APPLICATION_VOIP,&error);
    if(error != OPUS_OK)
    {
        qDebug() << "audio encoder creation failed";
    }
    _dec = opus_decoder_create(8000,1,&error1);
    if(error1 != OPUS_OK)
    {
        qDebug() << "audio decoder creation failed";
    }
    _codec2 = codec2_create(CODEC2_MODE_1200);
}

AudioEncoder::~AudioEncoder()
{
    opus_encoder_destroy(_enc);
    opus_decoder_destroy(_dec);
    codec2_destroy(_codec2);
}

unsigned char* AudioEncoder::encode_opus(short *audiobuffer, int audiobuffersize, int &encoded_size)
{
    int opus_bandwidth;
    opus_encoder_ctl(_enc, OPUS_SET_BITRATE(4000));
    opus_encoder_ctl(_enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(_enc, OPUS_SET_COMPLEXITY(5));
    opus_encoder_ctl(_enc, OPUS_SET_DTX(1));
    opus_encoder_ctl(_enc, OPUS_GET_BANDWIDTH(&opus_bandwidth));
    opus_encoder_ctl(_enc, OPUS_SET_INBAND_FEC(1));

    unsigned char *encoded_audio = new unsigned char[opus_bandwidth];
    encoded_size = opus_encode(_enc, audiobuffer, audiobuffersize/sizeof(short), encoded_audio, opus_bandwidth);
    return encoded_audio;

}

short* AudioEncoder::decode_opus(unsigned char *audiobuffer, int audiobuffersize, int &samples)
{
    int nr_of_frames = opus_packet_get_nb_frames(audiobuffer,audiobuffersize);
    int fs = 960 * nr_of_frames;
    short *pcm = new short[fs*sizeof(short)];

    samples = opus_decode(_dec,audiobuffer,audiobuffersize, pcm, fs, 0);
    return pcm;
}

unsigned char* AudioEncoder::encode_codec2(short *audiobuffer, int audiobuffersize, int &length)
{
    int bits = codec2_bits_per_frame(_codec2);
    int bytes = (bits + 7) / 8;
    unsigned char *encoded = new unsigned char[bytes];
    codec2_encode(_codec2, encoded, audiobuffer);
    length = bytes;
    return encoded;
}

short* AudioEncoder::decode_codec2(unsigned char *audiobuffer, int audiobuffersize, int &samples)
{
    int samp = codec2_samples_per_frame(_codec2);
    short* decoded = new short[samp];
    codec2_decode(_codec2, decoded, audiobuffer);
    samples = samp;
    return decoded;
}
