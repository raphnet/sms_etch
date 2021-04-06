#include "vfont.h"
#include "main.h"

struct letter {
	int8_t org_x, org_y;
	int8_t w;
	const char *data;
};

struct letter lowercase[] = {
	{ 2, 2, 3, "azxddewqa" }, // a
	{ 0, 5, 3, "xxxxxddewqa" }, // b
	{ 2, 3, 3, "azxcde" }, // c
	{ 2, 3, 3, "azxcddwwwww" }, // d
	{ 2, 2, 3, "dqazxcdd" }, // e
	{ 1, 5, 1, "zxxdaxxxz" }, // f
	{ 2, 0, 3, "aqwedcxxxz" }, // g
	{ 0, 5, 3, "xxxxxwwedcxx" }, // h
	{ 0, 3, 0, "xxxswwwwsw" }, // i
	{ 0, 3, 1, "xxxxzsewwwwwsw" }, // j
	{ 0, 5, 2, "xxxxxwweezzcc" }, // k
	{ 0, 5, 1, "xxxxx" }, // l
	{ 0, 3, 4, "xxxwwecxxwwecxx" }, // m
	{ 0, 3, 2, "xxxwwecxx" }, // n
	{ 1, 3, 3, "zxcdewqa" }, // o
	{ 1, 0, 3, "dewqazxxxx" }, // p
	{ 2, 0, 3, "aqwedcxxxx" }, // q
	{ 0, 3, 2, "xxxwwed" }, // r
	{ 2, 3, 2, "azcsdsza" }, // s
	{ 0, 0, 1, "wwwwwxadd" }, // t
	{ 0, 3, 3, "xxcdewwxxx" }, // u
//	{ 0, 3, 3, "xxcdeww" }, // v
	{ 0, 3, 2, "xccwww" }, // v
	{ 0, 3, 4, "xxceceww" }, // w
	{ 0, 3, 3, "cccswwwszzz" }, // x
	{ 0, 3, 3, "xxcswwwddsxzzzz" }, // y
	{ 0, 3, 3, "dddzzzddd" }, // z
};

struct letter uppercase[] = {
	{ 0, 0, 3, "wwwwedcxxxxwwaaa" }, // A
	{ 0, 5, 3, "xxxxxswwwwwsddczcxxaaa" }, // B
	{ 3, 4, 3, "qazxxxcde" }, // C
	{ 0, 5, 3, "xxxxxswwwwwsdccxxxaaa" }, // D
	{ 0, 5, 3, "xxxxxdddswwasaawwddd" }, // E
	{ 0, 5, 3, "xxxxxwwddaawwddd" }, // F
	{ 3, 5, 3, "aazxxxcdewq" }, // G
	{ 0, 5, 3, "xxxxxswwwsddswwsxxxxx" }, // H
	{ 0, 5, 2, "ddaxxxxxadd" }, // I
	{ 0, 5, 2, "ddsxxxxxaq" }, // J
	{ 0, 5, 2, "xxxxxwwweezzccx" }, // K
	{ 0, 5, 2, "xxxxxdd" }, // L
	{ 0, 0, 4, "wwwwwcceexxxxx" }, // M
	{ 0, 0, 3, "wwwwwcccxxwwwww" }, // N
	{ 1, 5, 3, "zxxxcdewwwqa" }, // O
	{ 0, 0, 3, "wwwwwddcxzaa" }, // P
	{ 1, 5, 3, "zxxxcdewwwqasxxxscc" }, // Q
	{ 0, 0, 3, "wwwwwddcxzsascc" }, // R
	//{ 0, 2, 3, "xcdewqszsqwedc" }, // S
	{ 0, 2, 3, "xcdewqaawedc" }, // S
	{ 0, 5, 4, "ddddaaxxxxx" }, // T
	{ 0, 5, 3, "xxxxcdewwww" }, // U
	{ 0, 5, 4, "xxxcceewww" }, // V
	{ 0, 5, 4, "xxxxcewxcewwww" }, // W
	{ 0, 5, 3, "xcccxsaaasweeew" }, // X
	{ 0, 5, 4, "cceezzxxx" }, // Y
	{ 0, 5, 3, "dddxzzzxddd" }, // Z
};


