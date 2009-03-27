/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "pho.h"

lookup(u_char *s, char *num, char *typ)
{
	int i;
	char tt[3], *pp;

	if (*s < 128)
		return *s-'0';
	tt[0]=s[0];
	tt[1]=s[1];
	tt[2]=0;
	for(i=0;i<4;i++)
		if (pp=strstr(pho_chars[i],tt)) break;
	if (!pp) return 0;
	*typ=i;
	*num=(pp-pho_chars[i])/2;
}

void prph(u_short kk)
{
u_int k1,k2,k3,k4;
		k4=(kk&7)<<1;
		kk>>=3;
		k3=(kk&15)<<1;
		kk>>=4;
		k2=(kk&3)<<1;
		kk>>=2;
		k1=(kk&31)<<1;
		printf("%c%c%c%c%c%c%c%c",
			pho_chars[0][k1], pho_chars[0][k1+1],
			pho_chars[1][k2], pho_chars[1][k2+1],
			pho_chars[2][k3], pho_chars[2][k3+1],
			pho_chars[3][k4], pho_chars[3][k4+1]);
}

main(int argc, char **argv)
{
FILE *fp,*fw;
char s[128];
int i,len,cou;
u_short kk;
u_char tt[3];
PHOKBM phkb;
char num, typ, chk;
char fnamesrc[40];
char fnameout[40];

if (argc < 2) {
	puts("file name expected");
	exit(1);
}
bzero(&phkb,sizeof(phkb));
strcpy(fnameout,argv[1]);
strcat(fnameout,".kbm");
strcpy(fnamesrc,fnameout);
strcat(fnamesrc,".src");
if ((fp=fopen(fnamesrc,"r"))==NULL) {
	printf("Cannot open %s\n", fnamesrc);
	exit(1);
}

fgets(s,sizeof(s),fp);
len=strlen(s);
s[len-1]=0;
strcpy(phkb.selkey, s);

while (!feof(fp)) {

	fgets(s,sizeof(s),fp);
	len=strlen(s);
	if (!len) break;
	if (s[len-1]=='\n') s[--len]=0;
	if (!len) break;
	lookup(s,&num,&typ);
	chk=s[3];
	if (chk>='A' && chk<='Z') chk+=32;
	for(i=0;i<3;i++)
		if (!phkb.phokbm[chk][i][0]) {
			phkb.phokbm[chk][i][0]=num;
			phkb.phokbm[chk][i][1]=typ;
//			printf("%c %d %d\n", chk, num, typ);
			break;
		}
}
fclose(fp);

if ((fp=fopen(fnameout,"w"))==NULL) {
	printf("Cannot create %s\n", fnameout);
	exit(1);
}

fwrite(&phkb,sizeof(phkb),1,fp);
fclose(fp);
exit(0);
}
