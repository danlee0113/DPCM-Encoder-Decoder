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
#define B 16// BLOCK 한 변 길이
#define BLOCK (B*B)
#define F 1 //(전체 프레임 수)-1
//main Encoder
#include "Horizontal Block Encoder.h"
#include "Vertical Block Encoder.h"
#include "DC Block Encoder.h"
#include "Inter Block Encoder.h"

double mse0, mse1, mse2, mse3;//0:horizontal, 1:vertical, 2:DC, 3: Inter


int r[SIZEY];

double mseBlock(unsigned char* frameQ, unsigned char* frame, int startIdx) {
    double total = 0.0;
    int r, idx;//훑고 있는 블럭의 인덱스 
    for (int i = 0; i < B; i++) {
        for (int j = 0; j < B; j++) {
            idx = startIdx + W * i + j;
            r = (double)(frameQ[idx] - frame[idx]) * (frameQ[idx] - frame[idx]);
            total += r;
        }
    }
    double mse = total / BLOCK;
    return mse;
}
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
int MinMseIntra(double mse0, double mse1, double mse2) {
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
int MinMseInter(double mse0, double mse1, double mse2, double mse3) {
    int mseIdx;
    if (mse0 < mse1) {
        if (mse2 < mse0) {
            if (mse3 < mse2)
                mseIdx = 3;
            else//mse3>mse2
                mseIdx = 2;
        }
        else {//mse2,mse1>mse0
            if (mse0 < mse3)
                mseIdx = 0;
            else//mse0>mse3
                mseIdx = 3;
        }
    }
    else {//mse0>mse1
        if (mse2 < mse1) {//mse2<mse1<mse0
            if (mse3 < mse2)//mse3<mse2<mse1<mse0
                mseIdx = 3;
            else//mse2<mse1<mse0,mse3
                mseIdx = 2;
        }
        else {//mse0,mse2>mse1
            if (mse3 > mse1)
                mseIdx = 1;
            else//mse0,mse2>mse1>mse3
                mseIdx = 3;
        }

    }
    return mseIdx;
}
int FindMinMse(int f, double mse0, double mse1, double mse2, double mse3) {//최적의 mse값을 구하기 위한 함수.
    int mseIdx;
    if (f == 0) {//3이 나올수 없다.
        mseIdx = MinMseIntra(mse0, mse1, mse2);
    }
    else {
        mseIdx = MinMseInter(mse0, mse1, mse2, mse3);
    }
    return mseIdx;
}

int main() {
    FILE* inputfile = fopen("FOREMAN_CIF30.yuv", "rb");
    FILE* outputfile = fopen("Optimal_Inter_Encoder.yuv", "wb");
    FILE* txt = fopen("bitstream_inter_block.txt", "w");
    if (inputfile == NULL) {
        printf("There is no input file.\n");
        return;
    }
    unsigned char* frame = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    unsigned char* frameH = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//Horizontal 저장할 프레임
    unsigned char* frameV = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//Vertical 저장할 프레임
    unsigned char* frameDC = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//DC 저장할 프레임
    unsigned char* frameInter = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//Inter 저장할 프레임
    unsigned char* framePrev = (unsigned char*)malloc(sizeof(unsigned char) * SIZE * F);//전체 프레임-1 만큼의 프레임 수로 저장. -> Inter 예측에 필요함. 
    unsigned char* frameQ = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    for (int f = 0; f < 2; f++) {
        fread(frame, sizeof(unsigned char), SIZE, inputfile);
        int idx;
        int startIdx;
        int mseIdx[H * W / BLOCK];//블럭 개수만큼의 배열 
        int cntH = 0, cntV = 0, cntDC = 0, cntInter = 0;
        int BLoc;//mseIdx 배열에서 블럭의 위치를 나타내는 변수(Block Location의 줄임말)

        for (int i = 0; i < H / B; i++) {
            for (int j = 0; j < W / B; j++) {
                startIdx = W * 16 * i + 16 * j;
                BLoc = i * (W / B) + j;
                frameH = Block_Horizontal_Encoder(frame, i, j);
                frameV = Block_Vertical_Encoder(frame, i, j);
                frameDC = Block_DC_Encoder(frame, i, j);
                frameInter = Block_Inter_Encoder(frame, framePrev, i, j);
                mse0 = mseBlock(frameH, frame, startIdx);
                mse1 = mseBlock(frameV, frame, startIdx);
                mse2 = mseBlock(frameDC, frame, startIdx);
                mse3 = mseBlock(frameInter, frame, startIdx);
                mseIdx[BLoc] = FindMinMse(f, mse0, mse1, mse2, mse3);

                if (mseIdx[BLoc] == 0) {
                    cntH++;
                    fwrite(&mseIdx[BLoc], sizeof(int), 1, txt);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            r[idx] = rH[idx];
                            fwrite(&r[idx], sizeof(int), 1, txt);
                            frameQ[idx] = frameH[idx];
                        }
                    }
                }
                else if (mseIdx[BLoc] == 1) {
                    cntV++;
                    fwrite(&mseIdx[BLoc], sizeof(int), 1, txt);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = idx = W * (16 * i + k) + 16 * j + p;
                            r[idx] = rV[idx];
                            fwrite(&r[idx], sizeof(int), 1, txt);
                            frameQ[idx] = frameV[idx];
                        }
                    }
                }
                else if (mseIdx[BLoc] == 2) {
                    cntDC++;
                    fwrite(&mseIdx[BLoc], sizeof(int), 1, txt);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            r[idx] = rDC[idx];
                            fwrite(&r[idx], sizeof(int), 1, txt);
                            frameQ[idx] = frameDC[idx];
                        }
                    }
                }
                else if (mseIdx[BLoc] == 3) {//Inter
                    cntInter++;
                    fwrite(&mseIdx[BLoc], sizeof(int), 1, txt);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            r[idx] = rInter[idx];
                            fwrite(&r[idx], sizeof(int), 1, txt);
                            frameQ[idx] = frameInter[idx];

                        }
                    }
                }
            }
        }
        for (int i = SIZEY; i < SIZE; i++)
            frameQ[i] = 128;

        if (f != F) {
            for (int i = 0; i < SIZE; i++) {
                framePrev[i] = frameQ[i];
            }
        }
        printf("H: %d\nV: %d\nDC: %d\nInter: %d\n", cntH, cntV, cntDC, cntInter);
        printf("PSNR: %.4f\n", psnr(mse(frameQ, frame)));
        fwrite(frameQ, sizeof(unsigned char), SIZE, outputfile);
    }
    return 0;
}//frame 통째로 했을 때의 PSNR: 34.0530dB이지만, block 단위로 하면 PSNR: 34.2182dB로 소폭 상승함. 