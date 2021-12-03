INCLUDE AppBuildZBPro.ld

SECTIONS
{
        /*
         * Make a dummy section, so that previous section is padded to a 16-byte boundary
         */
        .pad ((LOADADDR(.text) + SIZEOF(.text) + SIZEOF(.data) + 15) & ~ 15 ):
        {
            . = ALIGN (16);
            LONG(0x00000000)
            LONG(0x00000000)
            LONG(0x00000000)
            LONG(0x00000000)
        } > flash
}

