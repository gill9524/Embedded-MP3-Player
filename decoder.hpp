#ifndef VS1053_H
#define VS1053_H

#include <GPIO_0.hpp>
#include "LPC17xx.h"
#include "printf_lib.h"
#include "utilities.h"
#include "ssp0.h"
#include "ff.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>



typedef enum{
    MODE            = 0x00,
    STATUS          = 0x01,
    BASS            = 0x02,
    CLOCKF          = 0x03,
    DECODE_TIME     = 0x04,
    AUDATA          = 0x05,
    WRAM            = 0X06,
    WRAMADDR        = 0X07,
    HDAT0           = 0X08,
    HDAT1           = 0X09,
    VOL             = 0X0B,
    AICTRL0         = 0xC,
    AICTRL1         = 0xD,
    AICTRL2         = 0xE,
    AICTRL3         = 0xF
}SCI_T;

class Decoder
{
private:
  GPIO_0 *CS;
  GPIO_0 *DREQ;
  GPIO_0 *RST;
  GPIO_0 *XDCS;
  bool playing;        // bool to check if the decoder is playing or not
  bool alien;
  uint8_t volume;      // Volume amount
  int numberOfSongs;    // Songs in SD Card
  char *songs[100];    // Song titles retrieved
  int currentSongPlaying;    // Current song playing position
  int maxSongs;       // Maximum number of songs to play
  bool skipToNextSongFlag; // Flag for next song
  bool previousSongFlag; // flag for previous song


public:
  bool init(GPIO_0 *cs, GPIO_0 *dreq, GPIO_0 *rst, GPIO_0 *xdcs);
  void getSongs();
  void sciWrite(uint8_t reg_addr, uint16_t value);      //Write to decoder registers
  uint16_t sciRead(uint8_t reg_addr);                   //Read from decoder registers
  bool waitForDREQ();                                  //Wait for DREQ
  void setVolume(uint8_t left, uint8_t right);        //Set the volume
  int getVolume();                                      //Return the volume
  void setXDCSHigh();
  void setXDCSLow();
  void transferSDI(uint8_t *data, uint32_t size);          //Transfer data
  void pause();
  void play();
  bool isPlaying();
  bool isAlien();
  void nextSong();
  void decSong();
  void skipSong();
  void prevSong();
  bool getNextFlag();
  bool getPreviousFlag();
  void clearFlags();
  char *getCurrentSongName();
  Decoder();
};

#endif
