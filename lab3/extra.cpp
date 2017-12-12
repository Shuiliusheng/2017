#include"lab3.h"

int get_op(char src[],char dst[],int &reg)//获取操作数的类型，表达式，常数，变量（局部/全局）
{
	if(src[0]<='9'&&src[0]>='0'||src[0]=='-')//常数
	{
		strcpy(dst,src);
		return CONST;
	}
	else if(src[0]=='(')//表达式
	{
		sscanf(src,"(%d)",&reg);
		return EXPR;
	}
	else//变量
	{
		int l=strlen(src),i=0;
		for(i=0;i<l;i++)
		{
			if(src[i]=='#')
			{
				break;
			}
			else
				dst[i]=src[i];
		}
		dst[i]='\0';
		sscanf(&src[i+1],"%d",&reg);
		return VARR;
	}
}

int judge_expr(int n,char name[],int &value)
{
	int i=0;
	char op[100],fuc1[100],fuc2[100];
	sscanf(inst[n],"%s %s %s",op,fuc1,fuc2);
	int reg=0;
	if(strcmp(op,"move")==0)
	{
		int t=get_op(fuc1,fuc1,reg);
		strcpy(name,fuc2);
		if(t==1)
		{
			sscanf(fuc1,"%d",&value);
			return 1;
		}
		else
			return 2;
	}
	else if(strcmp(op,"store")==0)
	{
		int t=get_op(fuc1,fuc1,reg);
		char temp[100];
		sscanf(inst[n-1],"%s %s %s",op,temp,fuc2);
		if(strcmp(fuc2,"GP")==0&&strcmp(op,"add")==0)
			strcpy(name,temp);
		else
			return 0;//useless
		if(t==1)
		{
			sscanf(fuc1,"%d",&value);
			return 1;//constant
		}
		else
			return 2;//not constant
	}
	return 0;
}


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