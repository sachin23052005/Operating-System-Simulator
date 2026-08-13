#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

void EXECUTE();
void INIT();
void LOADINSTRUCTIONSETINTOMEMORY();
void CHECKERRORS();
void READ_FILE();
void LOADINSTRUCTIONTOBUFFER();
void PRINTMEMORY();
void PRINTBUFFER();

int GETRANDOM(int min, int max) {
    int rd_num = rand() % (max - min + 1) + min;
    return rd_num;
}

char M[4][300], IR[4], R[4], BUFF[4][10], ttl[4], tll[4];
int m, IC, SI, PCI=0, DCI=0, ptr, bp=0, c=1, temp=0, x=0, TTL=0, TLL=0, PI, TTC, PTR, errors, LLC;
int PTE, RA, ICR, ptl, toggle[300], v, pe;
FILE *input, *output;
char line[256], job[4], line2[256];
int randAd[30] = {14, 3, 25, 7, 0, 27, 9, 12, 21, 5, 17, 10, 28, 1, 23, 18, 19, 26, 2, 4, 15, 6, 13, 24, 8, 29, 22, 16, 11, 20};

int MAP(int va){
    int ra=0;
    for(int i=PTR; i<PTR+10; i++){
        if((M[0][i]-'0')*10 + (M[1][i] - '0') == va){
            ra = (M[2][i] - '0') *10 + (M[3][i] - '0');
            return ra;
        }
    }
    return ra;
}

void PRINTMEMORY(){
    for(int i=0; i<300; i++){
        printf("%d ", i);
        for(int j=0; j<4; j++){
            printf("%c ", M[j][i]);
        }
        printf("\n");
    }
}

void PRINTBUFFER(){
    for(int i=0; i<10; i++){
        printf("%d ", i);
        for(int j=0; j<4; j++){
            printf("%c ", BUFF[j][i]);
        }
        printf("\n");
    }
}

void LOADINSTRUCTIONTOBUFFER(char*s, int x, int y){
    int ap=0;
    for(int i=40*(x-y); i<40+(40*(x-y)); i+=4){
        if(s[i] == '\n') break;
        BUFF[0][ap] = s[i];
        if(s[i] == 'H'){
            i-=3;
        }else{
            BUFF[1][ap] = s[i+1];
            BUFF[2][ap] = s[i+2];
            BUFF[3][ap] = s[i+3];
        }
        ap++;
    }
    LOADINSTRUCTIONSETINTOMEMORY();
    if(y > 0){
        LOADINSTRUCTIONTOBUFFER(s, x, y-1);
    }
}

void INIT(){
    for(int i=0; i<4; i++){
        for(int j=0; j<300; j++){
            M[i][j] = '-';
            toggle[j] = 0;
        }
    }
    for(int i=0; i<4; i++){
        IR[i] = '-';
        R[i] = '-';
        for(int j=0; j<10; j++){
            BUFF[j][i] = '-';
        }
    }
    m=0, SI=0, IC=0, TTL=0, TLL=0, TTC=0, PI=0;
    LLC=0;
    ptr=1;
    PTR = randAd[0] * 10;
    errors = 0;
    ICR=0;
    ptl = 0;
    pe=0;
}

void EXECUTE() {
    CHECKERRORS();
    if (errors == 1) return;
    for (int i = 0; i < 4; i++) {
        IR[i] = M[i][m];
    }

    int IC, t, lf = 0, e = 0;

    switch (IR[0]) {
        case 'G':
            if (IR[1] != 'D') {
                PI = 1;
                break;
            }
            ICR++;
            TTC += 2;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            toggle[IC] = 1;
         //   M[0][PTR + ptl] = IC / 10 + '0';
          //  M[1][PTR + ptl] = IC % 10 + '0';
            t = randAd[ptr] * 10;
            M[2][PTR + ptl] = randAd[ptr] / 10 + '0';
            M[3][PTR + ptl] = randAd[ptr] % 10 + '0';
            ptr++;
            ptl++;
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }

            if (BUFF[0][bp] == '-') {
                PI = 4;
                break;
            }

            while (lf == 0) {
                for (int i = 0; i < 4; i++) {
                    if (BUFF[i][bp] != '\n') {
                        M[i][t + e] = BUFF[i][bp];
                    } else {
                        lf = 1;
                    }
                }
                bp++;
                e++;
            }
            break;

        case 'P':
            if (IR[1] != 'D') {
                PI = 1;
                break;
            }
            ICR++;
            TTC++;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            t = MAP(IC) * 10;
            if (toggle[IC] == 0) {
                PI = 3;
                break;
            }
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }

            output = fopen("output.txt", "a");
            for (int j = 0; j < 10; j++) {
                for (int i = 0; i < 4; i++) {
                    if (M[i][t + j] == '\n' || M[i][t + j] == '-') {
                        break;
                    }
                    fprintf(output, "%c", M[i][t + j]);
                }
            }
            fprintf(output, "\n");
            fclose(output);
            break;

        case 'L':
            if (IR[1] != 'R') {
                PI = 1;
                break;
            }
            ICR++;
            TTC++;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            if (toggle[IC] == 0) {
                PI = 3;
                break;
            }
            t = MAP(IC / 10 * 10) * 10 + IC % 10;
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }
            for (int i = 0; i < 4; i++) {
                R[i] = M[i][t];
            }
            break;

        case 'S':
            if (IR[1] != 'R') {
                PI = 1;
                break;
            }
            ICR++;
            TTC += 2;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            toggle[IC] = 1;
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }
            t = MAP(IC / 10 * 10) * 10 + IC % 10;
            for (int i = 0; i < 4; i++) {
                M[i][t] = R[i];
            }
            break;

        case 'C':
            if (IR[1] != 'R') {
                PI = 1;
                break;
            }
            ICR++;
            TTC++;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            if (toggle[IC] == 0) {
                PI = 3;
                break;
            }
            t = MAP(IC / 10 * 10) * 10 + IC % 10;
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }
            for (int i = 0; i < 4; i++) {
                if (M[i][t] != R[i]) {
                    c = 0;
                }
            }
            break;

        case 'B':
            if (IR[1] != 'T') {
                PI = 1;
                break;
            }
            ICR++;
            TTC++;
            IC = (IR[2] - '0') * 10 + (IR[3] - '0');
            if (toggle[IC] == 0) {
                PI = 3;
                break;
            }
            if (IC < 0 || IC > 99) {
                PI = 2;
                break;
            }
            break;

        case 'H':
            ICR++;
            TTC++;
            return;
            break;

        default:
            PI = 1;
            break;
    }

    m++;
    if (v < 10) {
        v++;
        EXECUTE();
    }
}

