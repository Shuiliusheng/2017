#include"ssa.h"

typedef struct{
	char inst[100][200];
	int num;
}Block_inst;

Block_inst block_inst[200];
int num_ib=0;

char inst_temp[MAX][200];
int num_temp;
int new_place[MAX];
int block_flag[MAX];


bool judge_global(char name[])
{
	int l=strlen(name);
	for(int i=0;i<l-5;i++)
		if(name[i]=='_'&&name[i+1]=='b'&&name[i+2]=='a')
			return true;
	return false;
}

void add_inst(int f,int b,char name[],char param[])
{
	int block=fuction[f].block[b].e;
	int binst=block_flag[block];
	if(binst==-1)
	{
		block_inst[num_ib].num=0;
		binst=num_ib;
		block_flag[block]=binst;
		num_ib++;
	}
	int num=block_inst[binst].num;
	bool fname=judge_global(name);
	bool fparam=judge_global(param);
	if(fname&&fparam)
	{
		sprintf(block_inst[binst].inst[num++],"add %s GP ",param);
		sprintf(block_inst[binst].inst[num++],"loadfront");
		sprintf(block_inst[binst].inst[num++],"add %s GP ",name);
		sprintf(block_inst[binst].inst[num++],"storefront");
		block_inst[binst].num=num;
	}
	else
	{
		sprintf(block_inst[binst].inst[num],"move %s %s",param,name);
	//	cout <<fuction[f].block[b].s<<" : "<<block_inst[binst].inst[num]<<endl;
		block_inst[binst].num++;
	}
}

void delete_phi(int f,int b)
{
	if(fuction[f].block[b].phi==-1)
		return ;
	int phi_place=fuction[f].block[b].phi;
//	cout <<"delete phi:"<<fuction[f].block[b].s<<endl;
	for(int expr=0;expr<phib[phi_place].n_expr;expr++)//要插入的表达式
	{
	//	cout <<phib[phi_place].pexpr[expr].name<<" ";
		for(int i=0;i<phib[phi_place].pexpr[expr].n_param;i++)//应该在哪个前驱中插入哪个参数
		{
			int block=phib[phi_place].pexpr[expr].place[i];
		//	cout <<phib[phi_place].pexpr[expr].param[i]<<" ";
			add_inst(f,block,phib[phi_place].pexpr[expr].name,phib[phi_place].pexpr[expr].param[i]);
		}
		//cout <<endl;
	}
}

void add_phi_inst(int &num)
{
	for(int i=1;i<num_inst;i++)
	{
		strcpy(inst_temp[num],inst[i]);
		new_place[i]=num;
		num++;
		if(block_flag[i]!=-1)
		{
			char op[100]="",fuc1[100]="",fuc2[100]="";
			sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
			int flag=0;
			if(strcmp(op,"br")==0||strcmp(op,"call")==0)//之前调到这个地方的指令都需要改变
			{
				num--;
				new_place[i]=num;
		//		cout <<"------br :"<<i<<" "<<num<<endl;
				flag=1;
			}
			else if(strcmp(op,"blbs")==0||strcmp(op,"blbc")==0)
			{
				num=num-2;
				flag=2;
			}
			int block=block_flag[i];
			for(int c=0;c<block_inst[block].num;c++)
			{
				if(strcmp(block_inst[block].inst[c],"loadfront")==0)
				{
					sprintf(inst_temp[num],"load !(%d) ",num-1);
			//		cout <<num<<" : "<<inst_temp[num]<<endl;
					num++;
				}
				else if(strcmp(block_inst[block].inst[c],"storefront")==0)
				{
					sprintf(inst_temp[num],"store !(%d) !(%d) ",num-2,num-1);
				//	cout <<num<<" : "<<inst_temp[num]<<endl;
					num++;
				}
				else
				{
					strcpy(inst_temp[num++],block_inst[block].inst[c]);
			//		cout <<num-1<<" : "<<inst_temp[num-1]<<endl;
				}
			}
		//	cout <<endl;
			if(flag==1)
			{
				strcpy(inst_temp[num],inst[i]);
				num++;
			}
			else if(flag==2)
			{
				strcpy(inst_temp[num],inst[i-1]);
				new_place[i]=num;
				num++;
				strcpy(inst_temp[num],inst[i]);
				new_place[i]=num;
				num++;
			}
		}
	}
}

int use_def_var[1000];

