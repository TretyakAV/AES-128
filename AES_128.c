#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AES_128.h"
#include "Tables.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

void encrypt(char final_fname[256])
{
	FILE *decr_file = fopen(final_fname, "rb");
	FILE *encr_file = fopen("Encrypted.txt", "wb");

	long fsize = filesize(decr_file);
	long encrypt_size = 0;

	unsigned long RoundKeys[44];
	unsigned char State[4][4];
	unsigned char InputKey[4][4] =
	{
		{ 0x24, 0x34, 0x31, 0x13 },
	{ 0x75, 0x75, 0xe2, 0xaa },
	{ 0xa2, 0x56, 0x12, 0x54 },
	{ 0xb3, 0x88, 0x00, 0x87 }
	};

	Key_Expansion(InputKey, RoundKeys);

	decr_file = complement_file(decr_file, fsize, final_fname);

	while (encrypt_size < fsize)
	{
		get_new_block(State, decr_file);
		encrypt_block(State, RoundKeys);
		output_block(State, encr_file);
		encrypt_size = ftell(decr_file);
	}

	fclose(decr_file);
	fclose(encr_file);

	copy_final_text(decr_file, encr_file, final_fname);


#ifdef _WIN32
	_unlink("Encrypted.txt");
#else
	unlink("Encrypted.txt");
#endif

	return;
}
void decrypt(char  file_fname[256])
{
	FILE *decr_file = fopen("temple_decrypted.txt", "wb");
	FILE *encr_file = fopen(file_fname, "rb");

	unsigned char State[4][4];

	long fsize = filesize(encr_file);
	long decrypt_size = 0;

	unsigned long RoundKeys[44];
	unsigned char InputKey[4][4] =
	{
		{ 0x24, 0x34, 0x31, 0x13 },
	{ 0x75, 0x75, 0xe2, 0xaa },
	{ 0xa2, 0x56, 0x12, 0x54 },
	{ 0xb3, 0x88, 0x00, 0x87 }
	};

	Key_Expansion(InputKey, RoundKeys);

	while (decrypt_size < fsize)
	{
		get_new_block(State, encr_file);
		decrypt_block(State, RoundKeys);
		output_block(State, decr_file);
		decrypt_size = ftell(encr_file);
	}

	fclose(decr_file);
	fclose(encr_file);

	delete_excess_zeroes(decr_file, file_fname);
}

