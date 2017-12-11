#include"lab2.h"

void write_inst()
{
	for(int i=1;i<num_inst;i++)
	{
		fprintf(inst_new,"    instr %d: %s\n",i,inst[i]);
	}
	fclose(inst_new);
}

bool load_inst(char filename[])
{
	FILE *p=NULL;
	if(strlen(filename)==0)
	{
		printf("input the 3adr file name:");
		gets(filename);
	}
	p=fopen(filename,"r");
	
	if(p==NULL)
	{
		printf("%s is not exist",filename);
		return false;
	}
	char str[100];
	int n=1;
	int temp=0;
	num_inst=0;
	while(!feof(p))
	{
		fscanf(p,"%s %d%s ",str,&temp,str);
		if(temp==n)
		{
			fgets(str,100,p);
			str[strlen(str)-1]='\0';
			Flag[n][0]=0;
			Flag[n][1]=0;
			Flag[n][2]=0;
			strcpy(inst[n++],str);
		}
	}
	num_inst=n;
	fclose(p);
	return true;
}

void pause()
{
	char str[10];
	gets(str);
}

void show()
{
	for(int i=1;i<num_inst;i++)
		cout <<i<<" "<<inst[i]<<endl;
}