#include <decoder.hpp>


Decoder::Decoder()
{
}

bool Decoder::init(GPIO_0 *cs, GPIO_0 *dreq, GPIO_0 *rst, GPIO_0 *xdcs)
{
  numberOfSongs = 0;
  currentSongPlaying = 0;
  getSongs();
  maxSongs = numberOfSongs;

  CS = cs; CS->setAsOutput(); CS->setHigh();
  RST = rst; RST->setAsOutput(); RST->setLow();
  DREQ = dreq; DREQ->setAsInput();
  XDCS = xdcs; XDCS->setAsOutput(); XDCS->setHigh();

  ssp0_init(32);        //SSP0, SPI
  RST->setHigh();       //Reset
  delay_ms(50);         //Set delay

  const uint16_t mode_default_state   = 0x4800;
  const uint16_t clock_default_state  = 0x6000;
  const uint16_t volume_default_state = 0x2020;

  //Write Decoder registers
  sciWrite(MODE, (1 << 5) | 0x0810);
  sciWrite(BASS, 0x0000);
  sciWrite(AUDATA, 0xAC45);
  sciWrite(CLOCKF, 0x6000);
  sciWrite(VOL,0x0000);

  //Set volume

  //Reset flags
  skipToNextSongFlag = false;
  previousSongFlag = false;
  playing = true;

  return (sciRead(STATUS) >> 4 & 0x04);
}

//This doesn't get the correct files from sd card, supposed to be dynamic
void Decoder::getSongs()
{
  DIR Dir;
  FILINFO Finfo;
  FRESULT returnCode;
  char Lfname[128];
  // Open directory
  f_opendir(&Dir, "1:");
  printf("\n Get Songs");
  //for (int i=0;i<3;i++)
  for(;;)
  {
    Finfo.lfname = Lfname;
    Finfo.lfsize = sizeof(Lfname);

    // Read directory contents
    returnCode = f_readdir(&Dir, &Finfo);
    printf("\n file info: %s",Finfo.fname);
    if ((FR_OK != returnCode) || !Finfo.fname[0])
    {

      break;
    }
    // If filename ends with .mp3, store song title in an array
    if (strstr(Finfo.lfname, ".mp3") || strstr(Finfo.lfname, ".MP3"))
    {
      int len = strnlen(Finfo.lfname, 128);
      songs[numberOfSongs] = new char[len + 1];
      strcpy(songs[numberOfSongs], Finfo.fname);
      //printf("\n %c",songs[numberOfSongs]);
      numberOfSongs++;
    }
  }


//  for(int i=0;i<numberOfSongs;i++)
//  {
//
//      //printf("\n 1%s ",songs[numberOfSongs]);
//      //printf("\n\n 2%s ",songs[1]);
//
//      //printf("\n\n 3%s ",songs[2]);
//      //printf("\n\n 4%c ",songs[4]);
//      //printf("\n\n 5%c ",songs[5]);
//      //printf("\n\n %s ",songs[2]);
//  }
}


void Decoder::sciWrite(uint8_t reg_addr, uint16_t value)
{
  while (!waitForDREQ());
  CS->setLow();
  //Write
  ssp0_exchange_byte(0x02);     //write opcode
  ssp0_exchange_byte(reg_addr);  //write address
  ssp0_exchange_byte(value >> 8);  //shift right 8 to send next 8
  ssp0_exchange_byte(value & 0xff);  //wr

  CS->setHigh();
}

uint16_t Decoder::sciRead(uint8_t reg_addr)
{
  uint16_t data;
  while (!waitForDREQ());
  CS->setLow();                     //Update CS
  //Read
  ssp0_exchange_byte(0x03);         //read opcode
  ssp0_exchange_byte(reg_addr);
  delay_ms(10);
  data = ssp0_exchange_byte(0x00);
  data <<= 8;
  data |= ssp0_exchange_byte(0x00);

  CS->setHigh();                    //Update CS
  return data;
}

bool Decoder::waitForDREQ()
{
  return DREQ->getLevel();
}

/*
 * First 8 bits for volume register control the left channel
 * Second 8 bits control the right channel
 */
void Decoder::setVolume(uint8_t left, uint8_t right)
{
  volume = left;
  uint16_t temp;
  temp = left;
  temp <<= 8;
  temp |= right;
  sciWrite(VOL, temp);         //Write the volume register
}

int Decoder::getVolume()
{
  return volume;
}

void Decoder::setXDCSHigh()
{
  XDCS->setHigh();
}

void Decoder::setXDCSLow()
{
  XDCS->setLow();
}

void Decoder::transferSDI(uint8_t *data, uint32_t size)
{
  while (!waitForDREQ());
  XDCS->setLow();
  for (int i = 0; i < size; i++)
  {
    while (!waitForDREQ());
    ssp0_exchange_byte(data[i]);
  }
  if (size < 32)
    for (int i = 0; i < (32 - size); i++)
      ssp0_exchange_byte(0x00);
  XDCS->setHigh();
}


void Decoder::pause()
{
  playing = false;
}

void Decoder::play()
{
  playing = true;
}

bool Decoder::isPlaying()
{
  return playing;
}

void Decoder::nextSong()
{

    currentSongPlaying++;
    if (currentSongPlaying >= maxSongs)
    {
      currentSongPlaying = 0;
    }

}

void Decoder::decSong()
{
  if (currentSongPlaying != 0)
  {
    currentSongPlaying--;
  }
  else
  {
    currentSongPlaying = maxSongs - 1;
  }
}

void Decoder::skipSong()
{
  skipToNextSongFlag = true;
}

void Decoder::prevSong()
{
  previousSongFlag = true;
}

bool Decoder::getNextFlag()
{
  return skipToNextSongFlag;
}

bool Decoder::getPreviousFlag()
{
  return previousSongFlag;
}

void Decoder::clearFlags()
{
  previousSongFlag = false;
  skipToNextSongFlag = false;
}

char *Decoder::getCurrentSongName()
{
    //printf("\nnum:%s song:",songs);
  return songs[currentSongPlaying];
}

