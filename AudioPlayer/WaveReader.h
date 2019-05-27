#ifndef WAVEREADER_H
#define WAVEREADER_H

#include <QString>
#include <fstream>
#include "IFFileFormat.h"

const unsigned int ChunkID = 0x46464952; // 0x52494646;     // "RIFF"
const unsigned int SubChunkID1 = 0x20746d66; // 0x666d7420; // "fmt "
const unsigned int DataChunkID = 0x61746164; // 0x64617461; // "data"
const unsigned int FactChunkID = 0x74636166; // 0x66616374; // "fact"
const unsigned int FormatChunkID = 0x45564157; // 0x66616374; // "WAVE"

const unsigned int WAVE_FORMAT_PCM = 0x0001;            //PCM
const unsigned int WAVE_FORMAT_IEEE_FLOAT = 0x0003 ;	//	IEEE float
const unsigned int WAVE_FORMAT_ALAW = 0x0006;           //8-bit ITU-T G.711 A-law
const unsigned int WAVE_FORMAT_MULAW = 0x0007;      	//8-bit ITU-T G.711 µ-law
const unsigned int WAVE_FORMAT_EXTENSIBLE = 0xFFFE; 	//Determined by SubFormat

/*
The canonical WAVE format starts with the  RIFF header:

0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk
                               following this number.  This is the size of the
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this
                               number.
44        *   Data             The actual sound data.

  */
#pragma pack(push)

#pragma pack(1)
struct WaveFileCommonHeader
{
    unsigned int ChunkId;
    unsigned int ChunkSize;
    unsigned int Format;
    unsigned int SubChunk1ID;
    unsigned int SubChunk1Size;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
};

struct WaveFileDataChunk
{
    unsigned int DataChunkID;
    unsigned int DataChunkSize;
};

struct WaveFileExtensionChunk
{
    unsigned short      wValidBitsPerSample;
    unsigned int        dwChannelMask;
    unsigned char       SubFormat[16];
};

struct WaveFileFactChunk
{
    unsigned int      FactChunkID;
    unsigned int      FactChunksize;
    unsigned int      dwSampleLength;
};


#pragma pack(pop)



class WaveReader: public IFFileFormat
{
public:
    WaveReader();
    ~WaveReader();
    QString GetAudioFileProperties(QString filePath, AudioFileProperties *waveFormat, std::ifstream **fptr );
    QString GetAlbumName(){ return "Unknown";}
    QString GetTitle(){return "Unknown";}

private:
    std::ifstream *File;

};

#endif // WAVEREADER_H
