#include <decoder.hpp>
#include <GPIO_0.hpp>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "tasks.hpp"
#include "utilities.h"
#include "LPC17xx.h"
#include "FreeRTOS.h"
#include "printf_lib.h"
#include "LabGPIOInterrupts.hpp"
#include "adc0.h"
#include "semphr.h"

SemaphoreHandle_t SPI_Bus; // Semaphore to protect SPI transfer
Decoder MP3Player;                     // VS1053 Driver
int elapsed;
char *song_title;
SoftTimer debouncer(200);
uint8_t currentVolume;


/************* UART INIT *********/
bool Init_UART(void)
{
    // Init PINSEL, baud rate, frame size, etc.

    /*
     * UART Baud: PCLK / (16 * (256 * unDLM + unDLL) )
     */

         LPC_SC->PCONP |= (1<<24);

         LPC_SC->PCLKSEL1 &= ~(0x3<<16);
         LPC_SC->PCLKSEL1 |= (0x1<<16);

         LPC_UART2->FCR &= ~(0xF<<0);
         LPC_UART2->FCR &= ~(0x3<<6);
         LPC_UART2->FCR |= (0x1<<0); //Enable UART FIFO

         LPC_PINCON->PINSEL4 &= ~(0xF<<16);
         LPC_PINCON->PINSEL4 |= (0xA<<16); // Set GPIO p2.8 (Tx) and p2.9 (Rx)

         uint32_t dl = 0;

         dl = (sys_get_cpu_clock())/(16*9600); //Calculate dl for baud rate
                                                 // Buad rate is 9600

         LPC_UART2->LCR &= ~(0x3<<0); //Reset two bits
         LPC_UART2->LCR |= (1<<7) | (0x3<<0); // Set frame to 8-bit char length and DLAB = 1

         LPC_UART2->DLM = (dl>>8) & 0xFF; //
         LPC_UART2->DLL = (dl>>0) & 0xFF;

         LPC_UART2->LCR &= ~(1<<7);

         return true;

}

/************* LCD INIT *********/
void LCD_Display_On(void)
{
    uint8_t x = 17;

        //if(LPC_UART2->LSR & (1<<5))
        //{
           LPC_UART2->THR = x;
        //}

}

/************* Send Song's Name to LCD *********/
void Song_Name(const char *song)
{
    int men_len = strlen(song);
    LPC_UART2->THR = 12;
    //if(LPC_UART2->LSR & (1<<5))
    //{
        for(int i = 0; i < men_len; i++)
            {

            LPC_UART2->THR = song[i];

            }
    //}

}




void PauseAndResume()
//(P0.1 Not Current) P1.9 Pauses and Resumes
{
    printf("\n PauseAndResume Called\n");
  if (MP3Player.isPlaying())
  {
    MP3Player.pause();
  }
  else
  {
    MP3Player.play();
  }
}

void IncreaseVolume()
{
        //uint16_t reading = currentVolume;
      //reading = adc0_get_reading(5);
      //uint8_t volume = reading * 254.0 / 4095;

        printf("\nincrease volume");
//      currentVolume=currentVolume-1;
//      MP3Player.setVolume(currentVolume, currentVolume);

              currentVolume=currentVolume-5;
              printf("%i",currentVolume);
              MP3Player.setVolume(currentVolume, currentVolume);


}

void IncreaseVolumeISR()
{
    IncreaseVolume();
}
void DecreaseVolume()
{
    printf("\ndecrease volume");
    currentVolume=currentVolume-.1;
    MP3Player.setVolume(currentVolume, currentVolume);
}

void DecreaseVolumeISR()
{
    DecreaseVolume();
}

//void alienModeISR()
//{
//    alienMode();
//}
//void alienMode()
//{
//    if(MP3Player.alien==false)
//    {
//
//}

void skipSongISR()
{
//P2.4 Skips song
  MP3Player.skipSong();
}

void previousSong()
{

}
void previousSongISR(void *pvParameters)
//P2.7 Previous song
{
    MP3Player.prevSong();
//  while (1)
//  {
//    if (debouncer.expired())
//    {
//      if (prev.getLevel())
//      {
//        MP3Player.prevSong();
//        debouncer.reset();
//      }
//    }
//    vTaskDelay(100);
//  }
}



