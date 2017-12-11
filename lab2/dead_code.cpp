#include"lab2.h"

char (*use)[100]=gen;
char (*def)[100]=kill;

int cmp_var(char name[])
{
	for(int i=0;i<num_expr;i++)
		if(strcmp(name,expr[i].name)==0)
			return i;
	return -1;
}

void get_use(char str[],int b)
{
	int i=0;
	for(i=0;i<num_expr;i++)
	{
		if(strcmp(str,expr[i].name)==0)
		{
			use[b][i]=1;
			def[b][i]=0;
		}
	}
}

void get_def(char str[],int b)
{
	int i=0;
	for(i=0;i<num_expr;i++)
	{
		if(strcmp(str,expr[i].name)==0)
		{
			def[b][i]=1;
			use[b][i]=0;
		}
	}
}

void def_use(int f,int b)
{
	int i=0;
	int start=fuction[f].block[b].s;
	int end=fuction[f].block[b].e;
	char op[100],fuc1[100],fuc2[100];
	for(i=0;i<num_expr;i++)
	{
		use[b][i]=0;
		def[b][i]=0;
	}
	for(i=end;i>=start;i--)
	{
		fuc1[0]='\0';
		fuc2[0]='\0';
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"move")==0)
		{
			get_def(fuc2,b);
			get_use(fuc1,b);
		}
		else if(strcmp(op,"store")==0)
		{
			char temp[100];
			sscanf(inst[i-1],"%s %s %s",op,temp,fuc2);
			if(strcmp(op,"add")==0&&strcmp(fuc2,"GP")==0)
			{
				get_def(temp,b);
				i--;
			}
			get_use(fuc1,b);
		}
		else
		{
			get_use(fuc1,b);
			get_use(fuc2,b);
		}
	}
}

void def_use_fuc(int f)
{
	for(int i=0;i<fuction[f].n_b;i++)
	{
		def_use(f,i);
	}
}

void add_single_var(char name[],int place)
{
	int i=0;
	for(i=0;i<num_expr;i++)
	{
		if(strcmp(name,expr[i].name)==0)
			break;
	}
	if(i==num_expr)
	{
		strcpy(expr[num_expr].name,name);
		expr[num_expr].place=place;
	//	cout <<num_expr<<" "<<name<<" "<<place<<endl;
		num_expr++;
	}
}

void get_out(int f,int b)
{
	if(b==fuction[f].n_b-1)
		return ;
	for(int i=0;i<fuction[f].block[b].n_p;i++)
	{
		int t=fuction[f].block[b].point[i];
		for(int j=0;j<num_expr;j++)
		{
			if(in[t][j]==1)
				out[b][j]=1;
		}
	}
}

int cal_in(int f,int b)
{
	int i=0;
	int flag=0;
	for(i=0;i<num_expr;i++)
	{
		if((out[b][i]==1&&def[b][i]!=1)||use[b][i]==1)
		{
			if(in[b][i]!=1)
				flag=1;
			in[b][i]=1;
		}
		else
		{
			if(in[b][i]!=0)
				flag=1;
			in[b][i]=0;
		}
	}
	return flag;
}

void live_var(int f)
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
		for(i=fuction[f].n_b-1;i>=0;i--)
		{
			get_out(f,i);
			int t=cal_in(f,i);
			if(t==1)
				change=1;
		}
	}
}

int elim[500];
int num_elim=0;

void delete_inst(int n)
{
//	cout <<"delete "<<n<<" "<<inst[n]<<" "<<"nop"<<endl;
	char op[100],fuc1[100],fuc2[100]="";
	sscanf(inst[n],"%s %s %s",op,fuc1,fuc2);
	int reg=0;
	char temp[100];
	int t=get_op(fuc1,temp,reg);
	if(t==2)
		delete_inst(reg);
	t=get_op(fuc2,temp,reg);
	if(t==2)
		delete_inst(reg);
	sprintf(inst[n],"nop");
}

