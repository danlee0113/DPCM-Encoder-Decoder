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
//main decoder
#include "Horizontal Block Decoder.h"
#include "Vertical Block Decoder.h"
#include "DC Block Decoder.h"
#include "Inter Block Decoder.h"

int main() {
    FILE* inputfile = fopen("bitstream_inter_block.txt", "r");
    FILE* outputfile = fopen("Optimal_Inter_Decoder.yuv", "wb");
    if (inputfile == NULL) {
        printf("There is no input file.\n");
        return;
    }
    int mseIdx[H * W / BLOCK];
    int r[SIZE];
    int idx;
    int BLoc;
    unsigned char* frameQ = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);//최종 ouptutfile 담을 배열.
    unsigned char* frameH = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    unsigned char* frameV = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    unsigned char* frameDC = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    unsigned char* frameInter = (unsigned char*)malloc(sizeof(unsigned char) * SIZE);
    int* frameRecon = (int*)malloc(sizeof(int) * SIZE);//텍스트 파일 읽어놓은거 저장한 것.
    for (int f = 0; f < 2; f++) {
        int cntH = 0, cntV = 0, cntDC = 0;
        for (int i = 0; i < H / B; i++) {
            for (int j = 0; j < W / B; j++) {
                fread(&mseIdx[i * W / B + j], sizeof(int), 1, inputfile);
                for (int k = 0; k < B; k++) {
                    for (int p = 0; p < B; p++) {
                        idx = W * (16 * i + k) + 16 * j + p;
                        fread(&frameRecon[idx], sizeof(int), 1, inputfile);
                    }
                }

            }
        }
        for (int i = 0; i < H / B; i++) {//18
            for (int j = 0; j < W / B; j++) {//22
                BLoc = i * W / B + j;
                if (mseIdx[BLoc] == 0) {//horizontal 
                    frameH = Block_Horizontal_Decoder(frameRecon, i, j);//frameH가 들어옴. 근데 매번 바뀔거 아니냐고... 블럭 단위로 받아서 다시 frameQ에 합치는게 나을 것 같음. 
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            frameQ[idx] = frameH[idx];
                        }
                    }
                    cntH++;
                }
                else if (mseIdx[BLoc] == 1) {//vertical
                    frameV = Block_Vertical_Decoder(frameRecon, i, j);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            frameQ[idx] = frameV[idx];
                        }
                    }
                    cntV++;
                }
                else if (mseIdx[BLoc] == 2) {//DC
                    frameDC = Block_DC_Decoder(frameRecon, i, j);
                    for (int k = 0; k < B; k++) {
                        for (int p = 0; p < B; p++) {
                            idx = W * (16 * i + k) + 16 * j + p;
                            frameQ[idx] = frameDC[idx];
                        }
                    }
                    cntDC++;
                }
                else if (mseIdx[BLoc] == 3) {
                    frameInter = Block_Inter_Decoder(frameRecon, i, j);
                }
            }
        }
        printf("H: %d\nV: %d\nDC: %d\n", cntH, cntV, cntDC);
        for (int i = SIZEY; i < SIZE; i++)
            frameQ[i] = 128;
        fwrite(frameQ, sizeof(unsigned char), SIZE, outputfile);
    }
    return 0;
}