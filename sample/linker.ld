OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)

SECTIONS
{
	. = 0x40300000;
	.text : {
		*(.text)
		*(.text*)
	}

	.data ALIGN(4) : {
		_data_start = .;
		*(.data)
		. = ALIGN(4);
		_data_end = .;
	}

	.bss ALIGN(4) (NOLOAD) : {
		_bss_start = .;
		*(.bss)
		. = ALIGN(4);
		_bss_end = .;
	}
}
