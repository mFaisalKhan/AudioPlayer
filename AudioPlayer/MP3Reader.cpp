#include "MP3Reader.h"
#include <QString>
#include <ios>
#include <QDebug>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////
MP3Reader::MP3Reader()
    : File(NULL)
    , ID3FooterLength(0)
    , ID3HeaderLength(0)
{
}

////////////////////////////////////////////////////////////////////////////////////////////
unsigned int MP3Reader::ReadBits(unsigned int data, int pos, int count)
{
    unsigned int result = 0;
    unsigned int temp=data;  //4 bytes
    unsigned int mask = 0xFFFFFFFF;

    int shift = (sizeof(data)*8) - (pos + count) ;
    temp = data >> shift;
    mask = mask << count;
    mask = ~mask;

    result = temp & mask;

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString MP3Reader::GetAudioFileProperties(QString filePath, AudioFileProperties *format, std::ifstream **fptr )
{
    QString err= "NO ERROR";
    FrameMP3 FrameHeader;
    File = new ifstream(filePath.toStdString().c_str(), ios::in|ios::binary);

    if( File->is_open())
    {
        ID3FooterLength = GetID3FooterLength();
        if(FindMP3Frame())
        {
            ID3HeaderLength = File->tellg();
            unsigned int ActualData=0;
            File->read((char*)&ActualData, sizeof(ActualData));
            unsigned int temp = __builtin_bswap32 (ActualData);
            int pos = 0;

            FrameHeader.sync  = ReadBits(temp, pos, 11); // temp & SYNC_MASK;
            if(FrameHeader.sync != SYNC_MASK)
            {
                qDebug() << "Sync Mask Mismatched";
            }
            pos += 11;

            FrameHeader.MpegVersion = ReadBits(temp, pos, 2);  pos +=2;//(temp & MPEG_VER_MASK) >> 19;
            FrameHeader.LayerDesc = ReadBits(temp, pos, 2);  pos+=2;//(temp & LAYER_MASK)>>17;
            FrameHeader.ProtctionBit = ReadBits(temp, pos, 1); pos+=1;//(temp & PROTECTION_BIT_MASK)>>16;
            FrameHeader.BitRate = ReadBits(temp, pos, 4); pos+=4;//(temp & BIT_RATE_MASK) >> 12;
            FrameHeader.sampleRate = ReadBits(temp, pos, 2); pos+=2; //(temp & SAMPLE_RATE_MASK)>>10;
            FrameHeader.PaddingBit = ReadBits(temp, pos, 1); pos+=1;// (temp & PADDING_BIT_MASK)>>9;
            FrameHeader.PrivateBit = ReadBits(temp, pos, 1); pos+=1;//(temp & PRIV_BIT_MASK)>>8;
            FrameHeader.ChannelMode = ReadBits(temp, pos, 2); pos+=2;//(temp & CHANNEL_MODE_MASK)>>6;
            FrameHeader.Extension = ReadBits(temp, pos, 2); pos+=2;//(temp & EXTENSION_MASK) >> 4;
            FrameHeader.CopyRight = ReadBits(temp, pos, 1); pos+=1;//(temp & COPY_RIGHT_MASK) >> 3;
            FrameHeader.Original = ReadBits(temp, pos, 1); pos+=1;//(temp & ORIGINAL_MASK) >> 2;
            FrameHeader.Emphasis = ReadBits(temp, pos, 1); pos+=1;//(temp & EMPHASIS_MASK);

            format->AudioFormat = COMPRESSION_MPEG3;

            int mpegLayerIndex = 0;
            int mpegVersionIndex = 0;

            if(FrameHeader.MpegVersion == 0)    // MPEG 2.5
                mpegVersionIndex = 2;
            else
            if(FrameHeader.MpegVersion == 2)    // 2.0
                mpegVersionIndex = 1;
            else
            if(FrameHeader.MpegVersion == 3)    // 1.0
                mpegVersionIndex = 0;

            mpegLayerIndex = 3 - FrameHeader.LayerDesc;

            format->SampleRate = SampleRateMap[mpegVersionIndex][FrameHeader.sampleRate];
            format->NumChannels = ChannelMode[FrameHeader.ChannelMode];

            if(mpegVersionIndex == 0 && mpegLayerIndex != 3) //MPEG V 1
               format->BitRate = BitRateTable[mpegLayerIndex][FrameHeader.BitRate];
            else
            {
            if(mpegVersionIndex == 1 || mpegVersionIndex == 2) //MPEG V 2 and 2.5
            {
                int index = mpegLayerIndex;

                if(mpegLayerIndex==1 || mpegLayerIndex==2)
                    index = 4;
                else
                    index = 3;

               format->BitRate = BitRateTable[index][FrameHeader.BitRate];
            }
            }

            format->BytesPerSample = format->BitRate / 8 / 1000;

            streampos fileSavepos = File->tellg();
            File->seekg(0, ios::end);
            int end = File->tellg();
            format->AudioLength = (float) (end - ID3FooterLength - ID3HeaderLength)/(format->BitRate*8);
            Title+= " (" + QString().setNum(format->AudioLength) + " min)";

            qDebug()<< "Audio Length = " << format->AudioLength;
            File->seekg(fileSavepos-4,ios::beg);


        }

        *fptr = File;
        Q_UNUSED(format);
    }
    else
    {
        err = "FILE Open error";
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void MP3Reader::SkipID3()
{
    ID3V2_HEADER Id3Header;
    ID3V2_FRAME_HEADER frameHeader;

    // Read Header
    File->read((char *)&Id3Header, sizeof(Id3Header));

    if(Id3Header.Id[0]=='I' &&
            Id3Header.Id[1] == 'D' &&
            Id3Header.Id[2] == '3')
    {
        qDebug() << "Reading ID3 Frames";
        while(1)
        {
            // Read Frame Header
            File->read((char *)&frameHeader, sizeof(frameHeader));
            int size = __builtin_bswap32(frameHeader.Size);

            QString Id3Tag(frameHeader.Frameid);
            if(size>0)
            {
                if(Id3Tag == "TALB")
                {
                    char album[size];
                    memset(album, 0, size);
                    File->read(album, 1);
                    File->read(album, size-1);
                    AlbumName = QString(album);
                }
                else
                if(Id3Tag == "TIT2")
                {
                    char title[size];
                    memset(title, 0, size);
                    File->read(title, 1);
                    File->read(title, size-1);
                    Title = QString(title);
                }
                else
                {
                    File->seekg(size,ios::cur);
                }
            }
            else
                break;
        }
    }else

        qDebug() << "No ID3 Tags Found";
}

/////////////////////////////////////////////////////////////////////////////////////////////////
bool MP3Reader::FindMP3Frame()
{
  bool found=false;
  unsigned char testByte1 = 0, testByte2=0;
  const unsigned char MASK = 0xE0;

  SkipID3();

  while(!found)
  {
      File->read((char *)&testByte1, sizeof(testByte1));

      if(testByte1 == 0xFF)
      {
          File->read((char *)&testByte2, sizeof(testByte2));
          if(((testByte2 & MASK) == MASK) && testByte2!=0xFF)
          {
              found = true;
              File->seekg(-2,ios::cur);
          }
      }

      if(File->eof()) break;
  }

  return found;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//How much room does ID3 version 1 tag info
//take up at the end of this file (if any)?
int MP3Reader::GetID3FooterLength()
{
   streampos savePos = File->tellg();

   //get to 128 bytes from file end
   File->seekg(0, ios::end);
   streampos length = File->tellg() - (streampos)128;
   File->seekg(length);

   int size;
   char buffer[3] = {0};
   File->read(buffer, 3);
   if( buffer[0] == 'T' && buffer[1] == 'A' && buffer[2] == 'G' )
     size = 128; //found tag data
   else
     size = 0; //nothing there

   File->seekg(savePos);

   return size;

}

