INCLUDE AppBuildZBPro.ld

SECTIONS
{
        .ro_mac_address :
        {
            KEEP(*(.ro_mac_address));
        } > flash

        .ro_ota_header :
        {
            KEEP(*(.ro_ota_header));
        } > flash
        
        .ro_se_lnkKey :
        {
            KEEP(*(.ro_se_lnkKey));
        } > flash

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

