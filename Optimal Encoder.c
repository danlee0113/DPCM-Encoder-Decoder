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
//Optimal Encoder

int rH[SIZEY];//Horizontal residual ������ �迭
int rV[SIZEY];//Vertical residual ������ �迭
int rDC[SIZEY];//DC residual ������ �迭
double mse0, mse1, mse2;//0:horizontal, 1:vertical, 2:DC

double mse(unsigned char* frameQ, unsigned char* frame) {
	double total = 0.0;
	int r;
	for (int i = 0; i < SIZEY; i++) {
		r = (double)(frameQ[i] - frame[i]) * (frameQ[i] - frame[i]);
		total += r;
	}
	double mse = total / SIZEY;
	return mse;
}
double psnr(double mse) {
	double psnr = mse != 0.0 ? 10.0 * log10(MAX * MAX / mse) : 99.99;
	return psnr;
}

unsigned char* Horizontal_Encoder(unsigned char* frame) {
	unsigned char* frameH = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int e;
	int idx;
	int idx_pre;

	for (int i = 0; i < H / B; i++) {//block ��
		for (int j = 0; j < W / B; j++) {//block �� 
			for (int k = 0; k < B; k++) {//block���� ��(�ȼ� ����)
				for (int p = 0; p < B; p++) {//block���� ��(�ȼ� ����)
					idx = W * (16 * i + k) + (16 * j + p);//���� �ε��� �� ���� 
					if (j == 0) {// �� ���� ���� ���
						e = frame[idx] - 128;
						e /= Q;
						rH[idx] = e;//���� �迭�� ���� ����.(������ ����� ���� fwrite�� ���� ����.)
						e *= Q;
						frameH[idx] = e + 128;

					}
					else {
						if (p == 0)//�� �ȿ��� ���� �ٲ� ������ prediction�� ����ϴ� �� ������Ʈ.
							idx_pre = W * (16 * i + k) + (16 * j + p - 1);
						e = frame[idx] - frameH[idx_pre];
						e /= Q;
						rH[idx] = e;
						e *= Q;
						frameH[idx] = e + frameH[idx_pre];

					}
				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)// chorma ���� 128�� �ʱ�ȭ
		frameH[i] = 128;

	mse0 = mse(frameH, frame);
	printf("Horizontal MSE : %.4f\n", mse0);// ������ psnr ���� ����� ������ Ȯ���ϱ� ���� psnr �� ���.
	return frameH;
}

unsigned char* Vertical_Encoder(unsigned char* frame) {
	unsigned char* frameV = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int idx;
	int idx_pre;
	int e;
	for (int j = 0; j < W / B; j++) {//block ��
		for (int i = 0; i < H / B; i++) {//block �� 
			for (int p = 0; p < B; p++) {//block���� ��
				for (int k = 0; k < B; k++) {//block���� ��
					idx = W * (16 * i + k) + 16 * j + p;//���� �ε��� �� ���� 
					if (i == 0) {//�� ���� ���� ��� 
						e = frame[idx] - 128;
						e /= Q;
						rV[idx] = e;//���� �迭�� ���� ����.(������ ����� ���� fwrite�� ���� ����.)
						e *= Q;
						frameV[idx] = e + 128;

					}
					else {
						if (k == 0)//�� ������ ���� �ٲ� ���� prediction�� ����� �� ������Ʈ.
							idx_pre = W * (16 * i + k - 1) + 16 * j + p;
						e = frame[idx] - frameV[idx_pre];
						e /= Q;
						rV[idx] = e;
						e *= Q;
						frameV[idx] = e + frameV[idx_pre];

					}
				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)
		frameV[i] = 128;
	
	mse1 = mse(frameV, frame);
	printf("Vertical MSE : %.4f\n", mse1);
	return frameV;
}

unsigned char* DC_Encoder(unsigned char* frame) {
	unsigned char* frameDC = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	int idx;
	int e;
	int total = 0;
	int avg = 0;
	for (int i = 0; i < H / B; i++) {//block ����
		for (int j = 0; j < W / B; j++) {//block ����
			for (int k = 0; k < B; k++) {//block ���� ����
				for (int p = 0; p < B; p++) {//block ���� ����
					total = 0;//����� �� �� ����� ���� 
					idx = W * (16 * i + k) + (16 * j + p);
					if (i == 0 && j == 0) {//���� ���� ���� ���� ��� 
						e = frame[idx] - 128;
						e /= Q;
						rDC[idx] = e;
						e *= Q;
						frameDC[idx] = e + 128;
					}

					else if ((i == 0) && (j != 0)) {//������ �ȼ��鸸 �̿��� �� �ִ� ���
						if (k == 0 && p == 0) {
							for (int z = 0; z < B; z++) {
								total += frameDC[idx + W * z - 1];
							}
							avg = total / 16;
						}

						e = frame[idx] - avg;
						e /= Q;
						rDC[idx] = e;
						e *= Q;
						frameDC[idx] = e + avg;
					}
					else if ((i != 0) && (j == 0)) {//������ �ȼ��鸸 �̿��� �� �ִ� ��� 
						if (k == 0 && p == 0) {
							for (int z = 0; z < B; z++) {
								total += frameDC[idx - W + z];
							}
							avg = total / 16;
						}

						e = frame[idx] - avg;
						e /= Q;
						rDC[idx] = e;
						e *= Q;
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
						e = frame[idx] - avg;
						e /= Q;
						rDC[idx] = e;
						e *= Q;
						frameDC[idx] = e + avg;
					}

				}
			}
		}
	}
	for (int i = SIZEY; i < SIZE; i++)
		frameDC[i] = 128;

	mse2 = mse(frameDC, frame);
	printf("DC MSE : %.4f\n", mse2);
	return frameDC;
}
int FindMinMse(double mse0, double mse1, double mse2) {//������ mse���� ���ϱ� ���� �Լ�.
	int mseIdx;
	if (mse0 < mse1) {
		if (mse0 < mse2)
			mseIdx = 0;
		else//mse0>mse2
			mseIdx = 2;
	}
	else {//mse1<mse0
		if (mse2 > mse1)
			mseIdx = 1;
		else//mse2<mse1
			mseIdx = 2;
	}
	return mseIdx;
}
int main() {
	FILE* inputfile = fopen("FOREMAN_CIF30.yuv", "rb");
	FILE* outputfile = fopen("Optimal_Encoder.yuv", "wb");
	FILE* txt = fopen("bitstream_optimal.txt", "w");
	if (inputfile == NULL) {
		printf("There is no input file.\n");
		return;
	}
	unsigned char* frame = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
	unsigned char* frameH = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//Horizontal ������ ������
	unsigned char* frameV = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//Vertical ������ ������
	unsigned char* frameDC = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//DC ������ ������

	fread(frame, sizeof(unsigned char), SIZE, inputfile);
	int e;
	int total = 0;
	int avg = 0;
	int idx_pre;
	int idx;

	int mseIdx;
	frameH = Horizontal_Encoder(frame);
	frameV = Vertical_Encoder(frame);
	frameDC = DC_Encoder(frame);

	mseIdx = FindMinMse(mse0, mse1, mse2);

	fwrite(&mseIdx, sizeof(int), 1, txt);
	if (mseIdx == 0) {
		printf("Horizontal is selected.\n");
		for (int i = 0; i < SIZEY; i++) {
			fwrite(&rH[i], sizeof(int), 1, txt);
		}
		fwrite(frameH, sizeof(unsigned char), SIZE, outputfile);
	}
	else if (mseIdx == 1) {
		printf("Vertical is selected.\n");
		for (int i = 0; i < SIZEY; i++) {
			fwrite(&rV[i], sizeof(int), 1, txt);
		}
		fwrite(frameV, sizeof(unsigned char), SIZE, outputfile);
	}
	else if (mseIdx == 2) {
		printf("DC is selected.\n");
		for (int i = 0; i < SIZEY; i++) {
			fwrite(&rDC[i], sizeof(int), 1, txt);
		}
		fwrite(frameDC, sizeof(unsigned char), SIZE, outputfile);
	}
	return 0;
}