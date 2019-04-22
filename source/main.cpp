/*
	ARM9 memory map viewer
	(C) 2019 Charles MacDonald

	L/R = Select nibble
	U/D = Change nibble
	Lt/Rt = Advance address by -/+ 0x10
	A = Read memory and update display
	B = Reset program state
	X = Hold to make other keys toggle when pressed

*/
#include <nds.h>
#include <stdio.h>
#include <ctype.h>

const int buffer_size = 16;

volatile bool running;
volatile int frame;
volatile uint32_t key_old, key_new, key_delta;
volatile uint32_t nibble_select;
volatile uint32_t buffer[buffer_size];
volatile uint32_t base_address;
volatile bool update;

void igotoxy(uint8_t x, uint8_t y)
{
	char buf[16];
	sprintf(buf, "\x1b[%d;%dH", x, y);
	iprintf(buf);
}	

uint32_t bswap(uint32_t value)
{
	value = (
		((value >> 24) & 0x000000FF) |
		((value >>  8) & 0x0000FF00) |
		((value <<  8) & 0x00FF0000) |
		((value << 24) & 0xFF000000) );
	return value;
}

void refresh_display(void)
{
	igotoxy(3, 0);
	for(int row = 0; row < buffer_size; row++)
	{
		/* Print address and data */
		iprintf("%08lX : %08lX | ", 
			base_address + (row << 2), 
			buffer[row]
			);

		/* Print characters */
		for(int x = 0; x < 4; x++)
		{
			char ch = (buffer[row] >> (x << 3)) & 0x7F;
			if(!isprint(ch))
				ch = '.';
			iprintf("%c", ch);
		}
		iprintf("\n");
	}
}

void reset_viewer(void)
{
	running = true;
	frame = 0;
	nibble_select = 0;
	base_address = 0xffff0000;
	for(int i = 0; i < 16; i++)
		buffer[i] = 0xdeadbeef;
	update = true;
}


void Vblank() 
{
	frame++;

	/* Update key state */
	scanKeys();
	key_old = key_new;
	key_new = keysCurrent();
	key_delta = (key_old ^ key_new) & key_new;

	/* When holding X the other keys when pressed will toggle their state on every frame */
	if(key_new & KEY_X)
		key_delta ^= (key_new & ~KEY_X);

	/* L/R = Change selected nibble */
	if(key_delta & KEY_LEFT)
		nibble_select = (nibble_select + 1) & 7;
	if(key_delta & KEY_RIGHT)
		nibble_select = (nibble_select - 1) & 7;
	
	/* D = Decrease selected nibble */
	if(key_delta & KEY_DOWN)
	{
		base_address -= (0x01 << (nibble_select << 2));
	}

	/* U = Increase selected nibble */
	if(key_delta & KEY_UP)
	{
		base_address += (0x01 << (nibble_select << 2));
	}

	/* B = Reset program state */
	if(key_delta & KEY_B)
	{
		reset_viewer();
	}

	/* A = Re-read memory and print */
	if(key_delta & KEY_A)
	{
		for(int i = 0; i < 16; i++)
			buffer[i] = 0xdeadbeef;
		
		uint32_t *p = (uint32 *)base_address;
		for(int i = 0; i < 16; i++)
			buffer[i] = p[i];

		update = true;
	}

	/* RTrg = Increment by one page */
	if(key_delta & KEY_R)
	{
		base_address += 0x10;
		update = true;
	}

	/* LTrg = Decrement by one page */
	if(key_delta & KEY_L)
	{
		base_address -= 0x10;
		update = true;
	}

	/* Start : SWI call to SoftReset SWI (hangs)*/
	if(key_delta & KEY_START)
	{
		swiSoftReset();
	}

	/* Select : Make main() return to caller (power off) */
	if(key_delta & KEY_SELECT)
	{
		running = false;
	}

	/* Print current base address and nibble selected */
	igotoxy(1, 0);
	iprintf("Base: %08lx Nibble:%lu\n", base_address, nibble_select);

	/* Update display if requested */
	if(update)
	{
		refresh_display();
		update = false;
	}
}

void print_banners(void)
{
	igotoxy(0, 0);
	iprintf("ARM9 memory map viewer\n");

	igotoxy(24-4, 0);
	iprintf("L/R = Select nibble\n");
	iprintf("U/D = Change nibble\n");
	iprintf("A   = Read memory\n");
	iprintf("B   = Reset view");
}	


int main(void) 
{
	reset_viewer();
	irqSet(IRQ_VBLANK, Vblank);
	consoleDemoInit();
	print_banners();

	while(running) 
	{
		swiWaitForVBlank();
	}

	return 0;
}

/* End */