void printf_matrix(unsigned char A[4][4])
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			//printf(" 0x%2x ", A[i][j]);
			printf(" %x ", A[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

//Циклический свдиг влево.
void RotWord(unsigned long  *dword)
{
	unsigned long b = 0xff000000;

	b &= *dword;
	b >>= 24;
	*dword &= 0x00ffffff;
	*dword <<= 8;
	*dword |= b;
}
//Циклический свдиг вправо.
void RotWord_right(unsigned long  *dword)
{
	unsigned long b = 0x000000ff;

	b &= *dword;
	b <<= 24;
	*dword &= 0xffffff00;
	*dword >>= 8;
	*dword |= b;
}

unsigned char multiply(unsigned char a, unsigned char b)
{
	switch (b)
	{
	case 2:
		return table_2[a];
	case 3:
		return table_3[a];
	case 14:
		return table_14[a];
	case 11:
		return table_11[a];
	case 13:
		return table_13[a];
	case 9:
		return table_9[a];
	default:
		return a;
	}
}

void SubWord(unsigned long  *dword)
{
	unsigned long i;
	unsigned long j;
	unsigned long tmp_dword = *dword;
	unsigned long result_dword = 0;

	for (int k = 0; k < 4; k++)
	{
		i = 0xf0000000;
		j = 0x0f000000;
		i &= tmp_dword;
		j &= tmp_dword;
		i >>= 28;
		j >>= 24;
		result_dword <<= 8;
		result_dword+=S_Box[i][j];
		tmp_dword <<= 8;
	}

	for (i = 0; i < 4; i++)
		RotWord(&result_dword);

	*dword = result_dword;
}
void InvSubWord(unsigned long  *dword)
{
	unsigned long i;
	unsigned long j;
	unsigned long tmp_dword = *dword;
	unsigned long result_dword = 0;

	for (int k = 0; k < 4; k++)
	{
		i = 0xf0000000;
		j = 0x0f000000;
		i &= tmp_dword;
		j &= tmp_dword;
		i >>= 28;
		j >>= 24;
		result_dword <<= 8;
		result_dword += InvS_Box[i][j];
		tmp_dword <<= 8;
	}

	for (i = 0; i < 4; i++)
		RotWord(&result_dword);

	*dword = result_dword;
}

void SubBytes(unsigned char State[4][4])
{
	int i = 0;
	unsigned long dword;

	for (i = 0; i < 4; i++)
	{
		dword = get_dword(State, i);
		SubWord(&dword);
		dword_to_matrix(dword, State, i);
	}
}
void InvSubBytes(unsigned char State[4][4])
{
	int i = 0;
	unsigned long dword;

	for (i = 0; i < 4; i++)
	{
		dword = get_dword(State, i);
		InvSubWord(&dword);
		dword_to_matrix(dword, State, i);
	}
}

void ShiftRows(unsigned char State[4][4])
{
	unsigned long dword;

	for (int i = 0; i < 4; i++)
	{
		dword = get_string_dword(State, i);
		for (int j = 0; j < i; j++)
		{
			RotWord(&dword);
		}
		dword_string_to_matrix(dword, State, i);
	}

}
void InvShiftRows(unsigned char State[4][4])
{
	unsigned long dword;

	for (int i = 0; i < 4; i++)
	{
		dword = get_string_dword(State, i);
		for (int j = 0; j < i; j++)
		{
			RotWord_right(&dword);
		}
		dword_string_to_matrix(dword, State, i);
	}

}

void MixColumns(unsigned char State[4][4])
{
	unsigned char C[4][4] =
	{
		{ 0x02, 0x03, 0x01, 0x01 },
		{ 0x01, 0x02, 0x03, 0x01 },
		{ 0x01, 0x01, 0x02, 0x03 },
		{ 0x03, 0x01, 0x01, 0x02 }
	};
	unsigned char Temp_array[4][4];
	unsigned char s = 0;
	unsigned char res;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			s = 0;
			for (int k = 0; k < 4; k++)
			{
				res = multiply(State[k][i], C[j][k]);
				s ^= res;
			}
			Temp_array[j][i] = s;
		}
	}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			State[i][j] = Temp_array[i][j];
}
void InvMixColumns(unsigned char State[4][4])
{
	unsigned char C[4][4] =
	{
		{ 0x0e, 0x0b, 0x0d, 0x09 },
	{ 0x09, 0x0e, 0x0b, 0x0d },
	{ 0x0d, 0x09, 0x0e, 0x0b },
	{ 0x0b, 0x0d, 0x09, 0x0e }
	};
	unsigned char Temp[4][4];
	unsigned char s = 0;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			s = 0;
			for (int k = 0; k < 4; k++)
				s ^= multiply(State[k][i], C[j][k]);
			Temp[j][i] = s;
		}
	}

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			State[i][j] = Temp[i][j];
}

void AddRoundKey(unsigned char State[4][4], unsigned long RoundKeys[44], int num_key)
{
	unsigned long dword_state;

	for (int i = 0; i < 4; i++)
	{
		dword_state = get_dword(State, i);
		dword_state ^= RoundKeys[i + num_key];
		dword_to_matrix(dword_state, State, i);
	}
}

long filesize(FILE *file)
{
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	return size;
}
void get_new_block(unsigned char State[4][4], FILE *file)
{

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			State[j][i] = (unsigned char)getc(file);
		}
	}
}

//Преобразует столбец матрицы в двойное слово
unsigned long get_dword(unsigned char A[4][4], int index)
{
	unsigned long  dword = 0;

	for (int i = 0; i < 4; i++)
	{
		dword = dword << 8;
		dword += A[i][index];
	}

	return dword;
}
//Преобразует строку матрицы в двойное слово
unsigned long get_string_dword(unsigned char A[4][4], int index)
{
	unsigned long  dword = 0;

	for (int i = 0; i < 4; i++)
	{
		dword = dword << 8;
		dword += A[index][i];
	}

	return dword;
}
// dword в столбец
void dword_to_matrix(unsigned long dword, unsigned char A[4][4], int index)
{
	unsigned long temp_dword;
	for (int i = 3; i >-1; i--)
	{
		temp_dword=0x00ff;
		temp_dword &= dword;
		dword = dword >> 8;
		A[i][index] = (unsigned char)temp_dword;
	}
}
// dword в строку
void dword_string_to_matrix(unsigned long dword, unsigned char A[4][4], int index)
{
	unsigned long temp_dword;
	for (int i = 3; i >-1; i--)
	{
		temp_dword = 0x00ff;
		temp_dword &= dword;
		dword = dword >> 8;
		A[index][i] = (unsigned char)temp_dword;
	}

}

