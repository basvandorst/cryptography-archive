/*   Copyright (c) 1988-1993 Shamus Software Ltd. */

#include <stdio.h>
#include <miracl.h>

#define MAXKEY 40

static FILE *fp;
static char licensee[100];
static char roottext[200];
static char compotext[200];
static char work[200];
static big root,compo,licensenum,key,secret_num;
static char *fixed={"AUTHORISED PC-SCRAMBLER USER -> "};

static int secret(buff,maxlen)
char *buff;
int maxlen;
{ /* get secret string */
    int cnt,ch;
    cnt=0;
    while ((ch=getch())!=13 && cnt<maxlen)
    { /* until RETURN pressed */
        if (ch==27)
        { /* ESC */
            cnt=0;
            break;
        }
        if (ch==127 || ch==8)
        { /* delete */
            if (cnt==0) continue;
            cnt--;
            continue;
        }
        buff[cnt++]=ch;
    }
    buff[cnt]=0;
    return cnt;
}

static void onewayhash(string,n,x)
char *string;      
big n,x;
{ /* nonlinear (portable) permutation *
   * x=(fixed|string xor R(n))^2 mod n      */
    int i,len;
    char t;
    static char buff[256];
    strcpy(work,fixed);
    strcat(work,string);
    INPLEN=otstr(n,buff);
    for (i=0;i<INPLEN/2;i++)
    { /* swap bytes of buff */
        t=buff[i];
        buff[i]=buff[INPLEN-i-1];
        buff[INPLEN-i-1]=t;
    }
    len=strlen(work);
    for (i=0;i<len;i++) buff[i]^=work[i];
    instr(x,buff);
    mad(x,x,x,n,n,x);
}

void main(argc,argv)
int argc;
char **argv;
{
    static char kstname[20],pass[MAXKEY+1],check[MAXKEY+1];
    int i,r,ch,len,liclen;

    mirsys(200,256); 
    root=mirvar(0);
    compo=mirvar(0);
    key=mirvar(0);
    secret_num=mirvar(0);
    licensenum=mirvar(0);
    IOBASE=60;

    argv++;
    argc--;
    if (argc==1) strcpy(kstname,argv[0]);
    else         strcpy(kstname,"scramble");
    strcat(kstname,".kst");
    ch=0;
    if ((fp=fopen(kstname,"r"))!=NULL)
    { /* read in key data */
        fgets(licensee,100,fp);
        liclen=strlen(licensee)-1;
        licensee[liclen]='\0';  /* remove CR */
        ch=fscanf(fp,"%s%s",roottext,compotext);
        fclose(fp);
    }
    else
    {
        printf("Key file %s does not exist\n",kstname);
        exit(0);
    }
    if (ch!=2 || strlen(compotext)<80 || strlen(roottext)<80)
    { /*  invalid file */
        printf("Invalid key file format\n"); 
        exit(0);
    }
    cinstr(root,roottext);
    cinstr(compo,compotext);
    power(root,3,compo,key);
    onewayhash(licensee,compo,licensenum);
    if (compare(key,licensenum)!=0)
    {
        printf("Invalid Identity - Already Clamped?\n");
        exit(0);
    }

    forever
    {   
        printf("Type password CAREFULLY\n");
        printf("Password: ");
        if ((len=secret(pass,MAXKEY))<8)
        { /* get secret password */
            printf("Password too short\n");
            printf("Should be at least 8 characters long\n");
            printf("and ideally 24 characters or more long\n");
            for (i=0;i<=MAXKEY;i++) pass[i]=0;
            continue;
        }
        printf("\nType in Password again: ");
        secret(check,MAXKEY);
        if (strcmp(pass,check)!=0)
        {
            printf("\nPassword typed incorrectly - start again\n");
            for (i=0;i<=MAXKEY;i++) pass[i]=check[i]=0;
            continue;
        }
        break;
    }
    INPLEN=len;
    instr(secret_num,pass);
    for (i=0;i<=MAXKEY;i++) pass[i]=check[i]=0;
    power(secret_num,64,compo,secret_num); /* hide password behind trapdoor */
    mad(root,secret_num,root,compo,compo,root);
    fp=fopen(kstname,"w");
    fputs(licensee,fp);
    fputc('\n',fp);
    cotnum(root,fp);
    cotnum(compo,fp);
    fclose(fp);
    zero(secret_num);
    mirexit();
    printf("\nKey file %s is now Clamped\n",kstname);
}

