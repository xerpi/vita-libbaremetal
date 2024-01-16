#include "sysroot.h"

static const struct sysroot_buffer *sysroot;

void sysroot_init(const struct sysroot_buffer *sysroot_buffer)
{
	sysroot = sysroot_buffer;
}

uint32_t sysroot_get_hw_info(void)
{
	return sysroot->hw_info;
}

int sysroot_model_is_vita(void)
{
	const uint16_t device_type = __builtin_bswap16(sysroot->device_type);

	if ((device_type - 0x100 > 0x11) || sysroot_model_is_dolce()) {
		const uint32_t hw_info_mask = sysroot->hw_info & 0xFF0000;

		if (!sysroot_model_is_diag())
			return 0;

		if ((hw_info_mask == 0x700000) || (hw_info_mask == 0x720000) || (hw_info_mask == 0x510000))
			return 0;
	}

	return 1;
}

int sysroot_model_is_dolce(void)
{
	const uint16_t device_config = __builtin_bswap16(sysroot->device_config);
	const uint16_t device_type = __builtin_bswap16(sysroot->device_type);
	const uint32_t hw_info_mask = sysroot->hw_info & 0xFF0000;

	if (device_type == 0x101 && device_config == 0x408)
		return 1;

	if (device_config & 0x200)
		return 1;

	if (!sysroot_model_is_diag())
		return 0;

	if ((hw_info_mask == 0x700000) || (hw_info_mask == 0x720000) || (hw_info_mask == 0x510000))
		return 1;

	return 0;
}

int sysroot_model_is_vita2k(void)
{
	const uint32_t hw_info_mask = sysroot->hw_info & 0xFF0000;

	if (sysroot_model_is_dolce())
		return 0;

	if (hw_info_mask == 0x800000)
		return 1;

	return 0;
}

int sysroot_model_is_diag(void)
{
	const uint16_t device_config = __builtin_bswap16(sysroot->device_config);
	const uint16_t device_type = __builtin_bswap16(sysroot->device_type);
	const uint16_t type = __builtin_bswap16(sysroot->type);

	if (device_type != 0x103 || device_config != 0x10)
		return 0;

	if (type == 0x24)
		return 1;

	return 0;
}

int sysroot_is_au_codec_ic_conexant(void)
{
	return (sysroot->hw_flags[0] & 3) == 1;
}
