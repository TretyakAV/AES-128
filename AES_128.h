#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printf_matrix(unsigned char A[4][4]);

long filesize(FILE *file);
void get_new_block(unsigned char State[4][4], FILE *file);
void output_block(unsigned char State[4][4], FILE *encr_file);

//����������� ������� ������� � ������� �����
unsigned long get_dword(unsigned char A[4][4], int index);
//����������� ������ ������� � ������� �����
unsigned long get_string_dword(unsigned char A[4][4], int index);

FILE* complement_file(FILE *file, int file_size, char final_fname[256]);

unsigned char multiply(unsigned char a, unsigned char b);
// dword � �������
void dword_to_matrix(unsigned long dword, unsigned char A[4][4], int index);
// dword � ������
void dword_string_to_matrix(unsigned long dword, unsigned char A[4][4], int index);
//���������� �����
void encrypt_block(unsigned char State[4][4], unsigned long RoundKeys[44]);
//����������� �����
void decrypt_block(unsigned char State[4][4], unsigned long RoundKeys[44]);


//����������� ����� ������.
void RotWord(unsigned long *dword);

//������ ������ ������� State S-Box
void SubBytes(unsigned char State[4][4]);
//������ ������ ������� State InvS-Box
void InvSubBytes(unsigned char State[4][4]);

//������ ������ 4 �������� �����  S-Box
void SubWord(unsigned long  *dword);
//������ ������ 4 �������� �����  InvS-Box
void InvSubWord(unsigned long  *dword);

//��������� ������� ������� State �� ������������� ������� C
void MixColumns(unsigned char State[4][4]);
//��������� ������� ������� State �� ������������� ������� C
void InvMixColumns(unsigned char State[4][4]);

void InvShiftRows(unsigned char State[4][4]);


unsigned long g(unsigned long key, int round_num);
//���������� �������� ����� �� 10 �������
void Key_Expansion(unsigned char InputKey[4][4], unsigned long RoundKeys[44]);

// State XOR RoundKey
void AddRoundKey(unsigned char State[4][4], unsigned long RoundKeys[44], int num_key);

//����������� �������������� ������ � �������� ����.
void copy_final_text(FILE *to, FILE *from, char final_fname[256]);
//�������� ������ �����, �������� ��������� ��������������� �����.
void delete_excess_zeroes(FILE *zero_file, char final_fname[]);

//���������� ������ � ������� AES-128
void encrypt(char final_fname[256]);
//������������� ������ � ������� AES-128
void decrypt(char  file_fname[256]);