void setVolume(void *pvParameters)
{
  while (1)
  {
    if (xSemaphoreTake(SPI_Bus, portMAX_DELAY))
    {
      uint16_t reading = 1;
      //reading = adc0_get_reading(5);
      uint8_t volume = reading * 254.0 / 4095;
      //currentVolume=volume;
      //MP3Player.setVolume(volume, volume);
      xSemaphoreGive(SPI_Bus);
    }
    vTaskDelay(500);
  }
}

void playSong(void *pvParameters)

{
  while (1)
  {

    FIL file;
    char title[100];
    song_title = MP3Player.getCurrentSongName();
    strcpy(title, "1:");
    strcat(title, MP3Player.getCurrentSongName());

    f_open(&file, title, FA_OPEN_EXISTING | FA_READ);

    Song_Name(title);//send you UART

    int bufferSize = 40;  //buffer size of 40 because song sounded cut up if played at lower buffer sizes
    int buffer_ofs = 0;
    unsigned char buffer[bufferSize] = {};

    unsigned int file_size = f_size(&file);
    unsigned int bytes_read;

    while (buffer_ofs < file_size && !MP3Player.getNextFlag() && !MP3Player.getPreviousFlag())
    {
      if (xSemaphoreTake(SPI_Bus, portMAX_DELAY))
      {
        if (MP3Player.isPlaying())
        {
          int buffer_pos = 0;
          unsigned char *p;
          f_read(&file, buffer, bufferSize, &bytes_read);
          p = buffer;
          while (buffer_pos < bufferSize)
          {
            while (!MP3Player.waitForDREQ())
              ;
            MP3Player.setXDCSLow();
            ssp0_exchange_byte(*p++);
            buffer_pos++;
          }
          MP3Player.setXDCSHigh();
          buffer_ofs += bufferSize;
        }
        elapsed = ((float)buffer_ofs / file_size) * 100;
        xSemaphoreGive(SPI_Bus);
      }
      vTaskDelay(1);
    }
    f_close(&file);
    if (MP3Player.getPreviousFlag())
    {
      MP3Player.decSong();
    }
    else
    {
      MP3Player.nextSong();
    }
    MP3Player.clearFlags();
    MP3Player.play();
  }
}

int main(void)
{
  // Create SPI Mutex
  SPI_Bus = xSemaphoreCreateMutex();

  // Initialize MP3 Decoder
  GPIO_0 rst(0, 29);
  GPIO_0 xdcs(1, 19);
  GPIO_0 cs(0, 30);
  GPIO_0 dreq(1, 20);
 currentVolume = 20;
  bool success_UART_Init=false;
     success_UART_Init = Init_UART();

     // Initiate Screen after UART init success
     if(success_UART_Init)
     {
         LCD_Display_On();
         printf("\nin Uart Init");
     }

  bool mp3_bool = MP3Player.init(&cs, &dreq, &rst, &xdcs);
  //Check That UART init success



  // Interrupt for Play/Pause/Skip/Previous
  LabGPIOInterrupts *x = LabGPIOInterrupts::getInstance();

  x->init();

  bool attach;
  InterruptCondition_E int1 = rising;
  attach = x->attachInterruptHandler(0, 1, &PauseAndResume, int1);
  attach = x->attachInterruptHandler(2, 6, &IncreaseVolumeISR, int1);
  //attach = x->attachInterruptHandler(2, 7, &DecreaseVolumeISR, int1);
  attach = x->attachInterruptHandler(2, 0, &skipSongISR, int1);
  //attach = x->attachInterruptHandler(2, 5, &previousSongISR, int1);

  FIL file;
  f_open(&file, "1:REZZ", FA_OPEN_EXISTING | FA_READ);

  // Create tasks for scheduler
  scheduler_add_task(new terminalTask(PRIORITY_HIGH));

  //This causes a crash loop, probably because the of the gpio pins associated with them
  //xTaskCreate(prevSongISR, "Prev Song", 1024, (void *)1, 2, NULL);
  xTaskCreate(playSong, "Play Song", 1024, (void *)1, 3, NULL);
  //xTaskCreate(setVolume, "Volume", 1024, (void *)1, 1, NULL);

  scheduler_start();

  return 0;
}
