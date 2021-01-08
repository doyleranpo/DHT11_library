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

// struct gpiod_chip *chip;
// struct gpiod_line *line;


uint64_t getMicro()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t microsec = (1000000 * tv.tv_sec) + tv.tv_usec;
    return microsec;
}

int request_open_line(struct gpiod_chip* chip, struct gpiod_line* line, char *chipname, unsigned int line_num)
{
    chip = gpiod_chip_open_by_name(chipname);
    if(!chip)
    {
        perror("Open chip failed\n");
    end:
        exit(1);
    }
    
    line = gpiod_chip_get_line(chip, line_num);
    if(!line)
    {
        perror("Getting line failed\n");
        gpiod_chip_close(chip);
        goto end;
    }
    return 0;
}

void gpio_set_input(struct gpiod_chip* chip, struct gpiod_line* line, char *chipname, unsigned int line_num)
{
    int ret;
        ret = gpiod_line_request_input(line, "DHT");
        if (ret < 0)
        {
            perror("Set mode failed\n");
            gpiod_line_release(line);
            gpiod_chip_close(chip);
            exit(1);
        }
    
}

void gpio_set_output(struct gpiod_chip* chip, struct gpiod_line* line, char *chipname, unsigned line_num)
{
    int ret, inf;
        ret = gpiod_line_request_output(line, "DHT", 0);
        if (ret < 0)
        {
            perror("Set mode failed\n");
            gpiod_line_release(line);
            gpiod_chip_close(chip);
            exit(1);
        }
    
}

void write_value(struct gpiod_chip* chip, struct gpiod_line* line, int val)
{
    int ret = gpiod_line_set_value(line, val);
    if (ret < 0)
    {
        perror("Line busy\n");
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        exit(1);
    }
}

int read_value(struct gpiod_chip* chip, struct gpiod_line* line)
{
    int val = gpiod_line_get_value(line);
    if ( val < 0 )
    {
        perror("Line issue\n");
        gpiod_line_release(line);
        gpiod_chip_close(chip);
        exit(1);
    }
}

void getDHTData(struct gpiod_chip* chip, struct gpiod_line* line)
{
    uint64_t t, t5;
    uint8_t buf[40];
    uint8_t RHH = 0, RHL = 0, TH = 0, TL = 0, CheckSum = 0;
    int i,k,data;
    for (int l = 0; l < 40; l++)
    {
        buf[l] = 0;
    }
    gpio_set_output(chip, line, "gpiochip0", DHT_PIN);
    write_value(chip, line, 0);
    usleep(18000);
    write_value(chip, line, 1);
    
    gpiod_line_release(line);
    
    gpio_set_input(chip, line, "gpiochip0", DHT_PIN);
    
    for (i = 0; i < 40; i++)
    {
        data = 0;
        while(data != 1)
        {
            data = read_value(chip, line);
        }
        
        t = getMicro();
        
        while(data != 0)
        {
            data = read_value(chip, line);
        }
        
        t5 = getMicro();
        buf[i] = ((t5-t)>50)?1:0;
        printf("%d ", buf[i]);
    }
    for ( k = 0; k < 8; k++)
    {
        RHH = (RHH<<1) | buf[k];
        RHL = (RHL<<1) | buf[k+8];
        TH = (TH<<1) | buf[k+16];
        TL = (TL<<1) | buf[k+24];
        CheckSum = (CheckSum<<1) | buf[k+32];
    }
    printf("\nHumidity = %d.%d %%\n", RHH, RHL);
    printf("Temperature = %d.%d\n", TH, TL);
}

int main()
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int var = request_open_line(chip, line,"gpiochip0",DHT_PIN);
    if (!var)
    	getDHTData(chip, line);
    return 0;
}
