#include"lab2.h"


int get_op(char src[],char dst[],int &reg)//获取操作数的类型，表达式，常数，变量（局部/全局）
{
	if(src[0]<='9'&&src[0]>='0')//常数
	{
		strcpy(dst,src);
		return 1;
	}
	else if(src[0]=='(')//表达式
	{
		sscanf(src,"(%d)",&reg);
		return 2;
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
		return 3;
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

void pre_kill()
{
	int i=0;
	for(i=0;i<num_expr;i++)
	{
		for(int j=0;j<num_expr;j++)
		{
			if(j!=i&&strcmp(expr[i].name,expr[j].name)==0)
			{
				expr[i].kill[j]=1;
			}
		}
	}
}

void gen_kill_block(int f,int b)
{
	int i=0;
	int start=fuction[f].block[b].s;
	int end=fuction[f].block[b].e;
	for(i=0;i<num_expr;i++)
		kill[b][i]=0;
	for(i=0;i<num_expr;i++)
	{
		if(expr[i].place<=end&&expr[i].place>=start)
		{
			gen[b][i]=1;
			for(int j=0;j<num_expr;j++)
			{
				if(expr[i].kill[j]==1)
					kill[b][j]=1;
				if(expr[i].kill[j]==1&&gen[b][j]==1)/////////////
					gen[b][j]=0;
			}
		}
		else
			gen[b][i]=0;
	}
}

void show_out(char out[][100],int b,char str[])
{
	cout <<str<<" "<<b<<" :";
	for(int i=0;i<num_expr;i++)
	{
		cout <<(int)out[b][i]<<" ";
	}
	cout <<endl;
}

void gen_kill_fuc(int n)
{
	for(int i=0;i<fuction[n].n_b;i++)
	{
		gen_kill_block(n,i);
	}
}

void get_in(int f,int b)
{
	if(b==0)
		return ;
	for(int i=0;i<fuction[f].block[b].num_pre;i++)
	{
		int t=fuction[f].block[b].pre[i];
		for(int j=0;j<num_expr;j++)
		{
			if(out[t][j]==1)
				in[b][j]=1;
		}
	}
}

int cal_out(int f,int b)
{
	int i=0;
	int flag=0;
	for(i=0;i<num_expr;i++)
	{
		if((in[b][i]==1&&kill[b][i]!=1)||gen[b][i]==1)
		{
			if(out[b][i]!=1)
				flag=1;
			out[b][i]=1;
		}
		else
		{
			if(out[b][i]!=0)
				flag=1;
			out[b][i]=0;
		}
	}
	return flag;
}


void reaching_define(int f)
{
	int i=0,j=0;
	for(j=0;j<fuction[f].n_b;j++)
		for(i=0;i<num_expr;i++)
		{
			out[j][i]=0;
			in[j][i]=0;
		}

	int change=1;
	while(change)
	{
		change=0;
		for(i=0;i<fuction[f].n_b;i++)
		{
			get_in(f,i);
			int t=cal_out(f,i);
			if(t==1)
				change=1;
		}
	}
}

bool replace_constant(int e,int f,int b)
{
	int i=0;
	bool flag=false;
	char op[100],fuc1[100],fuc2[100];
	for(i=fuction[f].block[b].s;i<=fuction[f].block[b].e;i++)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(fuc1,expr[e].name)==0&&strcmp(fuc2,"GP")==0)
		{
			char temp[100];
			sprintf(temp,"(%d)",i+1);
			for(int j=i+2;j<=fuction[f].block[b].e;j++)
			{
				sscanf(inst[j],"%s %s %s",op,fuc1,fuc2);
				if(strcmp(fuc1,temp)==0)
					sprintf(fuc1,"%d",expr[e].value);
				if(strcmp(fuc2,temp)==0)
					sprintf(fuc2,"%d",expr[e].value);
				sprintf(inst[j],"%s %s %s",op,fuc1,fuc2);
			}
			sprintf(inst[i],"nop");
			sprintf(inst[i+1],"nop");
			flag=true;
			continue;
		}
		if(strcmp(fuc1,expr[e].name)==0)
		{
			sprintf(inst[i],"%s %d %s",op,expr[e].value,fuc2);
			flag=true;
		}
		if(strcmp(fuc2,expr[e].name)==0)
		{
			sprintf(inst[i],"%s %s %d",op,fuc1,expr[e].value);
			flag=true;
		}
	}
	return flag;
}

void judge_constant(int f,int b)
{
	int i=0,j=0;
	char temp[100];
	for(i=0;i<num_expr;i++)
	{
		temp[i]=0;
		if(in[b][i]==out[b][i]&&in[b][i]==1)
			temp[i]=1;
	}
	for(i=0;i<num_expr;i++)
	{
		if(temp[i]==1)
		{
			for(j=0;j<num_expr;j++)
			{
				if(j!=i&&temp[j]==1&&strcmp(expr[i].name,expr[j].name)==0)
				{
					temp[j]=0;
					break;
				}
			}
			if(j>=num_expr)
				temp[i]=2;
		}
	}
	for(i=0;i<num_expr;i++)
	{
		if(temp[i]==2)
		{
			if(expr[i].place<=fuction[f].block[b].e&&expr[i].place>=fuction[f].block[b].s||expr[i].type==2)
				temp[i]=0;
		}
	}

	for(i=0;i<num_expr;i++)
	{
		if(temp[i]==2&&replace_constant(i,f,b))
		{
			constant_num[i]=1;
			//cout <<"replace inst"<<i<<":"<<expr[i].name<<" to "<<expr[i].value<<endl;
		}
	}
}

void constant_spread_fuc(int n)
{
	int i=0;
	char name[100];
	int value=0;
	for(i=fuction[n].start;i<=fuction[n].end;i++)
	{
		int t=judge_expr(i,name,value);
		if(t!=0)
		{
			expr[num_expr].place=i;
			expr[num_expr].type=t;
			expr[num_expr].value=value;
			strcpy(expr[num_expr].name,name);
			num_expr++;
		}
	}
	pre_kill();
	gen_kill_fuc(n);
	reaching_define(n);
	for(int j=0;j<fuction[n].n_b;j++)
	{
		judge_constant(n,j);
	}
	int num=0;
	for(i=0;i<num_expr;i++)
		if(constant_num[i]==1)
			num++;
	fprintf(const_rep,"Fuction:%d\n",fuction[n].start);
	fprintf(const_rep,"Number of constants propagated:%d\n",num);
}

void constant_spread()
{
	int i=0;
	for(i=0;i<num_fuc;i++)
	{
		num_expr=0;
		constant_spread_fuc(i);
	}
	fclose(const_rep);
	cout <<"constant spread accomplish!"<<endl;
}