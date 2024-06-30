# DPCM based Encoder/ Decoder

이 실습은 다양한 DPCM 기반의 encoding/decoding 압축 기법을 이용해 구현했습니다. 

사용된 동영상은 FOREMAN_CIF30.yuv 파일입니다. 



## Appendix
    이 프로젝트에서는 Inter/ Intra 두 가지 방법으로 encoder/decoder를 만들었습니다. 
    모든 Encoder/Decoder는 16*16 블럭 단위로 압축하고 복원했습니다. 
    Intra는 한 프레임 내에서 압축과 복원을 진행하며, Inter는 이전 프레임의 같은 위치의 블럭을 이용해 압축과 복원을 진행합니다.
    모든 Encoder와 Decoder에는 Intra에 속하는 DC, Horizontal, Vertical과 Inter를 합쳐서 모두 네 가지의 방법을 사용했습니다.
    Optimal Encoder.c 와 Optimal Decoder.c는 Intra의 방법만 사용했으며, Inter Encoder.c 와 Inter Decoder.c는 4가지 방법을 모두 사용해 구현했습니다. 

    DC는 블럭 픽셀들의 평균을 이용해 잔차를 구하고 압축에 이용하는 방식이며, Horizontal과 Vertical은 각각 수평/수직 방향으로 압축을 진행하는 방식입니다. 
    각 블럭 별로 3가지 또는 4가지 방법을 모두 적용해 MSE 값을 구한후, MSE값이 최소로 나오는 방법을 채택해 압축에 이용했습니다. 
    
