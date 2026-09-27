// Checks DataLayer_GetCRC for issue #72: the polynomial in DataLayer_CRC has to be
// able to be any number, not only 0b11011.
//
// The polynomial is a compile time define, so the test is run once per polynomial:
//
//   for p in 0b11 0b111 0b1011 0b11011 0b110101 0b1100111 0b11010101 0b100000111; do
//       gcc -Wall -Wno-unused-function -I.. -D DataLayer_CRC=$p -o /tmp/crc crc.c -luuid || exit 1
//       /tmp/crc || exit 1
//   done

#include <stdio.h>

#include "../Libs/Hash/md5.c"
#include "../Libs/Allocator.c"
#include "../Libs/File.c"
#include "../Libs/Memory.c"
#include "../Libs/String.c"
#include "../Libs/Buffer.c"
#include "../Libs/BitHelper.c"
#include "../Libs/uuid.c"

#include "../Libs/Communication/DataLayer.h"

static int failures = 0;

// The width the CRC is supposed to have, worked out here instead of with
// DataLayer_CRC_Width so the test stays an independent check of the header.
static int Test_Width()
{
	int width = 0;

	for (unsigned int poly = (unsigned int)(DataLayer_CRC); poly > 1; poly = poly >> 1)
		width++;

	return width;
}

// Plain polynomial long division, used as a reference for DataLayer_GetCRC. It keeps
// the top bit of the polynomial, so the XOR clears the leading bit on its own.
static UInt8 Test_ReferenceCRC(const unsigned char* _Data, int _Size)
{
	int width = Test_Width();
	UInt16 remainder = 0;

	for (int i = 0; i < _Size; i++)
	{
		UInt16 value = (UInt16)((_Data[i] << width) | remainder);

		for (int j = width + 7; j >= width; j--)
		{
			if(((value >> j) & 0x1) == 1)
				value = value ^ ((DataLayer_CRC) << (j - width));
		}

		remainder = value;
	}

	return (UInt8)remainder;
}

static void Test_Compare(const char* _Name, unsigned char* _Data, int _Size)
{
	UInt8 CRC = 0;
	DataLayer_GetCRC(_Data, _Size, &CRC);

	UInt8 expected = Test_ReferenceCRC(_Data, _Size);
	UInt8 max = (UInt8)((0x1 << Test_Width()) - 1);

	if(CRC != expected)
	{
		printf("FAIL %s: got 0x%02X, expected 0x%02X\n\r", _Name, CRC, expected);
		failures++;
		return;
	}

	// The CRC is the remainder, so it never uses more bits than the width.
	if(CRC > max)
	{
		printf("FAIL %s: 0x%02X does not fit in %i bits\n\r", _Name, CRC, Test_Width());
		failures++;
		return;
	}

	printf("ok   %s: 0x%02X\n\r", _Name, CRC);
}

// A single bit change anywhere in the message has to change the CRC, otherwise the
// division is not reducing the message at all.
static void Test_DetectsChange(const char* _Name, unsigned char* _Data, int _Size)
{
	UInt8 CRC = 0;
	DataLayer_GetCRC(_Data, _Size, &CRC);

	for (int i = 0; i < _Size; i++)
	{
		unsigned char original = _Data[i];
		UInt8 changed = 0;

		_Data[i] = (unsigned char)(original ^ 0x01);
		DataLayer_GetCRC(_Data, _Size, &changed);
		_Data[i] = original;

		if(changed == CRC)
		{
			printf("FAIL %s: flipping a bit in byte %i left the CRC at 0x%02X\n\r", _Name, i, CRC);
			failures++;
			return;
		}
	}

	printf("ok   %s: every single bit change changes the CRC\n\r", _Name);
}

int main()
{
	unsigned char zero[1] = { 0x00 };
	unsigned char one[1] = { 0x01 };
	unsigned char ff[1] = { 0xFF };
	unsigned char seq[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
	unsigned char text[11] = "DistroFiles";
	unsigned char mix[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x42 };

	printf("DataLayer_CRC: 0x%X, width: %i\n\r", (unsigned int)(DataLayer_CRC), Test_Width());

	Test_Compare("empty", zero, 0);
	Test_Compare("0x00", zero, 1);
	Test_Compare("0x01", one, 1);
	Test_Compare("0xFF", ff, 1);
	Test_Compare("seq8", seq, 8);
	Test_Compare("text", text, 11);
	Test_Compare("mix6", mix, 6);

	Test_DetectsChange("seq8", seq, 8);
	Test_DetectsChange("mix6", mix, 6);

	// The default polynomial has to keep giving the same CRCs as before issue #72,
	// a change there would break every peer that is already running.
	#if (DataLayer_CRC) == 0b11011
	{
		UInt8 CRC = 0;
		struct { const char* m_Name; unsigned char* m_Data; int m_Size; UInt8 m_Expected; } known[] =
		{
			{ "0x00", zero, 1, 0x00 },
			{ "0x01", one, 1, 0x0B },
			{ "0xFF", ff, 1, 0x0F },
			{ "seq8", seq, 8, 0x02 },
			{ "text", text, 11, 0x0E },
			{ "mix6", mix, 6, 0x03 },
		};

		for (int i = 0; i < (int)(sizeof(known) / sizeof(known[0])); i++)
		{
			DataLayer_GetCRC(known[i].m_Data, known[i].m_Size, &CRC);
			if(CRC != known[i].m_Expected)
			{
				printf("FAIL known %s: got 0x%02X, expected 0x%02X\n\r", known[i].m_Name, CRC, known[i].m_Expected);
				failures++;
			}
			else
				printf("ok   known %s: 0x%02X\n\r", known[i].m_Name, CRC);
		}
	}
	#endif

	if(failures != 0)
	{
		printf("%i check(s) failed\n\r", failures);
		return EXIT_FAILURE;
	}

	printf("All checks passed\n\r");
	return EXIT_SUCCESS;
}
