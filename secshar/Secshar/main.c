/*
	Secshar
	Writen by Lance Cottrell
	Version 0.3a 10/15/94
	Distributed unger GNU Lisence
*/


#include "crypt.h"
#define maxim	65537
#define fuyi	65536
#define one	65535
#define round   8
#define maxcount 5
#define BLOCK 1024




void getkey(uskey)
short unsigned	uskey[9];
{
	long int		i,x;
	
		for (i=1;i<=8;i++){
		printf("enter the %d subkey block: ",i);
		scanf("%d",&x);
		uskey[i] = x;
		printf("The %d subkey is %u\n",i,uskey[i]);
	}  /* key entry */
}

void	get_text(pt)
unsigned	pt[5];
{
	long int	x;
	
		for(x=1;x<=4;x++) {
		printf("\n\nenter the %d plaintext block: ",x);
		scanf("%d",&pt[x]);
	}  /* text entry */
}


void put_shadows(shadow_type *shadows, int n, int m, char *rootname)
{
	static int i=-1,first = 1;
	static shadow_type *shadow_array[BLOCK];
	char	name[80];
	FILE	*fptr;
	int	j,k;

	if(shadows > (shadow_type *) 0) {
	  i++;
	  shadow_array[i] = (shadow_type *)malloc(m*sizeof(shadow_type));
	  for(j=0;j<m;j++) shadow_array[i][j] = shadows[j];
	} else {
	  for(k=0;k<m;k++) { /* write out one share at a time */
	    sprintf(name,"%s%d.shr",rootname,k);
	    if((fptr = fopen(name,"a"))==NULL) {
	      fprintf(stderr,"Problem opening %s\n",name);
	      exit(0);
	    }
	    if(first) {
	      fclose(fptr);
	      fptr = fopen(name,"w");
	      fprintf(fptr,"----Begin Share----\n");
	      fprintf(fptr,"Required_Shares: %d\n",n);
	    }
	    for(j=0;j<=i;j++){ /* write out each element of the share */
	      fprintf(fptr,"%d %u\n",shadow_array[j][k].x,shadow_array[j][k].y);
	    }
	    if(i < 0) {
	      fprintf(fptr,"----End Share----\n");
	    }
	    fclose(fptr);
	  }
	  i=-1;
	  first = 0;
	}
}


void split_it(char *name, int n, int m)
{
	/* split secret, n of m scheme */

	long int			i, j, k, x, length;
	unsigned	Z[7][10], DK[7][10], XX[5],TT[5],YY[5],ZZ[5];
	short unsigned uskey[9];
	char		line[BLOCK],answer[256];
	shadow_type	*shadows;
	unsigned	tmp[257];
	FILE		*secfile;
	

	if((secfile = fopen(name,"r"))==NULL) {
	  fprintf(stderr,"Could not open file %s\n",name);
	  exit(0);
	}
	while((length = read(fileno(secfile),line,BLOCK)) > 0) {
	  for(i=0;i<length;i++){
	    shadows = (shadow_type *)make_shadow(n,m,(long int)line[i]);
	    put_shadows(shadows,n,m,name);
	  }
	  put_shadows(NULL,n,m,name); /* write out to disk */
	}
	put_shadows(NULL,n,m,name); /* write out end of share marker */
}

