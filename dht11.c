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

int data[5] = { 0, 0, 0, 0, 0 };

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
    
void read_dht()
{
    uint8_t laststate = 1;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    
    gpio_set_output("gpiochip0", DHT_PIN);
    write_value(0);
    gpiod_line_release(line);
    usleep(18000);
    
    gpio_set_input("gpiochip0", DHT_PIN);
     
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( read_value() == laststate )
        {
	    //printf("1\n");
            counter++;
            usleep(1000);
            if ( counter == 255 )
                break;
        }
        laststate = read_value();
        printf("2\n");
        if (counter == 255)
            break;
           
        if ( (i >= 4) && (i % 2 == 0) )
        {
            printf("i = %d\n",i);
	    data[j / 8] <<= 1;
            if (counter > 50)
                data[j / 8] |= 1;
            j++;
        }
    }
    printf("i = %d j = %d\n",i,j); 
    if ((j>=40) && (data[4] == ( ( data[0] + data[1] + data[2] + data[3]) & 0xFF)))
    {
        printf("Humidity = %d/%d %% Temperature = %d.%d C\n",data[0],data[1],data[2],data[3]);
    }
    else
        printf("Data not good\n");
}

int main()
{
    read_dht();
    
    return 0;
}
