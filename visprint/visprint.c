#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RES 200
#define NEQNS 4

typedef unsigned char byte;

byte pic[RES][RES];

double eqns[NEQNS][6];
byte hashd[NEQNS][6];

void inithashd(void)
{
    byte buf[8];
    int i;

    for(i=0;i<NEQNS;++i) {
        int j;

        fread(buf, 8, 1, stdin);
        for(j=0;j<8;++j) {
            byte c = buf[j];
            if (c >= '0' && c <= '9') buf[j] = c-'0';
            else if (c >= 'a' && c <= 'f') buf[j] = c-('a'-10);
            else if (c >= 'A' && c <= 'F') buf[j] = c-('A'-10);
            else buf[j] = 0;
        }
        hashd[i][0] = ( buf[0] << 2 ) | ( buf[1] >> 2 );
        hashd[i][1] = ( (buf[1]&3) << 4 ) | buf[2];
        hashd[i][2] = ( buf[3] << 2 ) | ( buf[4] >> 2 );
        hashd[i][3] = ( (buf[4]&3) << 4 ) | buf[5];
        hashd[i][4] = buf[6];
        hashd[i][5] = buf[7];
    }

}
void initeqn(int n, double x1, double y1, double x2, double y2, double xc,
    double yc)
{
    eqns[n][0] = x2-x1; eqns[n][1] = xc+xc-x2-x1;
    eqns[n][2] = y2-y1; eqns[n][3] = yc+yc-y2-y1;
    eqns[n][4] = x1; eqns[n][5] = y1;
}

void initeqns(void)
{
    int i;
    double x1,x2,y1,y2,xc,yc;

    for(i=0;i<NEQNS;++i) {
        x1 = (hashd[i][0])/64.0;
        y1 = (hashd[i][1])/64.0;
        x2 = (hashd[i][2])/64.0;
        y2 = (hashd[i][3])/64.0;
        xc = (hashd[i][4])/32.0 + 0.25;
        yc = (hashd[i][5])/32.0 + 0.25;
        initeqn(i,x1,y1,x2,y2,xc,yc);
    }
}

void nextpoint(int x, int y, int *nx, int *ny)
{
    int fcn = random()%NEQNS;
    *nx = eqns[fcn][0]*x + eqns[fcn][1]*y + eqns[fcn][4]*RES;
    *ny = eqns[fcn][2]*x + eqns[fcn][3]*y + eqns[fcn][5]*RES;
}

void ifs(void)
{
    int x = random()%RES;
    int y = random()%RES;
    int i,j;

    for(i=0;i<RES;++i) for(j=0;j<RES;++j) pic[i][j]=0;
    for(i=0;i<2000;++i)
    {
        int nx,ny;
        nextpoint(x,y,&nx,&ny);
        if (i>100 && nx>=0 && nx<RES && ny>=0 && ny<RES) pic[nx][ny] = 1;
        x=nx; y=ny;
    }
}

void writepnm(void)
{
    int i,j;
    printf("P1\n%d %d\n",RES,RES);
    for(j=0;j<RES;++j) {
        for (i=0;i<RES;++i) {
            printf("%d",pic[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    srandom(time(NULL));
    inithashd();
    initeqns();
    ifs();
    writepnm();
    return 0;
}
