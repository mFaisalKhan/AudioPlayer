#include "WaveReader.h"
#include <QDebug>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////
WaveReader::WaveReader()
    : IFFileFormat()
    , File(NULL)
{

}

////////////////////////////////////////////////////////////////////////////////////////////
WaveReader::~WaveReader()
{
    if(File)
    {
        File->close();
        File = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString WaveReader::GetAudioFileProperties(QString filePath,AudioFileProperties *waveFormat, std::ifstream **fptr )
{
    QString err= "NO ERROR";
    File = new ifstream(filePath.toStdString().c_str(), ios::in|ios::binary);
    WaveFileCommonHeader header;

    if( File->is_open())
    {
        File->read((char *)&header, sizeof(WaveFileCommonHeader));

        if( ChunkID == header.ChunkId &&
                SubChunkID1 == header.SubChunk1ID)
        {
            if(header.AudioFormat != WAVE_FORMAT_PCM)
            {
                unsigned short cbSize = 0;
                int size =  sizeof(cbSize);

                File->read((char *)&cbSize,size);

                if(header.BitsPerSample == 0)
                    header.BitsPerSample = 8 * header.NumChannels;
                if(cbSize > 0)
                {
                    WaveFileExtensionChunk extChunk;
                    File->read((char *)&extChunk, sizeof(WaveFileExtensionChunk));
                }

            }

            unsigned int dataOrFactChunk;

            File->read((char *)&dataOrFactChunk, sizeof(dataOrFactChunk));
            File->seekg(-sizeof(dataOrFactChunk), ios_base::cur);

            if(dataOrFactChunk == FactChunkID)
            {
                WaveFileFactChunk factChunk;

                File->read((char *)&factChunk, sizeof(factChunk));
            }

            WaveFileDataChunk dataChunk;

            File->read((char *)&dataChunk, sizeof(WaveFileDataChunk));

            waveFormat->AudioFormat = (COMPRESSIONS_FORMATS) header.AudioFormat;
            waveFormat->AudioLength = (float) dataChunk.DataChunkSize / header.ByteRate;
            waveFormat->BitRate = header.BitsPerSample * 1000;
            waveFormat->NumChannels = header.NumChannels;
            waveFormat->SampleRate = header.SampleRate;
            waveFormat->BytesPerSample = header.BitsPerSample /8;

            *fptr = File;
        }
        else
        {
            err = "Invalid file format";
        }
    }
    else
    {
        err = "FILE Open error";
    }

    return err;
}