struct letter number[] = {
	{ 1, 5, 3, "zxxxcdewwwqa" }, // 0
	{ 0, 4, 2, "exxxxxadd" }, // 1
	{ 0, 4, 3, "edcxzzzddd" }, // 2
	{ 0, 5, 3, "dddzzdcxzaa" }, // 3
	{ 0, 5, 3, "xxxdddswwwsxxxxx" }, // 4
	{ 3, 5, 3, "aaaxxddcxzaa" }, // 5
	{ 3, 5, 3, "aazxxxcdewqa" }, // 6
	{ 0, 5, 3, "dddxxzzx" }, // 7
	{ 1, 3, 3, "zxcdewqaqedcz" }, // 8
	{ 1, 2, 3, "qwedcxxzza" }, // 9
	{ 0, 0, 1, "wdxaswwswdxa" }, // :
	{ 0, 0, 1, "wdxzaeswwswdxa" }, // ;
	{ 0, 2, 3, "eesdszzccsasqq" }, // <<
	{ 0, 0, 1 }, // =
	{ 0, 0, 3, "eeqqsdscczz" }, // >>
	{ 1, 0, 3, "wdxawwewaxawedcxzx" }, // ?

};

struct letter symbols[] = {
	{ 0, 0, 3 }, // space
	{ 0, 0, 1, "wdxaswwswwwdxxxa" }, // !
	{ 0, 5, 1, "xsdsw" }, // "
	{ 0, 0, 1 }, // #
	{ 0, 0, 1 }, // $
	{ 0, 0, 1 }, // %
	{ 0, 0, 1 }, // &
	{ 0, 5, 1, "x" }, // '
	{ 1, 5, 1, "zxxxxc" }, // (
	{ 0, 5, 1, "cxxxxz" }, // )
	{ 0, 0, 1 }, // *
	{ 0, 0, 1 }, // +
	//{ 0, 0, 1, "wdxzae" }, // ,
	{ 0, 1, 1, "xz" }, // ,
	{ 0, 0, 1 }, // _
	{ 0, 0, 1, "wdxa" }, // .
};

uint8_t pen_x, pen_y;

void drawLetter(char c, uint8_t scale)
{
	const char *p = "";
	uint8_t x, y, w;
	uint8_t pen_down;
	uint8_t first;
	uint8_t old_x, old_y;
	uint8_t initial_pen_x;
	uint8_t initial_pen_y;

	if (c >= 'a' && c <= 'z') {
		p = lowercase[c - 'a'].data;
		x = lowercase[c - 'a'].org_x;
		y = lowercase[c - 'a'].org_y;
		w = lowercase[c - 'a'].w;
	} else if (c >= 'A' && c <= 'Z') {
		p = uppercase[c - 'A'].data;
		x = uppercase[c - 'A'].org_x;
		y = uppercase[c - 'A'].org_y;
		w = uppercase[c - 'A'].w;
	} else if (c >= '0' && c <= '?') {
		p = number[c - '0'].data;
		x = number[c - '0'].org_x;
		y = number[c - '0'].org_y;
		w = number[c - '0'].w;
	}
	else if (c >= ' ' && c <= '.') {
		p = symbols[c - ' '].data;
		x = symbols[c - ' '].org_x;
		y = symbols[c - ' '].org_y;
		w = symbols[c - ' '].w;
	} else {
		// default to space
		w = 3;
		x = 0; y= 0;
	}

	initial_pen_x = pen_x;
	initial_pen_y = pen_y;

	pen_x += x * scale;
	pen_y -= y * scale;
	first = 1;
	pen_down = 1;

	if (p) {
		while (*p)
		{
			old_x = pen_x;
			old_y = pen_y;

			switch (*p)
			{
				default:
					break;
				case 'a': pen_x -= scale; break;
				case 'q': pen_x -= scale; pen_y -= scale; break;
				case 'w': pen_y -= scale; break;
				case 'e': pen_x += scale; pen_y -= scale; break;
				case 'd': pen_x += scale; break;
				case 'c': pen_x += scale; pen_y += scale; break;
				case 'x': pen_y += scale; break;
				case 'z': pen_x -= scale; pen_y += scale; break;
				case 's': pen_down = !pen_down; break;
			}

			if (pen_down && ( (old_x != pen_x) || (old_y != pen_y) )) {
				updateDisplay(pen_x, pen_y, old_x, old_y, 0);

	//			drawLine(old_x, old_y, pen_x, pen_y);
			}

			first = 0;

			p++;

			vsync_keycheck();
			if (start_pressed)
				break;
		}
	}

	pen_x = initial_pen_x + ((w) * scale) + 2;
	pen_y = initial_pen_y;
}

void drawString(const char *str, int8_t scale)
{
	while (*str) {
		drawLetter(*str, scale);
		str++;

		if (start_pressed)
			break;
	}
}


