#ifndef FORMATRECOGNIZER_H
#define FORMATRECOGNIZER_H

#include <QString>
#include "IFFileFormat.h"

class FormatRecognizer
{
public:
    FormatRecognizer();
    COMPRESSIONS_FORMATS GetFormatType(QString strFilePath);
};

#endif // FORMATRECOGNIZER_H
