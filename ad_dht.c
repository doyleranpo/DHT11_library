//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
// Access from ARM Running Linux
#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <bcm2835.h>
#include <unistd.h>
#define MAXTIMINGS 100
 
#define DHT11 11
 
int readDHT(int type, int pin);
 
int main(int argc, char **argv)
{
  if (!bcm2835_init())
        return 1;
 
  if (argc != 3) {
        printf("usage: %s [11] GPIOpin#\n", argv[0]);
        printf("example: %s 11 4 - Read from an DHT11 connected to GPIO #4\n", argv[0]);
        return 2;
  }
  int type = 0;
  if (strcmp(argv[1], "11") == 0) type = DHT11;
  if (type == 0) {
        printf("Select 11 as type!\n");
        return 3;
  }
  int dhtpin = atoi(argv[2]);
  if (dhtpin <= 0) {
        printf("Please select a valid GPIO pin #\n");
        return 3;
  }
  readDHT(type, dhtpin);
  return 0;
} // main
 
int bits[250], data[100];
int bitidx = 0;
 
int readDHT(int type, int pin) {
  int counter = 0;
  int laststate = HIGH;
  int j=0;
 
  // Set GPIO pin to output
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
 
  bcm2835_gpio_write(pin, HIGH);
  usleep(500000);  // 500 ms
  bcm2835_gpio_write(pin, LOW);
  usleep(20000);
 
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
 
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;
 
  // wait for pin to drop?
  while (bcm2835_gpio_lev(pin) == 1) {
    usleep(1);
  }
 
  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(pin) == laststate) {
        counter++;
        //nanosleep(1);         // overclocking might change this?
        if (counter == 1000)
          break;
    }
    laststate = bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;
 
    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200)
        data[j/8] |= 1;
      j++;
    }
  }
 
  if ((j >= 39) &&
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
     if (type == DHT11)
        printf("temp:%d hum:%d\n", data[2], data[0]);
    return 1;
  }
 
  return 0;
}