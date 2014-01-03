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

#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <QDebug>
#include <opus/opus.h>
#include "codec2/codec2.h"

class AudioEncoder
{
public:
    AudioEncoder();
    ~AudioEncoder();
    unsigned char *encode_opus(short *audiobuffer, int audiobuffersize, int &encoded_size);
    short *decode_opus(unsigned char *audiobuffer, int data_length, int &samples);
    unsigned char* encode_codec2(short *audiobuffer, int audiobuffersize);
    short* decode_codec2(unsigned char *audiobuffer, int audiobuffersize);

private:
    OpusEncoder *_enc;
    OpusDecoder *_dec;
    struct CODEC2 *_codec2;
};

#endif // AUDIOENCODER_H
