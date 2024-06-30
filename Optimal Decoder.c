#define _CRT_SECURE_NO_WARNINGS
#include<windows.h>
#include<math.h>
#include <stdio.h>
#define H 288
#define W 352
#define SIZE (W*H*3/2)
#define SIZEY (W*H)
#define Q 10
#define MAX 255
#define B 16// BLOCK �� �� ����
#define BLOCK (B*B)
//Optimal Decoder



unsigned char* Horizontal_Decoder(int* frameRecon) {//frameRecon: �ؽ�Ʈ ���Ͽ��� �о�� ������ ������ ��.
	unsigned char* frameH = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int e;
	int idx = 0;
	int idx_pre;

	for (int i = 0; i < H / B; i++) {//block ��
		for (int j = 0; j < W / B; j++) {//block �� 
			for (int k = 0; k < B; k++) {//block���� ��
				for (int p = 0; p < B; p++) {//block���� ��
					idx = W * (16 * i + k) + (16 * j + p);
					e = frameRecon[idx];
					e *= Q;
					if (j == 0) {//�� ���� ���� ���
						frameH[idx] = e + 128;
					}
					else {
						if (p == 0)//�� �ȿ��� ���� �ٲ� ������ prediction �� ������Ʈ.
							idx_pre = W * (16 * i + k) + (16 * j + p - 1);
						frameH[idx] = e + frameH[idx_pre];

					}
				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)
		frameH[i] = 128;
	return frameH;
}

unsigned char* Vertical_Decoder(int* frameRecon) {
	unsigned char* frameV = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int idx = 0;
	int idx_pre;
	int e;
	for (int j = 0; j < W / B; j++) {//block ��
		for (int i = 0; i < H / B; i++) {//block �� 
			for (int p = 0; p < B; p++) {//block���� ��
				for (int k = 0; k < B; k++) {//block���� ��
					idx = W * (16 * i + k) + 16 * j + p;
					if (i == 0) {//�� ���� ���� ���
						e = frameRecon[idx];
						e *= Q;
						frameV[idx] = e + 128;

					}
					else {
						if (k == 0)//�� �ȿ��� ���� �ٲ��� ���� prediction �� ������Ʈ.
							idx_pre = W * (16 * i + k - 1) + 16 * j + p;
						e = frameRecon[idx];
						e *= Q;
						frameV[idx] = e + frameV[idx_pre];

					}
				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)
		frameV[i] = 128;
	return frameV;
}

unsigned char* DC_Decoder(int* frameRecon) {
	unsigned char* frameDC = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int e;
	int idx;
	int total = 0, avg;
	for (int i = 0; i < H / B; i++) {
		for (int j = 0; j < W / B; j++) {
			for (int k = 0; k < B; k++) {
				for (int p = 0; p < B; p++) {
					total = 0;
					idx = W * (16 * i + k) + (16 * j + p);
					e = frameRecon[idx];
					e *= Q;

					if (i == 0 && j == 0) {//������ �ȼ����� ���� ���
						frameDC[idx] = e + 128;

					}
					else if ((i == 0) && (j != 0)) {//���� �ȼ��鸸 ����� �� ���� ���
						if (k == 0 && p == 0) {
							for (int z = 0; z < B; z++) {
								total += frameDC[idx + W * z - 1];
							}
							avg = total / 16;
						}
						frameDC[idx] = e + avg;
					}
					else if ((i != 0) && (j == 0)) {//���� �ȼ��鸸 ����� �� ���� ���
						if (k == 0 && p == 0) {
							for (int z = 0; z < B; z++) {
								total += frameDC[idx - W + z];
							}
							avg = total / 16;
						}
						frameDC[idx] = e + avg;
					}
					else {//���ʰ� ���� �ȼ����� ��� �̿��� �� �ִ� ���
						if (k == 0 && p == 0) {
							for (int z = 0; z < B; z++) {
								total += frameDC[idx + W * z - 1];
								total += frameDC[idx - W + z];
							}
							avg = total / 32;
						}
						frameDC[idx] = e + avg;
					}

				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)
		frameDC[i] = 128;
	return frameDC;
}

int main() {
	FILE* inputfile = fopen("bitstream_optimal.txt", "r");
	FILE* outputfile = fopen("Optimal_Decoder.yuv", "wb");
	if (inputfile == NULL) {
		printf("There is no input file.\n");
		return;
	}
	int mseIdx;
	unsigned char* frameQ = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int* frameRecon = (int*)malloc(sizeof(int) * SIZE);
	fread(&mseIdx, sizeof(int), 1, inputfile);//� ����� ����ߴ��� ���� �б�
	fread(frameRecon, sizeof(int), SIZEY, inputfile);//�ؽ�Ʈ ���� �б� 
	if (mseIdx == 0) {
		printf("Horizontal is selected.\n");
		frameQ = Horizontal_Decoder(frameRecon);
	}
	else if (mseIdx == 1) {
		printf("Vertical is selected.\n");
		frameQ = Vertical_Decoder(frameRecon);
	}
	else if (mseIdx == 2) {
		printf("DC is selected.\n");
		frameQ = DC_Decoder(frameRecon);

	}
	fwrite(frameQ, sizeof(unsigned char), SIZE, outputfile);
	return 0;
}