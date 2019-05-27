#ifndef MP3DECODER_H
#define MP3DECODER_H

#include "DecoderBase.h"
#include "AudioPlayer/ALSAWrapper/ALSAWrapper.h"

#define INPUT_BUFFER_SIZE	5 * (4096)
#define OUTPUT_BUFFER_SIZE 4096 /* Must be an integer multiple of 4. */

using namespace AlsaWrapperSpace;

class MP3Decoder: public DecoderBase
{
public:
    MP3Decoder();
    virtual ~MP3Decoder();
    virtual QString SetStreams(std::ifstream * inStream, int fdInEnd);
    int PrintFrameInfo(FILE *fp, struct mad_header *Header);
    virtual void run();
    void Stop();

private:
    std::ifstream *InStream;
    int   fdOutput;
    bool IsRunning;

    int MpegAudioDecoder();

};

#endif // MP3DECODER_H