int get_shadows(long int *n, shadow_type *shadows, char *rootname)
{
	static shadow_type *shadow_array[BLOCK];
	static long position[256],index,buff=-1,done=0;
	static char *names[256];
	char	name[80],line[80],*temp;
	FILE	*fptr;
	int	i,j;

	if (*n == 0) {
	  sprintf(name,"ls %s*.shr",rootname);
	  fptr = popen(name,"r");
	  i=-1;
	  while(fgets(name,80,fptr)!=NULL){
	    i++;
	    temp = malloc(strlen(name) * sizeof(char) +1);
	    temp[0] = 0;
	    strncat(temp,name,strlen(name)-1);
	    names[i] = temp;
	  }
	  fclose(fptr);
	  for(j=0;j<=i;j++){
	    if((fptr=fopen(names[j],"r"))==NULL) {
	      fprintf(stderr,"Could not open %s\n",names[j]);
	      exit(0);
	    }
	    do {
	      fgets(line,80,fptr);
	    } while (strncmp(line,"----Begin Share",15)!=0);
	    fgets(line,80,fptr);
	    sscanf(line,"%s %d",name,n);
	    if(*n>i+1) {
	      fprintf(stderr,"Not enough shares to reconstruct secret!\n");
	      fprintf(stderr,"%ld shares are required\n",*n);
	      fprintf(stderr,"The names of the shares must match the search criteria\n");
	      fprintf(stderr,"%s*.shr\n",rootname);
	      exit(0);
	    }
	    position[j] = ftell(fptr);
	    fclose(fptr);
	  }
	  index = BLOCK + 1;
	  for(i=0;i<BLOCK;i++) shadow_array[i] = malloc(*n * sizeof(shadow_type));
	} else {   /* End of if this is the first time through */
	  if(index > buff) {
	    if(done) return(0); /* buffer empty and we found End Share marker */
	    for (i=0;i<*n;i++){
	      fptr = fopen(names[i],"r");
	      fseek(fptr,position[i],0);
	      for(j=0;j<BLOCK;j++){
		if(fgets(line,80,fptr)==NULL) {
		  fprintf(stderr,"End of file %s reached without finding\n",names[i]);
		  fprintf(stderr,"the '----End Share----' ,marker\n");
		  fprintf(stderr,"The file was probably correupted.\n");
		  fprintf(stderr,"Get a new copy, or delete it and try\n");
		  fprintf(stderr,"again if you have engough shares.\n");
		  fclose(fptr);
		  exit(0);
		}
		if(strncmp(line,"----End Share--",15) != 0) {
		  sscanf(line,"%d %u",&shadow_array[j][i].x,&shadow_array[j][i].y);
		  buff = j;
		} else {
		  buff = j-1;
		  j=BLOCK;
		  done=1;
		}
	      }
	      position[i] = ftell(fptr);
	      fclose(fptr);
	    }
	    if(buff < 0) return(0);
	    index = 0;
	  } /* end if buffer needs refilling */
	  for(i=0;i<*n;i++) {
	    shadows[i] = shadow_array[index][i];
	  }
	  index++;
	}
	return(1);
}

void combine_it(char *name)
{
	long int			i, j, k, x, n=0,m;
	unsigned	Z[7][10], DK[7][10], XX[5],TT[5],YY[5],ZZ[5];
	short unsigned uskey[9];
	char		line[255],answer[256];
	shadow_type	*shadows;
	unsigned	tmp[257];
	FILE		*fptr;
	
	if((fptr = fopen(name,"w"))==NULL){
	  fprintf(stderr,"Unable to open output file %s\n",name);
	  exit(0);
	}
	get_shadows(&n,shadows,name);
	shadows = malloc(n * sizeof(shadow_type));
	while (get_shadows(&n,shadows,name)) {
	  fprintf(fptr,"%c",(char) combine_shadows(n,shadows));
	}
	fclose(fptr);
}

void test_sub()
{
	unsigned	i,numb;
	FILE	*fptr;
	
	fptr = fopen("random.test","w+");
	for(i=1;i<=1000;i++) {
	  numb = prand(1);
	  fprintf(fptr,"%u %u\n",i,numb);
	}
	fclose(fptr);
}

void oops()
{
	fprintf(stderr,"Incorrect command syntax.\n");
	fprintf(stderr,"\nThe command syntax is:\n");
	fprintf(stderr,"secsplit name needed-shares total-shares\n");
	fprintf(stderr,"secsplit -c rootname-to-combine\n");
}


main(int argc, char *argv[])
{
	char	*name;	
	int	needed, total;

	/* If the first argument is a filename,
	   then that file will be split up
	   into fname###.shr

	   If it is -c then use the next argument as
	   the root name of the file to be reconstructed.
	*/


	if(argc >= 2) {
	  if(strstr(argv[1],"-c")) {
	    if (argc == 3) {
	      name = argv[2];
	      combine_it(name);
	    } else {
	      oops();
	      exit(0);
	    }
	  } else {
	    if (argc == 4) {
	      name = argv[1];
	      needed = atoi(argv[2]);
	      total = atoi(argv[3]);
	      if(total > 250) {
		fprintf(stderr,"Maximum of 250 shares allowed.\n");
		exit(0);
	      }
	      if(needed > total) {
		fprintf(stderr,"Number of shares needed must be less than the total number of shares.\n");
		exit(0);
	      }
	      split_it(name,needed,total);
	    } else {
	      oops();
	      exit(0);
	    }
	  }
	} else {
	  oops();
	  exit(0);
	} /* end if */

	return 0;
}
