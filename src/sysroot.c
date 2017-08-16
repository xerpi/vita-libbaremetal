#include "sysroot.h"

static const struct sysroot_buffer *sysroot;

void sysroot_init(const struct sysroot_buffer *sysroot_buffer)
{
	sysroot = sysroot_buffer;
}

unsigned int sysroot_get_hw_info(void)
{
	return sysroot->hw_info;
}

int sysroot_model_is_vita(void)
{
	const unsigned short device_type = __builtin_bswap16(sysroot->device_type);

	if ((device_type - 0x100 > 0x11) || sysroot_model_is_dolce()) {
		const unsigned int hw_info_mask = sysroot->hw_info & 0xFF0000;

		if (!sysroot_model_is_unk())
			return 0;

		if ((hw_info_mask == 0x700000) || (hw_info_mask == 0x720000) || (hw_info_mask == 0x510000))
			return 0;
	}

	return 1;
}

int sysroot_model_is_dolce(void)
{
	const unsigned short device_config = __builtin_bswap16(sysroot->device_config);
	const unsigned short device_type = __builtin_bswap16(sysroot->device_type);
	const unsigned int hw_info_mask = sysroot->hw_info & 0xFF0000;

	if (device_type == 0x101 && device_config == 0x408)
		return 1;

	if (device_config & 0x200)
		return 1;

	if (!sysroot_model_is_unk())
		return 0;

	if ((hw_info_mask == 0x700000) || (hw_info_mask == 0x720000) || (hw_info_mask == 0x510000))
		return 1;

	return 0;
}

int sysroot_model_is_vita2k(void)
{
	const unsigned int hw_info_mask = sysroot->hw_info & 0xFF0000;

	if (sysroot_model_is_dolce())
		return 0;

	if (hw_info_mask == 0x800000)
		return 1;

	return 0;
}

int sysroot_model_is_unk(void)
{
	const unsigned short device_config = __builtin_bswap16(sysroot->device_config);
	const unsigned short device_type = __builtin_bswap16(sysroot->device_type);
	const unsigned short type = __builtin_bswap16(sysroot->type);

	if (device_type != 0x103 || device_config != 0x10)
		return 0;

	if (type <= 0x24)
		return 1;

	return 0;
}
