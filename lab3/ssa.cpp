#include"ssa.h"

PHI Phi[200];
int num_phi=0;

Dvar dvar[100];
int num_dvar=0;

int idom[200];
char df[500][200];
char flag_df[500];
char DF[200];

char (*Aorig)[100]=kill;
char (*Aphi)[100]=gen;

void ssa_cal()
{
	int i=0;
	for(i=0;i<num_fuc;i++)
	{
		for(int j=0;j<fuction[i].n_b;j++)
			fuction[i].block[j].phi=-1;
		dom_cal_fuc(i);
		find_df(i);
		find_phi(i);
		rename_var(i);
	}

}


int judge_expr1(int n,char name[],int &value)
{
	int i=0;
	char op[100],fuc1[100],fuc2[100];
	sscanf(inst_temp[n],"%s %s %s",op,fuc1,fuc2);
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
		sscanf(inst_temp[n-1],"%s %s %s",op,temp,fuc2);
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




