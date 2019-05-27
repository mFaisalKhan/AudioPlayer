#include "ALSAWrapper.h"

using namespace AlsaWrapperSpace;
#include <alsa/pcm.h>
#include <QDebug>
#include <stdio.h>

static ALSAWrapper *AlsaRef=NULL;
static long Max = 0;

////////////////////////////////////////////////////////////////////////////////////////////
ALSAWrapper::ALSAWrapper()
          : MixerElem(NULL)
          , HandleMixer(NULL)
          , HandlePCMOut(NULL)
          , Audiobuffer(NULL)
          , BufferSizeInFrames(0)
          , HwParams(NULL)
          , PeriodSize(0)
          , FrameSize(0)
          , SampleSize(0)
          , KeepRunning(false)
{
    VolMax = 0;
    VolMin = 0;
    AlsaRef = this;
    InitMixer();

}

////////////////////////////////////////////////////////////////////////////////////////////
ALSAWrapper::~ALSAWrapper()
{

}

////////////////////////////////////////////////////////////////////////////////////////////
QString ALSAWrapper::GetVersion()
{
    const char *version;

    version = snd_asoundlib_version();
    QString VersionString(version);

    return VersionString;
}

////////////////////////////////////////////////////////////////////////////////////////////
QString ALSAWrapper::SetVolume(int value)
{
    return SetAlsaMasterVolume(value);
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::ProcessEvents()
{

    int res;
    res = snd_mixer_wait(HandleMixer, 0);
    if (res >= 0)
    {
        //printf("Poll ok: %i\n", res);
        res = snd_mixer_handle_events(HandleMixer);
        if(res > 0)
            qDebug() << "snd_mixer_handle_events : " << res;
    }

}

////////////////////////////////////////////////////////////////////////////////////////////
QString ALSAWrapper::SetAlsaMasterVolume(long volume)
{
    QString err = ERROR_NONE;


    if(err == ERROR_NONE && MixerElem)
    {

        snd_mixer_selem_set_playback_volume_all(MixerElem, volume * VolMax / 100);

    }
    else
    {
        qDebug () << "Playback failed";
    }

    return err;
}

////////////////////////////////////////////////////////////////////////////////////////////
void UpdatecurrentVolumeLevel(snd_mixer_elem_t *elem)
{
    long volumeLevel = 0;


    snd_mixer_selem_get_playback_volume (elem, SND_MIXER_SCHN_FRONT_LEFT, &volumeLevel);

    double curVol = (volumeLevel/ (double)Max) * 100.0 ;

    if(AlsaRef && curVol>=0)
        AlsaRef->EmitVolumeChanged((long)curVol) ;
    printf("CurVol = %f",curVol);
    qDebug () << "Mixer_CallBack  "  << "Value " << volumeLevel << " Curr =" << curVol;
}

////////////////////////////////////////////////////////////////////////////////////////////
int Mixer_CallBack(snd_mixer_elem_t *elem, unsigned int mask)
{
    UpdatecurrentVolumeLevel(elem);
    Q_UNUSED(mask);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::EmitVolumeChanged(int level)
{
    emit VolumeChanged(level);
}

////////////////////////////////////////////////////////////////////////////////////////////
_snd_pcm_format ALSAWrapper::MapBitRateToALSAResolution(unsigned int bitrate, bool bigEndian, bool sign)
{
    _snd_pcm_format resolution=SND_PCM_FORMAT_U8;

    switch(bitrate)
    {
    case 8000:
        SampleSize = 1;
//        if(sign)
//            resolution = SND_PCM_FORMAT_S8;
//        else
            resolution = SND_PCM_FORMAT_U8;
        break;

    case 16000: //pcm uncompressed
        SampleSize = 2;
        if(bigEndian)
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S16_BE;
            else
                resolution = SND_PCM_FORMAT_U16_BE;
        }
        else
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S16_LE;
            else
                resolution = SND_PCM_FORMAT_U16_LE;
        }

        break;

    case 24000: // microsoft adpcm
        SampleSize = 3;
        if(bigEndian)
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S24_BE;
            else
                resolution = SND_PCM_FORMAT_U24_BE;
        }
        else
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S24_LE;
            else
                resolution = SND_PCM_FORMAT_U24_LE;
        }
        break;

    case 32000: // A-Law
        SampleSize = 4;
        if(bigEndian)
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S32_BE;
            else
                resolution = SND_PCM_FORMAT_U32_BE;
        }
        else
        {
            if(sign)
                resolution = SND_PCM_FORMAT_S32_LE;
            else
                resolution = SND_PCM_FORMAT_U32_LE;
        }
        break;

    case 64000:
        SampleSize = 8;
        if(bigEndian)
            resolution = SND_PCM_FORMAT_FLOAT64_BE;
        else
            resolution = SND_PCM_FORMAT_FLOAT64_LE;

        break;

    default:
        resolution = SND_PCM_FORMAT_UNKNOWN;
        break;

    }
    return resolution;
}

