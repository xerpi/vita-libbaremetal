#ifndef DSI_H
#define DSI_H

void dsi_init(void);
int dsi_get_dimensions_for_vic(unsigned int vic, unsigned int *width, unsigned int *height);
void dsi_enable_bus(int bus, unsigned int vic);
void dsi_unk(int bus, unsigned int vic, unsigned int unk);

#endif
