#ifndef LABGPIO_H
#define LABGPIO_H

#include "LPC17xx.h"
#include <stdio.h>
#include <cassert>

class GPIO_0
{
  private:
    uint8_t portNumber;
    uint8_t pinNumber;
    LPC_GPIO_TypeDef *LPC_GPIO;

  public:
    GPIO_0(uint8_t port, uint8_t pin);      //Constructor
    void setAsInput();                      //Sset as input
    void setAsOutput();                     //Set as output
    void setDirection(bool output);         //Set direction, true = output, false = input
    void setHigh();                         //Set as high
    void setLow();                          //Set as low
    void set(bool high);                    //Set pins , true = high, false = low
    bool getLevel();                        //Return the level
    ~GPIO_0();                              //Deconstructor
};

#endif
