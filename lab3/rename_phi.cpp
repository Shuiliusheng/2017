#include"ssa.h"
void show_rename(int f);
typedef struct{
	int st[200];
	int n_s;
}ST;

ST st_var[200];
int count[200];
Phi_B phib[100];

void st_push(int n,int v)
{
	int t=st_var[n].n_s+1;
	st_var[n].st[t]=v;
	st_var[n].n_s=t;
}

int st_pop(int n)
{
	if(st_var[n].n_s==-1)
		return -1;
	else
	{
		int t=st_var[n].n_s;
		st_var[n].n_s=t-1;
		return st_var[n].st[t];
	}
}

int st_top(int n)
{
	if(st_var[n].n_s==-1)
		return -1;
	else
	{
		int t=st_var[n].n_s;
		return st_var[n].st[t];
	}
}
int rename_flag[200];

void init_rename()
{
	for(int i=0;i<num_dvar;i++)
	{
		count[i]=0;
		st_var[i].n_s=-1;
		st_push(i,0);
	}
	for(int i=0;i<num_phi;i++)
	{
		memset(Phi[i].tag,-1,sizeof(Phi[i].tag));
	}
	memset(rename_flag,0,sizeof(rename_flag));
}

int judge_dvar(char name[])
{
	for(int i=0;i<num_dvar;i++)
		if(strcmp(dvar[i].name,name)==0)
			return i;
	return -1;
}

void change_var(char src[],char dst[])
{
	int l=strlen(src),i=0;
	for(i=0;i<l;i++)
	{
		if(src[i]=='_')
		{
			strcpy(dst,src);
			return ;
		}
		if(src[i]!='#')
			dst[i]=src[i],dst[i+1]='\0';
		else
			break;
	}
	sprintf(dst,"%s#-1",dst);
}

void replace_dvar(char src[],char dst[],int tag)
{
	if(tag==1)
		strcpy(dst,src);
	else
	{
		char temp[100];
		change_var(src,temp);
		sprintf(dst,"%c%d%s",temp[0],tag,&temp[1]);
	}
}

bool judge_def(int n,int push[])
{
	int i=0;
	char op[100],fuc1[100]="",fuc2[100]="";
	sscanf(inst[n],"%s %s %s",op,fuc1,fuc2);
	int reg=0;
	if(strcmp(op,"move")==0)
	{
		int t=judge_dvar(fuc1);
		if(t!=-1)
		{
			t=st_top(t);
			replace_dvar(fuc1,fuc1,t);
		}
		t=judge_dvar(fuc2);
		if(t!=-1)
		{
			replace_dvar(fuc2,fuc2,count[t]+1);
			count[t]=count[t]+1;
			st_push(t,count[t]);
			push[t]++;
		}
		sprintf(inst[n],"%s %s %s",op,fuc1,fuc2);
		return false;
	}
	n++;
	sscanf(inst[n],"%s %s %s",op,fuc1,fuc2);
	if(strcmp(op,"store")==0)
	{
		sscanf(inst[n-1],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(fuc2,"GP")==0&&strcmp(op,"add")==0)
		{
			int t=judge_dvar(fuc1);
			if(t!=-1)
			{
				replace_dvar(fuc1,fuc1,count[t]+1);
				count[t]=count[t]+1;
				st_push(t,count[t]);
				push[t]++;
				sprintf(inst[n-1],"%s %s %s",op,fuc1,fuc2);
			}
			return false;
		}
	}
	return true;
}

int pre_place(int f,int now,int pre)
{
	for(int i=0;i<fuction[f].block[now].num_pre;i++)
		if(pre==fuction[f].block[now].pre[i])
			return i;
	return -1;
}

void rename_phi_def(int f,int b,int push_num[])
{
	if(fuction[f].block[b].phi!=-1)
	{
	//	cout <<"block"<<b<<" :"<<endl;
		int n=fuction[f].block[b].phi;
		for(int i=0;i<Phi[n].n_p;i++)
		{
			int t=judge_dvar(Phi[n].phi[i]);
			count[t]=count[t]+1;
			st_push(t,count[t]);
			push_num[t]++;
			Phi[n].own[i]=count[t];
		//	cout <<Phi[n].phi[i][0]<<count[t]<<&Phi[n].phi[i][1]<<endl;
		}
	}
}

