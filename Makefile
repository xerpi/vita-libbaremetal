TARGET	= baremetal-sample
SRCDIR	= src
INCDIR	= include
MBEDTLS	= mbedtls
OBJS	= start.o main.o libc.o utils.o pervasive.o cdram.o uart.o gpio.o \
	  i2c.o iftu.o dsi.o display.o spi.o syscon.o hdmi.o oled.o ctrl.o \
	  sysroot.o msif.o draw.o font.o tiny-printf.o font_data.o

PREFIX	= arm-vita-eabi
CC	= $(PREFIX)-gcc
AS	= $(PREFIX)-as
OBJCOPY	= $(PREFIX)-objcopy
CFLAGS	= -I$(INCDIR) -I$(MBEDTLS)/include -mcpu=cortex-a9 -mthumb-interwork \
	  -O0 -Wall -Wno-unused-const-variable -Wno-main
LDFLAGS	= -T linker.ld -nostartfiles -nostdlib -lgcc -lm
LDFLAGS	+= -L$(MBEDTLS)/library -lmbedcrypto
ASFLAGS	=
DEPS	= $(OBJS:.o=.d)

all: $(TARGET).bin

%.bin: %.elf
	$(OBJCOPY) -S -O binary $^ $@

$(TARGET).elf: $(OBJS) $(MBEDTLS)/library/libmbedcrypto.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
%.o: $(SRCDIR)/%.s
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@

$(MBEDTLS)/library/libmbedcrypto.a:
	@$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" -C $(MBEDTLS)/library libmbedcrypto.a

.PHONY: clean send

clean:
	@rm -f $(TARGET).bin $(TARGET).elf $(OBJS) $(DEPS)
	@$(MAKE) -C $(MBEDTLS)/library clean

send: $(TARGET).bin
	curl -T $(TARGET).bin ftp://$(PSVITAIP):1337/ux0:/baremetal/payload.bin
	@echo "Sent."

-include $(DEPS)
