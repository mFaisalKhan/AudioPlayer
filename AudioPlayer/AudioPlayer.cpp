#include "AudioPlayer.h"
#include "WaveReader.h"
#include "FormatRecognizer.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include "MP3Reader.h"
#include "AudioFormats/Mp3Decoder.h"
#include <ios>

using namespace std;

AudioPlayer::AudioPlayer(ALSAWrapper *alsa)
    : QObject()
    , Alsa(alsa)
    , Audioformat(NULL)
    , Decoder(NULL)
    , FptrIn(NULL)
{

}

void AudioPlayer::SetSource(QString src)
{
    SourceName = src;
}

void AudioPlayer::Play()
{
    FormatRecognizer fileFormatReader;
    AudioFileProperties fmt;

    int compressionType = fileFormatReader.GetFormatType(SourceName);

    if(compressionType == COMPRESSION_FILE_ERROR )
    {
        qDebug () << "Cannot find File " << SourceName;
        return;
    }

    if( compressionType == COMPRESSION_U8)
    {
        // this is wave file
        Audioformat = new WaveReader();
    }
    else
        if(compressionType == COMPRESSION_MPEG3)
        {
            // this is mp3 file
            Audioformat = new MP3Reader();
        }


    QString statusStr = Audioformat->GetAudioFileProperties(SourceName, &fmt, &FptrIn);
    emit SignalUpdateSongInfo(Audioformat->GetAlbumName() + "\n", Audioformat->GetTitle());
    if(statusStr == "NO ERROR")
    {
        if(fmt.AudioFormat == COMPRESSION_MPEG3)
        {
            Decoder = new MP3Decoder();
        }
        else
            Decoder = new DecoderBase();


        if(pipe(Pipefd) == -1)
        {
            qDebug() << "Error creating pipe";
        }
        else
        {
            int flags = fcntl(Pipefd[0], F_GETFL, 0);

            if(fcntl(Pipefd[0], F_SETFL, flags | O_NONBLOCK)<0)
            {
                qDebug() << "Error fcntl" << errno ;
            }

            connect(Decoder, SIGNAL(SignalDecodingFinished()),Alsa,SLOT(DecodingFinished()));
            Alsa->SetHardwareParams(fmt);

            qDebug() << "File opened for writing";
            Decoder->SetStreams(FptrIn, Pipefd[1]);
            Alsa->PlayAudio(Pipefd[0]);
            Decoder->start();
            sleep(1);
            Alsa->start();
            emit SignalPlayStatus(true);
        }
    }
}



void AudioPlayer::Pause()
{
    Alsa->Pause();
}

void AudioPlayer::Stop()
{
    emit SignalPlayStatus(false);

    if(Audioformat)
    {
        delete Audioformat;
        Audioformat = NULL;
    }

    if(Decoder)
    {
        Decoder->Stop();

        sleep(1);
        delete Decoder;
        Decoder = NULL;
    }


    close(Pipefd[0]);
    close(Pipefd[1]);
    FptrIn->close();
}

void AudioPlayer::ProcessEvents()
{
    static bool AlsaRunning = false;

    Alsa->ProcessEvents();
    if(Alsa->isRunning() && !AlsaRunning)
    {
       qDebug() << "Thread started";
        AlsaRunning = true;

    }

    if(AlsaRunning && Alsa->isFinished())
    {
        qDebug() << "Thread stopped";
        AlsaRunning = false;
        close(Pipefd[0]);
        emit SignalPlayStatus(false);

        Alsa = NULL;
        delete Audioformat;
        Audioformat = NULL;
    }

    if(Decoder)
    {
//        if(!DecoderRunning && Decoder->isRunning())
//        {
//            qDebug() << "Decoder Started ";
//            DecoderRunning = true;
//        }

        Decoder->exit();

            close(Pipefd[1]);
            delete Decoder;
            Decoder = NULL;
    }
}

void AudioPlayer::MainWindowClosing()
{
    ProcessEvents();
}

void AudioPlayer::SlotFileNameChanged(QString fileName)
{
    qDebug()<< "File Name =" << fileName;
    SourceName = fileName;
 }


void AudioPlayer::SlotPlayClicked()
{
    Play();
}

void AudioPlayer::SlotPauseClicked()
{
    Pause();
}

void AudioPlayer::SlotStopClicked()
{
    Stop();
}

void AudioPlayer::SlotSliderMoved(int position)
{
    Alsa->SetVolume(position);
}

AudioPlayer::~AudioPlayer()
{
    if(Audioformat)
    {
        delete Audioformat;
        Audioformat = NULL;
    }

    if(Decoder)
    {
        Decoder->exit();
        delete Decoder;
        Decoder = NULL;
    }
}
