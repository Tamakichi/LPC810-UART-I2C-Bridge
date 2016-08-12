/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2016
 * (c) NXP Semiconductors 2013-2016
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v7.6.2 [Build 326] [2015-02-02]  on Sat Aug 13 06:58:58 JST 2016)
*/

MEMORY
{
  /* Define each memory region */
  MFlash4 (rx) : ORIGIN = 0x0, LENGTH = 0x1000 /* 4K bytes */
  RamLoc1 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x400 /* 1K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_MFlash4 = 0x0 + 0x1000;
  __top_RamLoc1 = 0x10000000 + 0x400;



