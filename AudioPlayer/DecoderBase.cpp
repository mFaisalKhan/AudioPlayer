#include "AudioFormats/DecoderBase.h"
#include <QDebug>

DecoderBase::DecoderBase()
    :QThread()
    , InStream(NULL)
    , fdOutput(0)
    , IsRunning(false)
{

}

////////////////////////////////////////////////////////////////////////////////////////////
DecoderBase::~DecoderBase()
{

}

// Provide Default functionality for decoding
////////////////////////////////////////////////////////////////////////////////////////////
QString DecoderBase::SetStreams(std::ifstream *inStream, int fdInEnd)
{
 InStream = inStream;
 fdOutput = fdInEnd;
 IsRunning = true;

 return QString("NO Error");
}

////////////////////////////////////////////////////////////////////////////////////////////
void DecoderBase::run()
{
    unsigned char buffer[100];

    int read = 0;
    while(!InStream->eof() && IsRunning==true)
    {
        InStream->read((char *)buffer, sizeof(buffer));
        read = InStream->gcount();
        if(read>0)
            write(fdOutput, (char *) buffer, sizeof(buffer));
    }

    emit SignalDecodingFinished();
    qDebug() <<" Decoding finished";
    exit();
}

void DecoderBase::Stop()
{
    IsRunning = false;
}
