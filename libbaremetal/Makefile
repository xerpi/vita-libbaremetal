TARGET	= libbaremetal
SRCDIR	= src
INCDIR	= include
MBEDTLS	= mbedtls
OBJS	= libc.o utils.o pervasive.o cdram.o uart.o gpio.o i2c.o \
	  iftu.o dsi.o display.o spi.o syscon.o hdmi.o oled.o ctrl.o \
	  sysroot.o msif.o draw.o font.o tiny-printf.o font_data.o

PREFIX	= arm-vita-eabi
CC	= $(PREFIX)-gcc
AR	= $(PREFIX)-ar
CFLAGS	= -I$(INCDIR) -I$(MBEDTLS)/include -mcpu=cortex-a9 -mthumb-interwork \
	  -O0 -Wall -Wno-unused-const-variable
ASFLAGS	=
DEPS	= $(OBJS:.o=.d)
INSTDIR = $(VITASDK)/$(PREFIX)

all: $(TARGET).a

$(TARGET).a: $(OBJS) $(MBEDTLS)/library/libmbedcrypto.a
	$(AR) rcsv $@ $(OBJS) $(wildcard $(MBEDTLS)/library/*.o)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: $(SRCDIR)/%.s
	$(CC) $(ASFLAGS) -MMD -MP -c $< -o $@

$(MBEDTLS)/library/libmbedcrypto.a:
	@$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" -C $(MBEDTLS)/library libmbedcrypto.a

.PHONY: clean install

clean:
	@rm -f $(TARGET).a $(OBJS) $(DEPS)
	@$(MAKE) -C $(MBEDTLS)/library clean

install: $(TARGET).a
	cp $(TARGET).a $(INSTDIR)/lib
	cp -r $(INCDIR)/. $(INSTDIR)/include/baremetal
	@echo "Installed to $(INSTDIR)"

-include $(DEPS)
