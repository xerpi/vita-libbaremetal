#include "sysroot.h"

int sysroot_model_is_vita(const struct sysroot_buffer *sysroot)
{
	const unsigned short device_type = __builtin_bswap16(sysroot->device_type);

	if ((device_type - 0x100 > 0x11) || sysroot_model_is_dolce(sysroot)) {
		const unsigned int unkd4_mask = sysroot->unkd4[0] & 0xFF0000;

		if (!sysroot_model_is_unk(sysroot))
			return 0;

		if ((unkd4_mask == 0x700000) || (unkd4_mask == 0x720000) || (unkd4_mask == 0x510000))
			return 0;
	}

	return 1;
}

int sysroot_model_is_dolce(const struct sysroot_buffer *sysroot)
{
	const unsigned short device_config = __builtin_bswap16(sysroot->device_config);
	const unsigned short device_type = __builtin_bswap16(sysroot->device_type);
	const unsigned int unkd4_mask = sysroot->unkd4[0] & 0xFF0000;

	if (device_type == 0x101 && device_config == 0x408)
		return 1;

	if (device_config & 0x200)
		return 1;

	if (!sysroot_model_is_unk(sysroot))
		return 0;

	if ((unkd4_mask == 0x700000) || (unkd4_mask == 0x720000) || (unkd4_mask == 0x510000))
		return 1;

	return 0;
}

int sysroot_model_is_vita2k(const struct sysroot_buffer *sysroot)
{
	const unsigned int unkd4_mask = sysroot->unkd4[0] & 0xFF0000;

	if (sysroot_model_is_dolce(sysroot))
		return 0;

	if (unkd4_mask == 0x800000)
		return 1;

	return 0;
}

int sysroot_model_is_unk(const struct sysroot_buffer *sysroot)
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