int find_init_var(int start,int end)//寻找函数中未被定义即使用的变量
{
	memset(use_def_var,-1,sizeof(use_def_var));
	for(int i=1;i<end;i++)
	{
		char op[100],fuc1[100]="",fuc2[100]="";
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"ret")==0)
		{
			return i;
		}
		int t1=judge_dvar(fuc1);
		if(t1!=-1)
		{
			char op1[100],fuc3[100]="",fuc4[100]="";
			sscanf(inst_temp[i+1],"%s %s %s",op1,fuc3,fuc4);
			if(strcmp(op1,"store")==0)
			{
				if(use_def_var[t1]==-1)
					use_def_var[t1]=1;

			}
			else
			{
				if(use_def_var[t1]==-1)
				{
					use_def_var[t1]=2;
				}
			}
		}
		int t2=judge_dvar(fuc2);
		if(t2!=-1)
		{
			if(strcmp(op,"move")==0)
			{
				if(use_def_var[t2]==-1)
				{
					use_def_var[t2]=1;
				}
			}
			else if(use_def_var[t2]==-1)
			{
				use_def_var[t2]=2;
			}
			
		}
	}

}

bool find_base(char name[])
{
	int l=strlen(name);
	for(int i=0;i<l-5;i++)
		if(name[i]=='_'&&name[i+1]=='b'&&name[i+2]=='a')
			return true;
	return false;
}

void add_init_var(int &n)
{
	for(int c=0;c<num_dvar;c++)
		if(use_def_var[c]==2)
		{
			if(find_base(dvar[c].name))
			{
				sprintf(inst[n++],"add %s GP",dvar[c].name);
			//	cout <<n-1<<" "<<inst[n-1]<<endl;
				sprintf(inst[n],"store 0 (%d)",n-1);
				n++;
			//	cout <<n-1<<" "<<inst[n-1]<<endl;
			}
			else
			{
				sprintf(inst[n++],"move 0 %s",dvar[c].name);
			//	cout <<n-1<<" "<<inst[n-1]<<endl;
			}
		}
}

void init_var(int &num)
{
	char op[100],fuc1[100],fuc2[100]="";
	int n=1;
	for(int i=1;i<num;i++)
	{
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		new_place[i]=n;
		strcpy(inst[n++],inst_temp[i]);
		if(strcmp(op,"enter")==0)
		{
			find_init_var(i,num);
			add_init_var(n);
		}
	}
	num=n;
	for(int i=1;i<n;i++)
		strcpy(inst_temp[i],inst[i]);
}

void update_tag(int num)
{
	for(int i=1;i<num;i++)
	{
		char op[100]="",fuc1[100]="",fuc2[100]="";
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(fuc1[0]=='(')
		{
			int dst=0;
			sscanf(fuc1,"(%d)",&dst);
			sprintf(fuc1,"(%d)",new_place[dst]);
		}
		else if(fuc1[0]=='!')
		{
			sprintf(fuc1,"%s",&fuc1[1]);
		}
		if(fuc2[0]=='(')
		{
			int dst=0;
			sscanf(fuc2,"(%d)",&dst);
			sprintf(fuc2,"(%d)",new_place[dst]);
		}
		else if(fuc2[0]=='!')
		{
			sprintf(fuc2,"%s",&fuc2[1]);
		}
		sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);

		if(strcmp(op,"blbs")==0||strcmp(op,"blbc")==0)
		{
			int dst=0;
			sscanf(fuc2,"[%d]",&dst);
			sprintf(inst_temp[i],"%s %s [%d] ",op,fuc1,new_place[dst]);
		}
		else if(strcmp(op,"br")==0||strcmp(op,"call")==0)
		{
			int dst=0;
			sscanf(fuc1,"[%d]",&dst);
			sprintf(inst_temp[i],"%s [%d] ",op,new_place[dst]);
		}
	}
	num_temp=num;

	for(int i=0;i<num_fuc;i++)
	{
		fuction[i].start=new_place[fuction[i].start];
		fuction[i].end=new_place[fuction[i].end];
		for(int j=0;j<fuction[i].n_b;j++)
		{
			fuction[i].block[j].e=new_place[fuction[i].block[j].e];
			fuction[i].block[j].s=new_place[fuction[i].block[j].s];
		}
	}
}

void recontruct_inst()
{
	int num=1;
	add_phi_inst(num);
	update_tag(num);
	init_var(num);
	update_tag(num);
}

void write_new_phi(char filename[])
{
	FILE *p=fopen(filename,"w");
	for(int i=1;i<num_temp;i++)
	{
		fprintf(p,"    instr %d: %s\n",i,inst_temp[i]);
	}
	fclose(p);
}


void recovery_phi()
{
	memset(block_flag,-1,sizeof(block_flag));
	num_ib=0;
	for(int i=0;i<num_fuc;i++)
	{
		for(int j=0;j<fuction[i].n_b;j++)
			delete_phi(i,j);
	}
	recontruct_inst();
}