TARGET   = baremetal-sample
OBJS = start.o main.o lowio.o uart.o spi.o syscon.o hdmi.o libc.o

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
AS      = $(PREFIX)-as
OBJCOPY = $(PREFIX)-objcopy
CFLAGS  = -Wall -O0 -mcpu=cortex-a9 -mthumb-interwork
LDFLAGS = -T linker.ld -nostartfiles -nostdlib -lgcc
ASFLAGS =

all: $(TARGET).bin

%.bin: %.elf
	$(OBJCOPY) -S -O binary $^ $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean send

clean:
	@rm -f $(TARGET).bin $(TARGET).elf $(OBJS)

send: $(TARGET).bin
	curl -T $(TARGET).bin ftp://$(PSVITAIP):1337/ux0:/baremetal/payload.bin
	@echo "Sent."
