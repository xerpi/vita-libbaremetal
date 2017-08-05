#ifndef CTRL_H
#define CTRL_H

#define CTRL_BUTTON_HELD(ctrl, button) !((ctrl) & (button))

#define CTRL_PSBUTTON (1 << 14)

void ctrl_read(unsigned int *data);

#endif
