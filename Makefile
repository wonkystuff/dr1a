DEVICE     = attiny85
CLOCK      = 8000000
COMPILE    = avr-gcc -save-temps=obj -Wall -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)
OBJS       = main.o calc.o isr.o
OUTNAME    := $(notdir $(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST))))))

ifdef RESET_ACTIVE
EXTRA_FLAGS = -DRESET_ACTIVE
else
EXTRA_FLAGS =
endif

all: $(OUTNAME).bin Makefile

$(OUTNAME).bin: $(OUTNAME).elf
	rm -f $@ $(basename $@).eep
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0  $^ $(basename $@).eep
	avr-objcopy -O binary -R .eeprom  $^ $@

$(OUTNAME).hex: $(OUTNAME).elf
	rm -f $@
	avr-objcopy -j .text -j .data -O ihex $^ $@

clean:
	rm -f calc.c calc.h
	rm -f *.o *.s *.i
	rm -f *.elf *.bin *.hex *.map *.eep

$(OUTNAME).elf: $(OBJS)
	$(COMPILE) -Wall -Wextra -O2  -Wl,--gc-sections -Wl,-Map,$(basename $@).map -o $@ $^
	avr-size $@ -C --mcu=$(DEVICE)

main.c: calc.h
calc.c: calc.h

isr.o: isr.c calc.h
	$(COMPILE) -O3 -c $< -o $@

calc.c calc.h: calc.rb
	ruby $^

flashcode: $(OUTNAME).bin
	minipro -c code -s -p $(DEVICE) -w $^

flashfuse: fuse.txt
	minipro -c config -p $(DEVICE) -w $^

%.o : %.c
	$(COMPILE) $(EXTRA_FLAGS) -O2 -c $< -o $@