void READ_FILE() {
    input = fopen("input.txt", "r");
    while (fgets(line, sizeof(line), input) != NULL) {
        if(PCI == 1){
            x=0;
            for(int i=0; i<strlen(line); i++){
                if(line[i] != ' '){
                    line2[x] = line[i];
                    x++;
                }
            }

            LOADINSTRUCTIONTOBUFFER(line2, strlen(line2)/40, strlen(line2)/40);
            PCI=0;
        }

        if(DCI == 1){
            if(line[0] == '$'){
                DCI = 0;
                bp=0;
                m=((M[2][PTR] - '0') * 10 + (M[3][PTR] - '0')) * 10;
                v=0;
                EXECUTE();
            }else{
                int j=0;
                for(int i=0; i<strlen(line); i++){
                    if(j>3){
                        j=0;
                        bp++;
                    }
                    BUFF[j][bp] = line[i];
                    j++;
                }
                bp++;
            }
        }

        if(line[0] == '$'){
            switch(line[1]){
                case 'A':
                    INIT();
                    for(int i=0; i<4; i++){
                        job[i] = line[5+i];
                        ttl[i] = line[10+i];
                        tll[i] = line[15+i];
                    }
                    for(int i=0; i<4; i++){
                        TTL *= 10;
                        TTL += ttl[i] - '0';
                        TLL *= 10;
                        TLL += tll[i] - '0';
                    }
                    PCI=1;
                    break;

                case 'D':
                    DCI=1;
                    bp=0;
                    break;

                default:
                    output = fopen("output.txt", "a");
                    fprintf(output, "\n\n");
                    fclose(output);
                    PCI=0;
                    DCI=0;
            }
        }
    }
}

void LOADINSTRUCTIONSETINTOMEMORY(){
    m=randAd[ptr] * 10;
    M[2][PTR+ptl] = (randAd[ptr]/10) + '0';
    M[3][PTR+ptl] = (randAd[ptr]%10) + '0';
    ptr++;
    ptl++;
    int y=0;

    while(BUFF[0][y] != '-' && y<10){
        M[0][m] = BUFF[0][y];
        if(BUFF[0][y] == 'P' && BUFF[1][y] == 'D')++LLC;
        if(BUFF[0][y] != 'H'){
            M[1][m] = BUFF[1][y];
            M[2][m] = BUFF[2][y];
            M[3][m] = BUFF[3][y];
        }
        y++;
        m++;
    }

    for(int i=0; i<10; i++){
        for(int j=0; j<4; j++){
            BUFF[j][i] = '-';
        }
    }
    m=0;
}

void CHECKERRORS(){
    if(TTC+1 > TTL){
        errors = 1;
        output = fopen("output.txt", "a");
        fprintf(output, "Error:TIME LIMIT EXCEEDED \n");
        fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n TLL:%d", ICR, IR, TTC, TTL, LLC,TLL);
        fclose(output);
    }
    if(LLC > TLL){
        errors = 1;
        output = fopen("output.txt", "a");
        fprintf(output, "Error:LINE LIMIT EXCEEDED \n");
        fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d TLL:%d\n", ICR, IR, TTC, TTL, LLC,TLL);
        fclose(output);
    }
    if(PI>0){
        errors = 1;
        if(PI==1){
            output = fopen("output.txt", "a");
            fprintf(output, "Error:OPCODE Error \n");
            fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d TLL:%d\n", ICR, IR, TTC, TTL, LLC,TLL);
            fclose(output);
        }
        if(PI==2){
            output = fopen("output.txt", "a");
            fprintf(output, "Error:OPERAND Error\n");
            fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, TTC, TTL, LLC);
            fclose(output);
        }
        if(PI==3){
            output = fopen("output.txt", "a");
            fprintf(output, "Error:INVALID PAGE FAULT \n");
            fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, TTC, TTL, LLC);
            fclose(output);
        }
        if(PI==4){
            output = fopen("output.txt", "a");
            fprintf(output, "Error:OUT OF DATA \n");
            fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, TTC, TTL, LLC);
            fclose(output);
        }
    }
}

int main(){
    READ_FILE();
    PRINTMEMORY();
    fclose(input);
    return 0;
}