void delete_inst(int n,char str[])
{
	char temp[100];
	int reg=0;
	int t=get_op(str,temp,reg);
	if(t==2)
		delete_inst(reg);
//	cout <<"delete "<<n<<" "<<inst[n]<<" "<<"nop"<<endl;
	sprintf(inst[n],"nop");
}
void delete_deadcode(int start,int end,char name[])
{
	char op[100],fuc1[100],fuc2[100];
	for(int i=end;i>=start;i--)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"move")==0&&strcmp(fuc2,name)==0)
		{
			delete_inst(i,fuc1);
			elim[num_elim++]=i;
		}
		else if(strcmp(op,"store")==0)
		{
			char temp[100];
			sscanf(inst[i-1],"%s %s %s",op,temp,fuc2);
			if(strcmp(op,"add")==0&&strcmp(temp,name)==0)
			{
			//	cout <<"delete "<<i-1<<" "<<inst[i-1]<<" "<<"nop"<<endl;
				sprintf(inst[i-1],"nop");
				delete_inst(i,fuc1);
				elim[num_elim++]=i;
			}
		}
	}
}

void elimination_block(int f,int b)
{
	int start=fuction[f].block[b].s;
	int end=fuction[f].block[b].e;
	int i=0;
	for(i=0;i<num_expr;i++)
	{
		if(out[b][i]==0)
		{
			if(expr[i].place<=end&&expr[i].place>=start)
				start=expr[i].place+1;
			delete_deadcode(start,end,expr[i].name);
		}
	}
}

void elimination_fuc(int f)
{
	for(int i=0;i<fuction[f].n_b;i++)
	{
		elimination_block(f,i);
	}
}

void show_info(int f)
{
	int i=0;
	for(i=0;i<fuction[f].n_b;i++)
	{
		cout <<endl;
		for(int j=fuction[f].block[i].s;j<=fuction[f].block[i].e;j++)
			cout <<j<<" :"<<inst[j]<<endl;
		cout <<"in:";
		for(int j=0;j<num_expr;j++)
			cout <<(int)in[i][j]<<" ";
		cout <<endl<<"out:";
		for(int j=0;j<num_expr;j++)
			cout <<(int)out[i][j]<<" ";
		cout <<endl;
	}
}
void dead_code_fuc(int f)
{
	int i=0;
	char name[100];
	int value=0;
	for(i=fuction[f].start;i<=fuction[f].end;i++)
	{
		int t=judge_expr(i,name,value);
		if(t!=0)
		{
			add_single_var(name,i);
		}
	}
	def_use_fuc(f);
	live_var(f);
	elimination_fuc(f);
	//show_info(f);
}

int src=0;
int usrc=0;

void add_src(int s,int e)
{
	for(int i=0;i<num_elim;i++)
	{
		if(elim[i]<=e&&elim[i]>=s)
			src++;
	}
}

void statistic(int f)
{
	int start=fuction[f].start;
	int end=fuction[f].end;
	char op[100],fuc1[100],fuc2[100];
	for(int i=end;i>=start;i--)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"br")==0)
		{
			int reg=0;
			sscanf(fuc1,"[%d]",&reg);
			if(reg<=i)
			{
				add_src(reg,i);
				i=reg;
			}
		}
	}
}

void elimination_deadcode()
{
	int i=0;
	for(i=0;i<num_fuc;i++)
	{
		num_expr=0;
		dead_code_fuc(i);
		src=0;
		usrc=0;
		statistic(i);
		usrc=num_elim-src;
		fprintf(dead_rep,"Function: %d\n",fuction[i].start);
		fprintf(dead_rep,"Number of statements eliminated in SCR: %d\n",src);
		fprintf(dead_rep,"Number of statements eliminated not in SCR: %d\n",usrc);
	}
	fclose(dead_rep);
	cout <<"elimination deadcode accomplish!"<<endl;
}