FILE* complement_file(FILE *file, int file_size, char final_fname[256])
{
	fclose(file);
	file = fopen(final_fname, "ab");
	char count_zero;

	if ((count_zero=(char)(file_size % 16)) != 15)
	{
		count_zero = 15 - count_zero;
		for (int i = 0; i < count_zero; i++)
			fputc(0x00, file);

	}
	else count_zero = 0;

	fputc(count_zero, file);
	fclose(file);

	file = fopen(final_fname, "rb");
	return file;
}

void encrypt_block(unsigned char State[4][4], unsigned long RoundKeys[44])
{
	int round = 0;

	AddRoundKey(State, RoundKeys, 0);

	for (round = 1; round <= 9; round++)
	{
		SubBytes(State);
		ShiftRows(State);
		MixColumns(State);
		AddRoundKey(State, RoundKeys, round * 4);
	}

	SubBytes(State);
	ShiftRows(State);
	AddRoundKey(State, RoundKeys, round * 4);
}
void decrypt_block(unsigned char State[4][4], unsigned long RoundKeys[44])
{
	int round = 0;

	AddRoundKey(State, RoundKeys, 40);

	for (round = 1; round <= 9; round++)
	{
		InvSubBytes(State);
		InvShiftRows(State);
		AddRoundKey(State, RoundKeys, 40 - round * 4);
		InvMixColumns(State);
	}

	InvSubBytes(State);
	InvShiftRows(State);
	AddRoundKey(State, RoundKeys, 0);
}

unsigned long g(unsigned long key, int round_num)
{
	unsigned long rcon_value = rcon[round_num / 4 + 1] << 24;
	
	RotWord(&key);
	SubWord(&key);
	key = key ^ rcon_value;

	return key;
}
void Key_Expansion(unsigned char InputKey[4][4], unsigned long RoundKeys[44])
{
	int i;

	for (i = 0; i < 4; i++)
		RoundKeys[i] = get_dword(InputKey, i);

	for (i = 0; i < 40; i += 4)
	{
		RoundKeys[i + 4] = RoundKeys[i] ^ g(RoundKeys[i + 3], i);
		RoundKeys[i + 5] = RoundKeys[i + 4] ^ RoundKeys[i + 1];
		RoundKeys[i + 6] = RoundKeys[i + 5] ^ RoundKeys[i + 2];
		RoundKeys[i + 7] = RoundKeys[i + 6] ^ RoundKeys[i + 3];
	}

}

void output_block(unsigned char State[4][4], FILE *encr_file)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			fwrite(&State[j][i], sizeof(unsigned char), 1, encr_file);
		}
	}
}

void copy_final_text(FILE *to, FILE *from, char final_fname[256])
{
	long fsize;
	char c;

	to = fopen(final_fname, "wb");
	from = fopen("Encrypted.txt", "rb");

	fsize = filesize(from);

	for (int i = 0; i < fsize; i++)
	{
		c = fgetc(from);
		fwrite(&c, sizeof(unsigned char), 1, to);
	}

	fclose(to);
	fclose(from);
}
void delete_excess_zeroes(FILE *decr_file, char final_fname[])
{
	int cnt_zeroes;
	int c;
	long fsize;
	FILE *final_file;

	decr_file = fopen("temple_decrypted.txt", "rb");

	fsize = filesize(decr_file);
	fseek(decr_file, -1, SEEK_END);
	cnt_zeroes = fgetc(decr_file);

#ifdef _WIN32
	_unlink(final_fname);
#else
	unlink(final_fname);
#endif

	final_file = fopen(final_fname, "wb");

	fseek(decr_file, 0, SEEK_SET);
	for (int i = 0; i < fsize - cnt_zeroes - 1; i++)
	{
		c = fgetc(decr_file);
		fwrite(&c, sizeof(unsigned char), 1, final_file);
	}

	fclose(final_file);
	fclose(decr_file);


#ifdef _WIN32
	_unlink("temple_decrypted.txt");
#else
	unlink("temple_decrypted.txt");
#endif
}
