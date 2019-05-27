#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H

#include <QThread>
#include <fstream>


class DecoderBase: public QThread
{
 Q_OBJECT
 public:
    DecoderBase();
    virtual ~DecoderBase();
    virtual QString SetStreams(std::ifstream * inStream, int fdInEnd);
    virtual void run();
    virtual void Stop();

signals:
    void SignalDecodingFinished();

 private:
    std::ifstream *InStream;
    int fdOutput;
    bool IsRunning;
};


#endif // AUDIOFORMAT_H
