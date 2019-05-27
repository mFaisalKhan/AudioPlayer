#ifndef ALSAWRAPPER_H
#define ALSAWRAPPER_H
#include <QString>
#include <QObject>
#include <alsa/asoundlib.h>
#include <AudioPlayer/WaveReader.h>
#include <QThread>

namespace AlsaWrapperSpace
{
const QString ERROR_NONE = "NO ERROR";


class ALSAWrapper:public QThread
{
    Q_OBJECT

public:
    ALSAWrapper();
    virtual ~ALSAWrapper();
    QString GetVersion();
    QString SetVolume(int value);
    void ProcessEvents();
    void EmitVolumeChanged(int level);
    bool SetHardwareParams(AudioFileProperties fileFormat);
    bool PlayAudio( int fdPipeReadEnd);
    void Pause();

    void EndPlay();
    void PlaySample(int size);
    QString DetectCard(); // return device name

signals:
    void VolumeChanged(int);

private slots:
    void DecodingFinished();

private:
    QString SetAlsaMasterVolume(long volume);
    QString InitMixer();
    _snd_pcm_format MapBitRateToALSAResolution(unsigned int bitrate, bool bigEndian, bool sign);
    unsigned int GetSupportedBitRate(COMPRESSIONS_FORMATS format, unsigned int bitRate);
    unsigned int GetSupportedSampleRate(COMPRESSIONS_FORMATS format, unsigned int sampleRate);
    unsigned short GetSupportedChannels(COMPRESSIONS_FORMATS format, unsigned short channel);

private:
    long                VolMax, VolMin;
    snd_mixer_elem_t*   MixerElem;
    snd_mixer_t *       HandleMixer;
    snd_pcm_t   *       HandlePCMOut;
    unsigned char *     Audiobuffer;
    snd_pcm_uframes_t   BufferSizeInFrames;
    snd_pcm_hw_params_t *HwParams;
    snd_pcm_uframes_t   PeriodSize;
    int FrameSize;
    int SampleSize;
    int                 PipeReadEnd;
    bool                KeepRunning;

    void PrintHWParams();
public:
    void run();
    void stop();
};



}
#endif // ALSAWRAPPER_H
