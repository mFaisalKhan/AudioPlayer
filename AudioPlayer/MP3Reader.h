#ifndef MP3READER_H
#define MP3READER_H

#include <QString>
#include <fstream>
#include "IFFileFormat.h"

const unsigned int SYNC_MASK =              0xFFE00000;     // 0000E0FF
const unsigned int MPEG_VER_MASK =          0x00180000;     // 00001800
const unsigned int LAYER_MASK =             0x00060000;     // 00000600
const unsigned int PROTECTION_BIT_MASK =    0x00010000;     // 00000100
const unsigned int BIT_RATE_MASK =          0x0000F000;     // 0000F000
const unsigned int SAMPLE_RATE_MASK =       0x00000C00;     // 000C0000
const unsigned int PADDING_BIT_MASK =       0x00000200;
const unsigned int PRIV_BIT_MASK =          0x00000100;
const unsigned int CHANNEL_MODE_MASK =      0x000000C0;
const unsigned int EXTENSION_MASK =         0x00000030;
const unsigned int COPY_RIGHT_MASK =        0x00000008;
const unsigned int ORIGINAL_MASK =          0x00000004;
const unsigned int EMPHASIS_MASK =          0x00000003;


unsigned long const BitRateTable[5][15] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,  /* Layer I   */
       256000, 288000, 320000, 352000, 384000, 416000, 448000 },
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer II  */
       128000, 160000, 192000, 224000, 256000, 320000, 384000 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,  /* Layer III */
       112000, 128000, 160000, 192000, 224000, 256000, 320000 },

  /* MPEG-2 LSF */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer I   */
       128000, 144000, 160000, 176000, 192000, 224000, 256000 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,  /* Layers    */
        64000,  80000,  96000, 112000, 128000, 144000, 160000 } /* II & III  */
};

const unsigned int SampleRateMap[4][4]={{44100, 48000, 32000, 0xffff},  // MPEG 1
                                        {22050, 24000, 16000, 0xffff},  // MPEG 2
                                        {11025, 12000,  8000, 0xffff},  // MPEG 2.5
                                        {0xff, 0xff,0xff, 0xff}};       // reserved

const unsigned int ChannelMode[4] = {2,2,3,1};

#pragma pack(1)
struct MP3_Frame
{
    unsigned int sync;              // 11 bits, MASK FFE00000    0000E0FF
    unsigned char MpegVersion;    // 2 bits,  MASK 00180000    00001800
    unsigned char LayerDesc;      // 2 bits,  MASK 00060000    00000600
    unsigned char ProtctionBit;   // 1 bit.   MASK 00010000    00000100
    unsigned char BitRate;        // 4 bits,  MASK 0000F000    0000F000
    unsigned char sampleRate;     // 2 bits,  MASK 00000C00    000C0000
    unsigned char PaddingBit;     // 1 bit,   MASK 00000200
    unsigned char PrivateBit;     // 1 bit,   MASK 00000100
    unsigned char ChannelMode;    // 2 bits,  MASK 000000C0
    unsigned char Extension;      // 2 bits
    unsigned char CopyRight;      // 1 bit
    unsigned char Original;       // 1 bit
    unsigned char Emphasis;       // 2 bits
};

struct ID3V2_HEADER
{
 char Id[3];
 unsigned char MajVer;
 unsigned char MinVer;
 unsigned char Flags;
 unsigned int Size;
};

struct ID3V2_FRAME_HEADER
{
 char Frameid[4];
 unsigned int Size;
 unsigned char Flag1;
 unsigned char Flag2;
};

typedef struct MP3_Frame FrameMP3;
typedef struct ID3V2_HEADER ID3_Header;
typedef struct ID3V2_FRAME_HEADER ID3_FrameHeader;

class MP3Reader:public IFFileFormat
{
public:
    MP3Reader();
    QString GetAudioFileProperties(QString filePath, AudioFileProperties *Format, std::ifstream **fptr );
    QString GetAlbumName(){return AlbumName;}
    QString GetTitle(){return Title;}

private:
    std::ifstream *File;
    bool FindMP3Frame();
    void SkipID3();
    int GetID3FooterLength();
    unsigned int ReadBits(unsigned int data, int pos, int count);
    QString AlbumName;
    QString Title;
    int ID3FooterLength;
    int ID3HeaderLength;

};

#endif // MP3READER_H