////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ALSAWrapper::GetSupportedBitRate(COMPRESSIONS_FORMATS format, unsigned int bitRate)
{
    if(format==COMPRESSION_MPEG3 )
    {
            bitRate = 16000; // this is lib MAD output format
    }

    return bitRate;
}

////////////////////////////////////////////////////////////////////////////////////////////
unsigned int ALSAWrapper::GetSupportedSampleRate(COMPRESSIONS_FORMATS format, unsigned int sampleRate)
{
    Q_UNUSED(format);
    return sampleRate;
}

////////////////////////////////////////////////////////////////////////////////////////////
unsigned short ALSAWrapper::GetSupportedChannels(COMPRESSIONS_FORMATS format, unsigned short channel)
{

    // MAD supports 2 channels even for mono
    if(format==COMPRESSION_MPEG3 && channel==1)
        channel = 2;
    return channel;
}

////////////////////////////////////////////////////////////////////////////////////////////
bool ALSAWrapper::PlayAudio( int fdPipeReadEnd)
{
    PipeReadEnd = fdPipeReadEnd;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::PrintHWParams()
{

    int err=0;
    unsigned long buffer_size=0;
    int dir;

    err = snd_pcm_hw_params_get_buffer_size(HwParams, &buffer_size);
    qDebug() << "buffer_size " << buffer_size << "err:" << err;
    // BufferSize = buffer_size;

    err = snd_pcm_hw_params_get_buffer_size_min(HwParams, &buffer_size);
    qDebug() << "buffer_size min " << buffer_size << "err:" << err;

    err = snd_pcm_hw_params_get_buffer_size_max(HwParams, &buffer_size);
    qDebug() << "buffer_size max" << buffer_size << "err:" << err;

    unsigned int buffer_time=0;

    err = snd_pcm_hw_params_get_buffer_time(HwParams, &buffer_time, &dir);
    qDebug() << "buffer_time " << buffer_time << "err:" << err;

    err = snd_pcm_hw_params_get_buffer_time_min(HwParams, &buffer_time, &dir);
    qDebug() << "buffer_time min " << buffer_time << "err:" << err;

    err = snd_pcm_hw_params_get_buffer_time_max(HwParams, &buffer_time, &dir);
    qDebug() << "buffer_time max" << buffer_time << "err:" << err;

    /////////////

    unsigned long period_size=0;
    err = snd_pcm_hw_params_get_period_size(HwParams, &period_size, &dir);
    qDebug() << "period_size " << period_size << "err:" << err;
    // PeriodSize = period_size;

    err = snd_pcm_hw_params_get_period_size_min(HwParams, &period_size, &dir);
    qDebug() << "period_size min " << period_size << "err:" << err;

    err = snd_pcm_hw_params_get_period_size_max(HwParams, &period_size, &dir);
    qDebug() << "period_size max " << period_size << "err:" << err;


    //////////////
    unsigned int period_time = 0;
    err = snd_pcm_hw_params_get_period_time(HwParams, &period_time, &dir);
    qDebug() << "period_time " << period_time << "err:" << err;

    err = snd_pcm_hw_params_get_period_time_min(HwParams, &period_time, &dir);
    qDebug() << "period_time min " << period_time << "err:" << err;

    err = snd_pcm_hw_params_get_period_time_max(HwParams, &period_time, &dir);
    qDebug() << "period_time max " << period_time << "err:" << err;


}

////////////////////////////////////////////////////////////////////////////////////////////
bool ALSAWrapper::SetHardwareParams(AudioFileProperties fileFormat)
{
    // Set snd_hardware params
    bool        bRet = false;
    int err;
    int dir=0;

    unsigned int format = GetSupportedBitRate(fileFormat.AudioFormat,fileFormat.BitRate);
    unsigned int rate = GetSupportedSampleRate(fileFormat.AudioFormat,fileFormat.SampleRate);
    unsigned int channel = GetSupportedChannels(fileFormat.AudioFormat,fileFormat.NumChannels);
    snd_pcm_format_t resolution = MapBitRateToALSAResolution(format, false, true);
    FrameSize = SampleSize * channel;

    qDebug() << "format, rate, channel, resolution" << format << "," << rate << "," << channel << "," << resolution;

    if ((err = snd_pcm_open(&HandlePCMOut, DetectCard().toAscii(), SND_PCM_STREAM_PLAYBACK, 0)) >= 0)
    {
        if ((err = snd_pcm_hw_params_malloc(&HwParams)) >= 0)
        {

            if ((err = snd_pcm_hw_params_any(HandlePCMOut, HwParams)) >= 0)
            {
                
                if ((err = snd_pcm_hw_params_set_format(HandlePCMOut, HwParams, resolution)) >= 0)
                {
                    /* Interleaved mode */
                   err = snd_pcm_hw_params_set_access(HandlePCMOut, HwParams,
                                                 SND_PCM_ACCESS_RW_INTERLEAVED);

                    err = snd_pcm_hw_params_set_channels(HandlePCMOut, HwParams, channel);

                    unsigned int reqRate = rate;

                    if ((err = snd_pcm_hw_params_set_rate_near(HandlePCMOut, HwParams, &reqRate, &dir)) >= 0)
                    {
                        qDebug() << "Req rate :" << rate << " Rate Set :" << reqRate << "dir" << dir;

                        snd_pcm_uframes_t buffer_size = 0;

                        buffer_size = 4096;
                        err = snd_pcm_hw_params_set_buffer_size_near(HandlePCMOut, HwParams, &buffer_size);
                        if (err < 0) {
                            printf("Unable to set buffer_size %i for playback: %s\n", (int)buffer_size, snd_strerror(err));
                            // return false;
                        }

                        BufferSizeInFrames = buffer_size;

                        if ((err = snd_pcm_hw_params(HandlePCMOut, HwParams)) >= 0)
                        {
                            bRet = true;
                            err = snd_pcm_prepare(HandlePCMOut);

                            if(Audiobuffer == NULL)
                            {
                             Audiobuffer = new unsigned char[BufferSizeInFrames * FrameSize];
                            }
                            else
                                qDebug() << "Buffer not empty!";

                            qDebug() << "BufferSizeInFrames:" << BufferSizeInFrames << "FrameSize " << FrameSize;

                            PrintHWParams();
                        }
                        else
                        {
                            printf("Can't set hardware params: %s\n", snd_strerror(err));
                        }

                    }
                    else
                    {
                        printf("Can't set sample rate %d: %s\n",rate, snd_strerror(err));
                    }
                }
                else
                {
                    printf("Can't set 16-bit %d: %s\n", resolution, snd_strerror(err));
                }

            }
            else
            {
                printf("Can't init sound hardware struct: %s\n", snd_strerror(err));
            }
        }
        else
        {
            printf("Can't get sound hardware struct %s\n", snd_strerror(err));
        }
    }
    else
    {
        printf("Can't open wave output: %s\n", snd_strerror(err));
    }


    return(bRet);
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::Pause()
{

}

////////////////////////////////////////////////////////////////////////////////////////////
QString ALSAWrapper::InitMixer()
{
    static snd_mixer_selem_id_t *sid;
    static const char *card = "default";
    static const char *selem_name = "Master";
    int err=0;

    err = snd_mixer_open(&HandleMixer, 0);
    if(err == 0)
    {
        err =  snd_mixer_attach(HandleMixer, card);
        if (err == 0)
        {
            err = snd_mixer_selem_register(HandleMixer, NULL, NULL);
            if(err == 0)
            {
                err = snd_mixer_load(HandleMixer);
                if(err == 0 )
                {
                    snd_mixer_selem_id_alloca(&sid);
                    snd_mixer_selem_id_set_index(sid, 0);
                    snd_mixer_selem_id_set_name(sid, selem_name);
                    MixerElem = snd_mixer_find_selem(HandleMixer, sid);
                    if(MixerElem)
                    {
                        snd_mixer_elem_set_callback(MixerElem, &Mixer_CallBack);
                        snd_mixer_elem_set_callback_private(MixerElem, HandleMixer);

                        snd_mixer_selem_get_playback_volume_range(MixerElem, &VolMin, &VolMax);
                        Max = VolMax;
                        UpdatecurrentVolumeLevel(MixerElem);
                    }
                    else
                    {
                        qDebug() << "Cannot Find Mixer element sid = " << sid;
                    }

                }
                else
                {
                    qDebug() << "snd_mixer_selem_register Failed Err " << err;
                }
            }
            else
            {
                qDebug() << "snd_mixer_selem_register Failed Err " << err;
            }
        }
        else
        {
            qDebug() << "snd_mixer_attach Failed Err " << err;
        }

    }   else
    {
        qDebug() << "Cannot Open snd mixer: err " << err;
    }

    return ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::stop()
{
    qDebug() << "ALSAWrapper::stop()";
 //   KeepRunning = false;
 // EndPlay();
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::PlaySample(int size)
{
    snd_pcm_sframes_t ret = snd_pcm_writei(HandlePCMOut, Audiobuffer, size);
    if (ret == -EPIPE) {
        /* EPIPE means underrun */
        fprintf(stderr, "underrun occurred\n");
        snd_pcm_prepare(HandlePCMOut);
    } else if (ret < 0) {
        fprintf(stderr,
                "error from writei: %s\n",
                snd_strerror(ret));
    }  else if (ret != (int)size) {
        fprintf(stderr,
                "short write, write %d frames\n",(int) ret);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////
// Detect sound card and return device name as QString
QString ALSAWrapper::DetectCard()
{
    char devName[30];

    memset(devName,0,30);
    sprintf(devName,"plughw:0,0");
    FILE *fptr = fopen("settings.ini", "r");
    if(fptr)
    {
        fgets(devName, 30, fptr);
        fclose(fptr);
    }

    QString deviceName(devName);

    return deviceName;
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::EndPlay()
{
    KeepRunning = false;

    snd_pcm_pause(HandlePCMOut, 1);
    if(HandlePCMOut)
    {
        int pcmState = snd_pcm_state(HandlePCMOut);

        while(pcmState== 3)
        {
           pcmState = snd_pcm_state(HandlePCMOut);
        qDebug() << "PCM state:" << pcmState;
        }

        qDebug() << "Changed PCM state:" << pcmState;
        snd_pcm_drain(HandlePCMOut);
        snd_pcm_close(HandlePCMOut);
        HandlePCMOut = NULL;
    }

    if(HwParams)
    {
        snd_pcm_hw_params_free(HwParams);
        HwParams = NULL;
    }

    if(Audiobuffer)
    {
        delete[] Audiobuffer;
        Audiobuffer = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::run()
{
    KeepRunning = true;
    int x=0;
    unsigned int bytesReadPos = 0;
    unsigned int requested = BufferSizeInFrames * FrameSize;
    const unsigned int bufferSizeInBytes = BufferSizeInFrames * FrameSize;
    int currReq = requested;

    while (1)
    {
        x = read(PipeReadEnd, Audiobuffer+bytesReadPos, currReq);

        if(x <= 0 && KeepRunning==false)
        {
            qDebug() << "End of stream in AlwaWrapper. Stopping";
            break;
        }
        else if (x < currReq)
        {
            bytesReadPos += x;
            currReq -= x;
            if(bytesReadPos < bufferSizeInBytes)
            {
                // we haven't read enough bytes from the pipe. Read again
                qDebug() << "Not enough bytes read yet" << "buffSize = "<< bufferSizeInBytes << " Remainging" << bufferSizeInBytes - bytesReadPos;
                continue;
            }
        }

            currReq = requested;
            bytesReadPos = 0;
            PlaySample(BufferSizeInFrames);


    }

    do
    {
         x = read(PipeReadEnd, Audiobuffer, requested);
         qDebug() << "flushing x=" << x;
    }while(x>0);

    EndPlay();
    qDebug() << "Wrapper End";
    exit();
}

////////////////////////////////////////////////////////////////////////////////////////////
void ALSAWrapper::DecodingFinished()
{
    qDebug() << "Decoding finished signal";
    KeepRunning = false;
}



