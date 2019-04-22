ARM9 memory map viewer for Nintendo DSi
Compiles with devkitPro (https://devkitpro.org/)
Run with dslink: dslink -a <DSi IP address> mmap.nds

Controls

A = Read memory at current address
L/R = Select nibble of address to modify
U/D = Adjust selected nibble of address by 1
LTrg/RTrg = Adjust address by 0x10
X = Hold for other keys to repeat automatically (useful with LTrg/RTrg)
B = Reset program state
Select = Exit program
Select = Run swiSoftReset()