void rename_raw_block(int f,int b,int push_num[])
{
	for(int n=fuction[f].block[b].s;n<=fuction[f].block[b].e;n++)
	{
		if(judge_def(n,push_num))
		{
			char op[100],fuc1[100]="",fuc2[100]="";
			sscanf(inst[n],"%s %s %s",op,fuc1,fuc2);
			int t=judge_dvar(fuc1);
			if(t!=-1)
			{
				t=st_top(t);
				replace_dvar(fuc1,fuc1,t);
			}
			t=judge_dvar(fuc2);
			if(t!=-1)
			{
				t=st_top(t);
				replace_dvar(fuc2,fuc2,t);
			}
			sprintf(inst[n],"%s %s %s",op,fuc1,fuc2);
		}
	}
}

void rename_phi_param(int f,int b)
{
	for(int i=0;i<fuction[f].block[b].n_p;i++)
	{
		int now=fuction[f].block[b].point[i];
		int t=fuction[f].block[now].phi;
		if(t==-1)
			continue;
		int place=pre_place(f,now,b);
		for(int c=0;c<Phi[t].n_p;c++)
		{
			Phi[t].tag[c][place]=st_top(judge_dvar(Phi[t].phi[c]));
		}
	}
}

void rename_var(int f,int b)
{
	if(rename_flag[b]==1)
		return ;
	int push_num[200]={0};

	rename_phi_def(f,b,push_num);//phi fuction dst rename
	
	rename_raw_block(f,b,push_num);//block statement rename

	rename_phi_param(f,b);//phi fuction param rename successoin

	rename_flag[b]=1;

	for(int i=0;i<fuction[f].block[b].n_p;i++)
		rename_var(f,fuction[f].block[b].point[i]);
	

	for(int i=0;i<num_dvar;i++)
	{
		for(int j=0;j<push_num[i];j++)
			st_pop(i);
	}
}

void group_phi(int f)
{
	for(int i=0;i<fuction[f].n_b;i++)
	{
		if(fuction[f].block[i].phi==-1)
			continue;

		int t=fuction[f].block[i].phi;
		int n=fuction[f].block[i].num_pre;
		char var[100];
		for(int j=0;j<Phi[t].n_p;j++)
		{

			replace_dvar(Phi[t].phi[j],var,Phi[t].own[j]);
			sprintf(phib[t].pexpr[j].name,"%s",var);

			int num_param=0;
			for(int c=0;c<n;c++)
			{
				if(Phi[t].tag[j][c]>0)
				{
					phib[t].pexpr[j].place[num_param]=fuction[f].block[i].pre[c];
					replace_dvar(Phi[t].phi[j],var,Phi[t].tag[j][c]);
					sprintf(phib[t].pexpr[j].param[num_param++],"%s",var);
				}
			}
			phib[t].pexpr[j].n_param=num_param;
			phib[t].n_expr=Phi[t].n_p;
			phib[t].block=i;
		}
	}
}

void write_phi(char filename[])
{
	FILE *p=fopen(filename,"w");
	int n=num_inst;
	for(int f=0;f<num_fuc;f++)
	{
		for(int i=0;i<fuction[f].n_b;i++)
		{
			int t=fuction[f].block[i].phi;
			if(t!=-1)
			{
				for(int c=0;c<phib[t].n_expr;c++)
				{
					fprintf(p,"    instr %d: phi ",n++);
					for(int n=0;n<phib[t].pexpr[c].n_param;n++)
						fprintf(p,"%s ",phib[t].pexpr[c].param[n]);
					fprintf(p,"\n    instr %d: move (%d) %s\n",n,n-1,phib[t].pexpr[c].name);
					n++;
				}
			}
			for(int j=fuction[f].block[i].s;j<=fuction[f].block[i].e;j++)
				fprintf(p,"    instr %d: %s\n",j,inst[j]);
			fprintf(p,"\n");
		}
	}
	fclose(p);
}

void show_rename(int f)
{
	cout <<"fuction "<<f<<endl;
	for(int i=0;i<fuction[f].n_b;i++)
	{
		int t=fuction[f].block[i].phi;
		if(t!=-1)
		{
			for(int c=0;c<phib[t].n_expr;c++)
			{
				cout <<"phi "<<phib[t].pexpr[c].name<<" = ";
				for(int n=0;n<phib[t].pexpr[c].n_param;n++)
					cout <<phib[t].pexpr[c].param[n]<<" ";
				cout <<endl;
			}
		}
		for(int j=fuction[f].block[i].s;j<=fuction[f].block[i].e;j++)
			cout <<"inst "<<j<<" :"<<inst[j]<<endl;
		cout <<endl;
	}
}

void rename_var(int f)
{
	init_rename();
	for(int i=0;i<fuction[f].n_b;i++)
	{
		rename_var(f,i);
	}
	group_phi(f);
	//show_rename(f);
}
