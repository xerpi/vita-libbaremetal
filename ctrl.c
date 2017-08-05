#include <stddef.h>
#include "ctrl.h"
#include "syscon.h"
#include "libc.h"
#include "utils.h"

void ctrl_read(unsigned int *data)
{
	syscon_ctrl_read(data);
}
