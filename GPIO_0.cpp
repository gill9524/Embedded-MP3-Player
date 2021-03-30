#include <GPIO_0.hpp>

GPIO_0::GPIO_0(uint8_t port, uint8_t pin)
{
  portNumber = port;
  pinNumber = pin;
  switch (port)
  {
  case 0:
    LPC_GPIO = LPC_GPIO0;
    break;
  case 1:
    LPC_GPIO = LPC_GPIO1;
    break;
  case 2:
    LPC_GPIO = LPC_GPIO2;
    break;
  }
}

void GPIO_0::setAsInput()
{
  LPC_GPIO->FIODIR &= ~(1 << pinNumber);
}

void GPIO_0::setAsOutput()
{
  LPC_GPIO->FIODIR |= (1 << pinNumber);
}

void GPIO_0::setDirection(bool output)
{
    if(output) setAsOutput();
    else setAsInput();
}

void GPIO_0::setHigh()
{
  LPC_GPIO->FIOSET = (1 << pinNumber);
}

void GPIO_0::setLow()
{
  LPC_GPIO->FIOCLR = (1 << pinNumber);
}

void GPIO_0::set(bool high)
{
    if(high) setHigh();
    else setLow();
}

bool GPIO_0::getLevel()
{
  return LPC_GPIO->FIOPIN & (1 << pinNumber);
}

GPIO_0::~GPIO_0()
{
  setAsInput();
}
