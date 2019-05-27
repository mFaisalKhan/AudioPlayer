#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QString>
#include "GUI/mainwindow.h"
#include <QApplication>
#include "AudioPlayer/ALSAWrapper/ALSAWrapper.h"
#include "AudioPlayer/IFFileFormat.h"
#include "AudioFormats/DecoderBase.h"

using namespace AlsaWrapperSpace;

class AudioPlayer: public QObject
{
 Q_OBJECT
public:
    AudioPlayer(ALSAWrapper *alsa);
    virtual ~AudioPlayer();

    void SetSource(QString src);
    void ProcessEvents();
    void MainWindowClosing();
private:
    void Play();
    void Pause();
    void Stop();

    static const int MaxLevel = 100;
    static const int MinLevel = 0;
    QString SourceName;
    ALSAWrapper *Alsa;
    IFFileFormat*  Audioformat;
    DecoderBase*   Decoder;
    int Pipefd[2];
    std::ifstream *FptrIn;

private slots:
    void SlotFileNameChanged(QString);
    void SlotPlayClicked();
    void SlotPauseClicked();
    void SlotStopClicked();
    void SlotSliderMoved(int);
signals:
    void SignalPlayStatus(bool isPlaying);
    void VolumeChanged(int level);
    void SignalUpdateSongInfo(QString album, QString Title);
};

#endif // AUDIOPLAYER_H
