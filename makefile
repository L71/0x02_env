
### Project specs
PROJNAME = 0x02


CPUTYPE = attiny84
AVRDUDECPUTYPE = t84

# AVR CPU Fuse settings for this project
# http://www.engbedded.com/fusecalc
# int 8MHz clock, no prescaler
CPUFUSEFLAGS = -B 10 -U lfuse:w:0xe2:m -U hfuse:w:0xd4:m -U efuse:w:0xff:m


# final .elf file name etc.
ELF = $(PROJNAME).elf
HEX = $(PROJNAME).hex

# object file list
OBJS = main.o pwm_timer.o adc.o

include makefile.avrproj


