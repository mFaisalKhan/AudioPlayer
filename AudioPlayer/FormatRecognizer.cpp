#include "FormatRecognizer.h"
#include "IFFileFormat.h"

#include <iostream>
#include <fstream>

using namespace std;

const unsigned int WAVE_FILE_MARK = 0x46464952; // RIFF
const unsigned int ID3_FILE_MARK = 0x03334449;  // ID3

FormatRecognizer::FormatRecognizer()
{
}

COMPRESSIONS_FORMATS FormatRecognizer::GetFormatType(QString strFilePath)
{
    std::ifstream inFile(strFilePath.toStdString().c_str(), ios::in|ios::binary);
    unsigned int headerMark;
    COMPRESSIONS_FORMATS format = COMPRESSION_UNKNOWN;

    if(inFile.is_open())
    {
        inFile.read((char *) &headerMark, sizeof(headerMark));

        switch(headerMark)
        {
        case WAVE_FILE_MARK:
            format = COMPRESSION_U8;
            break;
        case ID3_FILE_MARK:
            format = COMPRESSION_MPEG3;
            break;
        }

        inFile.close();
    }
    else
    {
        format = COMPRESSION_FILE_ERROR;
    }

    return format;
}

