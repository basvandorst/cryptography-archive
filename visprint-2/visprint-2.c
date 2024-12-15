#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RES 500
#define NEQNS 4

typedef unsigned char byte;

byte pic[RES][RES][3];

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

void initeqn(int n, double s, double t, double a, double b, double u,
    double v)
{
    eqns[n][0] = s*(1+a*b); eqns[n][1] = s*b;
    eqns[n][2] = t*a; eqns[n][3] = t;
    eqns[n][4] = u; eqns[n][5] = v;
}

void initeqns(void)
{
    int i;
    double s,t,a,b,u,v;

    for(i=0;i<NEQNS;++i) {
	s = (hashd[i][0])*0.3/64.0 + 0.3;
	t = (hashd[i][1])*0.3/64.0 + 0.3;
	a = (hashd[i][2])/32.0 - 1.0;
	b = (hashd[i][3])/32.0 - 1.0;
	u = (hashd[i][4])/32.0 - 0.25;
	v = (hashd[i][5])/32.0 - 0.25;
	initeqn(i,s,t,a,b,u,v);
    }
}

int gfcn = 0;
int colour = 0;

void nextpoint(int x, int y, int *nx, int *ny)
{
    int fcn = random()%NEQNS;
    if (colour && (colour>1 || ((random() & 1) == 0))) gfcn = fcn;
    *nx = eqns[fcn][0]*x + eqns[fcn][1]*y + eqns[fcn][4]*RES;
    *ny = eqns[fcn][2]*x + eqns[fcn][3]*y + eqns[fcn][5]*RES;
}

void ifs(void)
{
    int x = random()%RES;
    int y = random()%RES;
    int i,j,k;

    if (colour) {
	for(i=0;i<RES;++i) for(j=0;j<RES;++j) for(k=0;k<3;k++)
	    pic[i][j][k]=255;
    } else {
	for(i=0;i<RES;++i) for(j=0;j<RES;++j) pic[i][j][0]=0;
    }
    for(i=0;i<10*RES*RES;++i)
    {
	int nx,ny;
	nextpoint(x,y,&nx,&ny);
	if (i>RES*50 && nx+RES/2>=0 && nx+RES/2<RES &&
	    ny+RES/2>=0 && ny+RES/2<RES) {
	    if (colour) {
		if (((gfcn + 1) & 3) >= 2 && pic[nx+RES/2][ny+RES/2][0] != 0)
		    pic[nx+RES/2][ny+RES/2][0]--;
		if ((gfcn & 1) == 0 && pic[nx+RES/2][ny+RES/2][1] != 0)
		    pic[nx+RES/2][ny+RES/2][1]--;
		if (gfcn != 2 && pic[nx+RES/2][ny+RES/2][2] != 0)
		    pic[nx+RES/2][ny+RES/2][2]--;
	    } else {
		    pic[nx+RES/2][ny+RES/2][0] = 1;
	    }
	}
	x=nx; y=ny;
    }
}

void writepnm(void)
{
    int i,j;
    if (colour) {
	printf("P6\n%d %d\n255\n",RES,RES);
	fwrite (pic, 1, RES * RES * 3, stdout);
    } else {
	printf("P1\n%d %d\n",RES,RES);
	for(i=0;i<RES;++i) {
	    for (j=0;j<RES;++j) {
		printf("%d",pic[i][j][0]);
	    }
	    printf("\n");
	}
    }
}

int main(int argc, char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "-c")) colour = 1;
    if (argc > 1 && !strcmp(argv[1], "-C")) colour = 2;

    srandom(time(NULL));
    inithashd();
    initeqns();
    ifs();
    writepnm();
    return 0;
}
