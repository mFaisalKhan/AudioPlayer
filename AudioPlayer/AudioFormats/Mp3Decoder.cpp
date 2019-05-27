#include "Mp3Decoder.h"
#include <mad.h>
#include <errno.h>
#include <QDebug>

static const char ProgName[30]="MP3";

MP3Decoder::MP3Decoder()
    : DecoderBase()
    , InStream(NULL)
    , fdOutput(-1)
    , IsRunning(false)
{
}

MP3Decoder::~MP3Decoder()
{

}

/****************************************************************************
 * Return an error string associated with a mad error code.					*
 ****************************************************************************/
/* Mad version 0.14.2b introduced the mad_stream_errorstr() function.
 * For previous library versions a replacement is provided below.
 */
#if (MAD_VERSION_MAJOR>=1) || \
    ((MAD_VERSION_MAJOR==0) && \
    (((MAD_VERSION_MINOR==14) && \
    (MAD_VERSION_PATCH>=2)) || \
    (MAD_VERSION_MINOR>14)))
#define MadErrorString(x) mad_stream_errorstr(x)
#else
const char *MadErrorString(const struct mad_stream *Stream)
{
    switch(Stream->error)
    {
    /* Generic unrecoverable errors. */
    case MAD_ERROR_BUFLEN:
        return("input buffer too small (or EOF)");
    case MAD_ERROR_BUFPTR:
        return("invalid (null) buffer pointer");
    case MAD_ERROR_NOMEM:
        return("not enough memory");

        /* Frame header related unrecoverable errors. */
    case MAD_ERROR_LOSTSYNC:
        return("lost synchronization");
    case MAD_ERROR_BADLAYER:
        return("reserved header layer value");
    case MAD_ERROR_BADBITRATE:
        return("forbidden bitrate value");
    case MAD_ERROR_BADSAMPLERATE:
        return("reserved sample frequency value");
    case MAD_ERROR_BADEMPHASIS:
        return("reserved emphasis value");

        /* Recoverable errors */
    case MAD_ERROR_BADCRC:
        return("CRC check failed");
    case MAD_ERROR_BADBITALLOC:
        return("forbidden bit allocation value");
    case MAD_ERROR_BADSCALEFACTOR:
        return("bad scalefactor index");
    case MAD_ERROR_BADFRAMELEN:
        return("bad frame length");
    case MAD_ERROR_BADBIGVALUES:
        return("bad big_values count");
    case MAD_ERROR_BADBLOCKTYPE:
        return("reserved block_type");
    case MAD_ERROR_BADSCFSI:
        return("bad scalefactor selection info");
    case MAD_ERROR_BADDATAPTR:
        return("bad main_data_begin pointer");
    case MAD_ERROR_BADPART3LEN:
        return("bad audio data length");
    case MAD_ERROR_BADHUFFTABLE:
        return("bad Huffman table select");
    case MAD_ERROR_BADHUFFDATA:
        return("Huffman data overrun");
    case MAD_ERROR_BADSTEREO:
        return("incompatible block_type for JS");

        /* Unknown error. This switch may be out of sync with libmad's
         * defined error codes.
         */
    default:
        return("Unknown error code");
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////
QString MP3Decoder::SetStreams(std::ifstream *inStream, int fdInEnd)
{
    InStream = inStream;
    fdOutput = fdInEnd;
    IsRunning = true;

    return QString("NO Error");
}


/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
signed short MadFixedToSshort(mad_fixed_t Fixed)
{
    /* Clipping */
    if(Fixed>=MAD_F_ONE)
        return(SHRT_MAX);
    if(Fixed<=-MAD_F_ONE)
        return(-SHRT_MAX);

    /* Conversion. */
    Fixed=Fixed>>(MAD_F_FRACBITS-15);
    return((signed short)Fixed);
}

/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
int MP3Decoder::PrintFrameInfo(FILE *fp, struct mad_header *Header)
{
    const char	*Layer,
            *Mode,
            *Emphasis;

    /* Convert the layer number to it's printed representation. */
    switch(Header->layer)
    {
    case MAD_LAYER_I:
        Layer="I";
        break;
    case MAD_LAYER_II:
        Layer="II";
        break;
    case MAD_LAYER_III:
        Layer="III";
        break;
    default:
        Layer="(unexpected layer value)";
        break;
    }

    /* Convert the audio mode to it's printed representation. */
    switch(Header->mode)
    {
    case MAD_MODE_SINGLE_CHANNEL:
        Mode="single channel";
        break;
    case MAD_MODE_DUAL_CHANNEL:
        Mode="dual channel";
        break;
    case MAD_MODE_JOINT_STEREO:
        Mode="joint (MS/intensity) stereo";
        break;
    case MAD_MODE_STEREO:
        Mode="normal LR stereo";
        break;
    default:
        Mode="(unexpected mode value)";
        break;
    }

    /* Convert the emphasis to it's printed representation. Note that
     * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
     * version 0.15.0b.
     */
    switch(Header->emphasis)
    {
    case MAD_EMPHASIS_NONE:
        Emphasis="no";
        break;
    case MAD_EMPHASIS_50_15_US:
        Emphasis="50/15 us";
        break;
    case MAD_EMPHASIS_CCITT_J_17:
        Emphasis="CCITT J.17";
        break;
#if (MAD_VERSION_MAJOR>=1) || \
    ((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
    case MAD_EMPHASIS_RESERVED:
        Emphasis="reserved(!)";
        break;
#endif
    default:
        Emphasis="(unexpected emphasis value)";
        break;
    }

    fprintf(fp,"%s: %lu kb/s audio MPEG layer %s stream %s CRC, "
            "%s with %s emphasis at %d Hz sample rate\n",
            ProgName,Header->bitrate,Layer,
            Header->flags&MAD_FLAG_PROTECTION?"with":"without",
            Mode,Emphasis,Header->samplerate);
    return(ferror(fp));
}


/****************************************************************************
 * Main decoding loop. This is where mad is used.							*
 ****************************************************************************/
int MP3Decoder::MpegAudioDecoder()

{
    struct mad_stream	Stream;
    struct mad_frame	Frame;
    struct mad_synth	Synth;
    mad_timer_t			Timer;
    unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
            OutputBuffer[OUTPUT_BUFFER_SIZE],
            *OutputPtr=OutputBuffer,
            *GuardPtr=NULL;
    const unsigned char	*OutputBufferEnd=OutputBuffer+OUTPUT_BUFFER_SIZE;
    int					Status=0, i;
    unsigned long		FrameCount=0;

    /* First the structures used by libmad must be initialized. */
    mad_stream_init(&Stream);
    mad_frame_init(&Frame);
    mad_synth_init(&Synth);
    mad_timer_reset(&Timer);

    /* This is the decoding loop. */
    do
    {
        if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
        {
            size_t			ReadSize, Remaining;
            unsigned char	*ReadStart;

            if(Stream.next_frame!=NULL)
            {
                Remaining=Stream.bufend-Stream.next_frame;
                memmove(InputBuffer,Stream.next_frame,Remaining);
                ReadStart=InputBuffer+Remaining;
                ReadSize=INPUT_BUFFER_SIZE-Remaining;
            }
            else
            {
                ReadSize=INPUT_BUFFER_SIZE;
                ReadStart=InputBuffer;
                Remaining=0;
            }

            InStream->read((char *)ReadStart, ReadSize);
            ReadSize= InStream->gcount();

            if(ReadSize<=0)
            {

                if(InStream->bad())
                {
                    fprintf(stderr,"%s: read error on bit-stream (%s)\n",
                            ProgName,strerror(errno));
                    Status=1;
                }
                if(InStream->eof())
                    fprintf(stderr,"%s: end of input stream\n",ProgName);
                break;
            }

            if(InStream->eof())
            {
                GuardPtr=ReadStart+ReadSize;
                memset(GuardPtr,0,MAD_BUFFER_GUARD);
                ReadSize+=MAD_BUFFER_GUARD;
            }

            /* Pipe the new buffer content to libmad's stream decoder
             * facility.
             */
            mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
            Stream.error = MAD_ERROR_NONE;
        }

        if(mad_frame_decode(&Frame,&Stream))
        {
            if(MAD_RECOVERABLE(Stream.error))
            {
                if(Stream.error!=MAD_ERROR_LOSTSYNC ||
                        Stream.this_frame!=GuardPtr)
                {
                    fprintf(stderr,"%s: recoverable frame level error (%s)\n",
                            ProgName,MadErrorString(&Stream));
                    fflush(stderr);
                }
                continue;
            }
            else
                if(Stream.error==MAD_ERROR_BUFLEN)
                    continue;
                else
                {
                    fprintf(stderr,"%s: unrecoverable frame level error (%s).\n",
                            ProgName,MadErrorString(&Stream));
                    Status=1;
                    break;
                }
        }

        if(FrameCount==0)
            if(PrintFrameInfo(stderr,&Frame.header))
            {
                Status=1;
                break;
            }

        FrameCount++;
        mad_timer_add(&Timer,Frame.header.duration);

        mad_synth_frame(&Synth,&Frame);

        for(i=0;i<Synth.pcm.length;i++)
        {
            signed short	Sample;


            if(Synth.pcm.length > OUTPUT_BUFFER_SIZE)
            {
                qDebug() << "PCM Length too long" << Synth.pcm.length;
            }
            /* Left channel */
            Sample=MadFixedToSshort(Synth.pcm.samples[0][i]);
            *(OutputPtr++)=Sample&0xff;
            *(OutputPtr++)=Sample>>8;

            if(MAD_NCHANNELS(&Frame.header)==2)
                Sample=MadFixedToSshort(Synth.pcm.samples[1][i]);
            *(OutputPtr++)=Sample&0xff;
            *(OutputPtr++)=Sample>>8;

            if((OutputPtr - OutputBuffer) > OUTPUT_BUFFER_SIZE)
            {
                qDebug() << "Outputptr goes beyond" << (OutputPtr - OutputBuffer);
            }

            /* Flush the output buffer if it is full. */
            if(OutputPtr==OutputBufferEnd)
            {
                int sz = write(fdOutput, (char *)OutputBuffer,OUTPUT_BUFFER_SIZE);

                if(sz!=OUTPUT_BUFFER_SIZE)
                {
                    fprintf(stderr,"%s: PCM write error (%s).\n",
                            ProgName,strerror(errno));
                    Status=2;
                    break;
                }
                OutputPtr=OutputBuffer;
            }
        }
    }while(IsRunning==true);


    mad_synth_finish(&Synth);
    mad_frame_finish(&Frame);
    mad_stream_finish(&Stream);

    if(OutputPtr!=OutputBuffer && Status!=2)
    {
        ssize_t	BufferSize= OutputPtr - OutputBuffer  ;

        qDebug() << "BufferSize = " << BufferSize;

        if(write(fdOutput,OutputBuffer,BufferSize)!=BufferSize)
        {
            fprintf(stderr,"%s: PCM write error (%s).\n",
                    ProgName,strerror(errno));
            Status=2;
        }
    }

    /* Accounting report if no error occurred. */
    if(!Status)
    {
        char Buffer[80];

        mad_timer_string(Timer,Buffer,"%lu:%02lu.%03u",
                         MAD_UNITS_MINUTES,MAD_UNITS_MILLISECONDS,0);
        fprintf(stderr,"%s: %lu frames decoded (%s).\n",
                ProgName,FrameCount,Buffer);
    }

    qDebug() << "Decoding loop done";
        close(fdOutput);
    /* That's the end of the world (in the H. G. Wells way). */
    return(Status);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void MP3Decoder::run()
{
    MpegAudioDecoder();

    emit SignalDecodingFinished();
    qDebug() << "MP3 Decoding finished";
    exit();
}

void MP3Decoder::Stop()
{
    IsRunning = false;
}
