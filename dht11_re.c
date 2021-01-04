#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#define MAX_TIMINGS 85
#define DHT_PIN     12

#ifndef CONSUMER
#define CONSUMER    "Consumer"
#endif

struct gpiod_chip *chip;
struct gpiod_line *line;

uint64_t tmicsec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t time_mic_sec =( 1000000 * tv.tv_sec) + tv.tv_usec;
    //printf("time = %ld\n", time_mic_sec);
    return time_mic_sec;
}

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
    gpio_set_output("gpiochip0", DHT_PIN);
    write_value(0);
    usleep(18000);
    write_value(1);
    
    gpiod_line_release(line);    
    
    gpio_set_input("gpiochip0", DHT_PIN);
    int i,data, k;
    uint8_t buf[40];
    unsigned long t,t5;
    //usleep(1000);
    printf("Entering loop\n");
    for (i = 0; i < 40; i++)
    {
	//if(i == 0) continue;
	printf("b");
	data = 0;
        while(data != 1)
	{
		data = read_value();
		printf("");
	}

        t =(unsigned long) tmicsec();
        while(data != 0){
		data = read_value();
	}
        t5 = (unsigned long) tmicsec();     
        buf[i] = ((t5-t)>50)?1:0;
        printf("%d ",buf[i]);
    }
    
    uint8_t RHH = 0, RHL = 0, TH = 0, TL = 0, CheckSum = 0;
    int j;
    for ( j = 0; j < 8; j++)
    {
        RHH = (RHH<<1) | buf[j];
        RHL = (RHL<<1) | buf[j+8];
        TH = (TH<<1) | buf[j+16];
        TL = (TL<<1) | buf[j+24];
        CheckSum = (CheckSum<<1) | buf[j+32];
    }
    printf("\nHumidity = %d.%d %%\n",RHH,RHL);
    printf("Temperature = %d.%d\n",TH,TL);
}

int main()
{
    getDHT11Data();
}
