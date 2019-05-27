#ifndef IFFILEFORMAT_H
#define IFFILEFORMAT_H

#include <QString>

enum COMPRESSIONS_FORMATS
{
    COMPRESSION_UNKNOWN = 0,
    COMPRESSION_U8 = 1,
    COMPRESSION_S16_LE = 2,
    COMPRESSION_A_LAW = 6,
    COMPRESSION_MU_LAW = 7,
    COMPRESSION_IMA_ADPCM1 = 17,
    COMPRESSION_IMA_ADPCM2 = 20,
    COMPRESSION_GSM = 49,
    COMPRESSION_IMA_ADPCM3 = 64,
    COMPRESSION_MPEG = 80,
    COMPRESSION_MPEG3 = 85,

    COMPRESSION_FILE_ERROR
};

struct AudioFileProperties
{
    COMPRESSIONS_FORMATS AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;        // KHz
    unsigned int BitRate;           // Resolution
    float AudioLength;              // length in seconds
    unsigned int BytesPerSample;
};

class IFFileFormat
{

public:
    IFFileFormat(){};
    ~IFFileFormat(){};
    virtual QString GetAudioFileProperties(QString filePath, AudioFileProperties *waveFormat, std::ifstream **fptr )=0;
    virtual QString GetAlbumName() = 0;
    virtual QString GetTitle() = 0;
};

#endif // IFFILEFORMAT_H
