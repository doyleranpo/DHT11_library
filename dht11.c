#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_TIMINGS 85
#define DHT_PIN     27

#ifndef CONSUMER
#define CONSUMER    "Consumer"
#endif

struct gpiod_chip *chip;
struct gpiod_line *line;

int data[5] = { 0, 0, 0, 0, 0 };

int gpio_set_output( char *chipname, unsigned int line_num, unsigned int val)
{
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip)
        return 0;
    
    line = gpiod_chip_get_line(chip, line_num);
    if (!line)
        gpiod_chip_close(chip);
        
    if (gpiod_line_request_output(line, CONSUMER, 0) < 0)
        gpiod_line_release(line);
}

int gpio_set_input( cahr *chipname, unsigned int line_num, unsigned int val)
{
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip)
        return 0;
    
    line = gpiod_chip_get_line(chip, line_num);
    if (!line)
        gpiod_chip_close(chip);
        
    if (gpiod_line_request_input(line, CONSUMER) < 0)
        gpiod_line_release(line);
}

void write_value(int val)
{
    int ret = gpiod_line_set_value(line, val);
    if (ret < 0) 
        gpiod_line_release(line)
}

int read_value()
{
    int val = gpiod_line_get_value(line)
    if (val < 0)
        gpiod_line_release(line)
    return val;
}
    
void read_dht()
{
    uint8_t laststate = HIGH
    uint8_t counter = 0;
    uint8_t j = 0, i;
    
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    