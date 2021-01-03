#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMINGS 85
#define DHT_PIN     12

#ifndef CONSUMER
#define CONSUMER    "Consumer"
#endif

struct gpiod_chip *chip;
struct gpiod_line *line;

int gpio_set_output( char *chipname, unsigned int line_num)
{
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip)
    {
	perror("Chip fail");
        return 0;
    }
    
    line = gpiod_chip_get_line(chip, line_num);
    if (!line)
    {
	perror("Line fail");
        gpiod_chip_close(chip);
    }

    if (gpiod_line_request_output(line, CONSUMER, 0) < 0)
	{
		perror("Set mode fail");
        gpiod_line_release(line);
	}
}

int gpio_set_input( char *chipname, unsigned int line_num)
{
    chip = gpiod_chip_open_by_name(chipname);
    if (!chip)
    { 
	perror("Open chip failed");   
        return 0;
    }
    
    line = gpiod_chip_get_line(chip, line_num);
    if (!line)
    {
	perror("Get line failed");
        gpiod_chip_close(chip);
    }

    if (gpiod_line_request_input(line, CONSUMER) < 0)
    {
	perror("Request line as input failed");
        gpiod_line_release(line);
    }
}

void write_value(int val)
{
    int ret = gpiod_line_set_value(line, val);
    if (ret < 0)
    {
	perror("line issue"); 
        gpiod_line_release(line);
    }
}

int read_value()
{
    int val = gpiod_line_get_value(line);
    if (val < 0 )
    {
	perror("Line issue");
        gpiod_line_release(line);
    }
    return val;
}

void getDHT11Data()
{
    uint8_t data;
    gpio_set_output("gpiochip0", DHT_PIN);
    write_value(0);
    usleep(18000);
    write_value(1);
    
    gpiod_line_release(line);    
    
    gpio_set_input("gpiochip0", DHT_PIN);
    uint8_t buf[40];
    for (int i = 0; i < 40; i++)
    {
        while(read_value() != 0);   
        usleep(50);
    
        while(read_value() != 1);
        usleep(40);
        
        data = read_value();
        if ( data == 1 ) 
            buf[i] = 1;
        else 
            buf[i] = 0;
    }
    
    uint8_t RHH = 0, RHL = 0, TH = 0, TL = 0, CheckSum = 0;
    
    for (int j = 0; j < 8; j++)
    {
        RHH = (RHH<<1) | buf[j];
        RHL = (RHL<<1) | buf[j+8];
        TH = (TH<<1) | buf[j+16];
        TL = (TL<<1) | buf[j+24];
        CheckSum = (CheckSum<<1) | buf[j+32];
    }
    printf("Humidity = %d.%d %%\n",RHH,RHL);
}

int main()
{
    getDHT11Data();
}