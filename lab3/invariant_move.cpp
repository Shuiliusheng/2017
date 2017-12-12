#include"ssa.h"

char inst_while[10000][100];
int num_iw;


Def_expr def_expr[200];
int num_def;

char def_var[200][50];
int def_flag[200];
int num_dv;


void find_expr_start(int &start)
{
	char op[100],fuc1[100],fuc2[100]="";
	sscanf(inst_temp[start],"%s %s %s",op,fuc1,fuc2);
	int reg=0,reg1=0;
	int t=get_op(fuc1,op,reg);
	if(t==EXPR)
	{
		find_expr_start(reg);
		start=min(start,reg);
	}
	t=get_op(fuc2,op,reg1);
	if(t==EXPR)
	{
		find_expr_start(reg1);
		start=min(start,reg1);
	}
}

void copy_inst(int start,int end)
{
	num_iw=0;
	for(int i=start;i<=end;i++)
		strcpy(inst_while[num_iw++],inst_temp[i]);
}

void del_inst(int start,int end)
{
	for(int i=start;i<=end;i++)
		sprintf(inst_while[i],"nop");
}

void find_br_end(int &end)
{
	char op[100],fuc1[100],fuc2[100]="";
	sscanf(inst_temp[end],"%s %s %s",op,fuc1,fuc2);
	int dst=0;
	sscanf(fuc2,"[%d]",&dst);
	sscanf(inst_temp[dst-1],"%s %s %s",op,fuc1,fuc2);
	if(strcmp(op,"br")!=0)
		end=dst-1;
	else
	{
		int br=0;
		sscanf(fuc1,"[%d]",&br);
		if(br<dst)
			end=dst-1;
		else
			end=br-1;
	}

}

void del_useless(int start0,int end0)
{
	char op[100],fuc1[100],fuc2[100]="";

	for(int i=start0;i<=end0;i++)
	{
		int start=0,end=0;
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"br")==0)
			sprintf(inst_while[i-start0],"nop");
		if(strcmp(op,"cmpeq")!=0&&strcmp(op,"cmple")!=0&&strcmp(op,"cmplt")!=0)
			continue;
		start=i;
		find_expr_start(start);
		end=i+1;
		find_br_end(end);
		del_inst(start-start0,end-start0);
		i=end;
	}
}

void add_def_var(char name[],int flag)
{
	for(int i=0;i<num_dv;i++)
		if(strcmp(name,def_var[i])==0)
		{
			if(flag==2)
			{
				def_flag[i]=flag;
			}
			return ;
		}
	def_flag[num_dv]=flag;
	strcpy(def_var[num_dv++],name);
}

void add_def_expr(char name[],int &n,int start0)
{
	strcpy(def_expr[num_def].name,name);
	int start=n+start0;
	find_expr_start(start);
	start-=start0;
	def_expr[num_def].s=start;
	def_expr[num_def].e=n;
	num_def++;
}

void find_else_def(int start0)
{
	char op[100],fuc1[100],fuc2[100]="";
	for(int i=num_iw-1;i>=0;i--)
	{
		sscanf(inst_while[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"nop")==0)
		{
			sscanf(inst_temp[i+start0],"%s %s %s",op,fuc1,fuc2);
			if(strcmp(op,"move")==0)
			{
				int t=find_def_varname(fuc2);
				add_def_var(fuc2,2);
			}
			else if(strcmp(op,"store")==0)
			{
				sscanf(inst_temp[i-1+start0],"%s %s %s",op,fuc1,fuc2);
				int t=find_def_varname(fuc1);
				add_def_var(fuc1,2);
			}
		}
	}
}

void find_def_var(int start0)
{
	num_dv=0;
	num_def=0;
	memset(def_flag,0,sizeof(def_flag));

	char op[100],fuc1[100],fuc2[100]="";
	for(int i=num_iw-1;i>=0;i--)
	{
		sscanf(inst_while[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"nop")==0)
			continue;
		if(strcmp(op,"move")==0)
		{
			add_def_var(fuc2,1);
			add_def_expr(fuc2,i,start0);
			def_expr[num_def-1].type=1;
		}
		else if(strcmp(op,"store")==0)
		{
			sscanf(inst_while[i-1],"%s %s %s",op,fuc1,fuc2);
			add_def_var(fuc1,1);
			add_def_expr(fuc1,i,start0);
			def_expr[num_def-1].type=2;
		}
	}
	find_else_def(start0);
}

int find_def_varname(char name[])
{
	for(int i=0;i<num_dv;i++)
		if(strcmp(def_var[i],name)==0)
			return i;
	return -1;
}

void find_expr_use(int n)
{
	char op[100],fuc1[100],fuc2[100]="";
	int end=def_expr[n].e-def_expr[n].type;
	for(int i=def_expr[n].s;i<=end;i++)
	{
		sscanf(inst_while[i],"%s %s %s",op,fuc1,fuc2);
		int t1=find_def_varname(fuc1);
		if(t1!=-1)
			def_expr[n].use[t1]=1;
		int t2=find_def_varname(fuc2);
		if(t2!=-1)
			def_expr[n].use[t2]=1;
	}
	sscanf(inst_while[def_expr[n].e],"%s %s %s",op,fuc1,fuc2);
	int t1=find_def_varname(fuc1);
	if(t1!=-1)
	{
		def_expr[n].use[t1]=1;
	}
	t1=find_def_varname(def_expr[n].name);
	if(def_flag[t1]==2)//自己本身在循环中的其它语句块中也被赋值了
		def_expr[n].flag=1;
}

bool judge_invariant_expr2()
{
	bool flag=false;
	for(int i=0;i<num_def;i++)
	{
		if(def_expr[i].flag!=0)
			continue;
		for(int c=0;c<num_dv;c++)
		{
			if(def_expr[i].use[c]==0)
				continue;

			if(def_expr[c].flag==1)
			{
				def_expr[i].flag=1;
				flag=true;
				break;
			}
			else if(def_flag[c]==2)//在循环中的其它基本块中使用定值了该语句使用的变量
			{
				def_expr[i].flag=1;
				flag=true;
				break;
			}
		}
	}
	return flag;
}

bool judge_nested_use(char varname[],int n)
{
	bool flag=false;
	for(int i=0;i<num_def;i++)
	{
		if(strcmp(varname,def_expr[i].name)==0)
		{
			for(int c=0;c<num_def;c++)
			{
				if(def_expr[i].use[c]==1)
				{
					if(def_expr[n].use[c]!=1)
					{
						def_expr[n].use[c]=1;
						flag=true;
					}
				}
			}
		}
	}
	return flag;
}

bool judge_nested_use(int n)//find second using var
{
	bool flag=false;
	for(int i=0;i<num_dv;i++)
	{
		if(def_expr[n].use[i]==1)
		{
			if(judge_nested_use(def_var[i],n))
				flag=true;
		}
	}
	return flag;
}

bool judge_invariant_expr1()
{
	bool flag=false;
	for(int i=0;i<num_def;i++)
	{
		int t1=find_def_varname(def_expr[i].name);
		if(def_expr[i].use[t1]==1)// self use by itself
			def_expr[i].flag=1;
	
		if(def_expr[i].flag!=0)
			continue;
		if(judge_nested_use(i))//循环引用
			flag=true;
	}
	return flag;
}

void show_invariant()
{
	for(int i=0;i<num_def;i++)
	{
		if(def_expr[i].flag==1)
			continue;
		for(int c=def_expr[i].s;c<=def_expr[i].e;c++)
			cout <<c<<" :"<<inst_while[c]<<endl;
		cout <<"name: "<<def_expr[i].name<<endl;
		cout <<"flag: "<<def_expr[i].flag<<endl;
		cout <<"use var: ";
		for(int c=0;c<num_dv;c++)
			if(def_expr[i].use[c]==1)
				cout <<def_var[c]<<" ";
		cout <<endl;
		cout <<endl;
	}
}

void judge_invariant(int f)//寻找循环不变量
{
	for(int i=0;i<num_def;i++)
	{
		def_expr[i].flag=0;
		memset(def_expr[i].use,0,sizeof(def_expr[i].use));
		find_expr_use(i);//正常的表达式的变量使用
	}

	int n=1;
	while(judge_invariant_expr1());//表达式的循环引用变量检测
		//cout <<"var nest check "<<n++<<endl;
	n=1;
	while(judge_invariant_expr2());//判断是否为循环不变量
		//cout <<"invariant check "<<n++<<endl;

	//show_invariant();

	for(int i=0;i<num_def;i++)
	{
		if(def_expr[i].flag==1)
			continue;;
		for(int c=def_expr[i].s;c<=def_expr[i].e;c++)
			sprintf(inst_while[c],"invariant");
		fuction[f].ntemp++;
	}

}

void rewrite_regnum(int start,int end)
{
	for(int i=start;i<=end;i++)
	{
		char op[100]="",fuc1[100]="",fuc2[100]="";
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(fuc1[0]=='(')
		{
			int t=0;
			sscanf(fuc1,"(%d)",&t);
			sprintf(fuc1,"(%d)",new_place[t]);
		}
		else if(fuc1[0]=='[')
		{
			int t=0;
			sscanf(fuc1,"[%d]",&t);
			sprintf(fuc1,"[%d]",new_place[t]);
		}

		if(fuc2[0]=='(')
		{
			int t=0;
			sscanf(fuc2,"(%d)",&t);
			sprintf(fuc2,"(%d)",new_place[t]);
		}
		else if(fuc2[0]=='[')
		{
			int t=0;
			sscanf(fuc2,"[%d]",&t);
			sprintf(fuc2,"[%d]",new_place[t]);
		}
		sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
	}
}

void rewrite_while(int start,int end)
{
	int n=end;
	char str[1000][100];
	int num=0;
	for(int i=start;i<=end;i++)
	{
		if(strcmp(inst_temp[i],"invariant")==0)
		{
			new_place[i]=start+num;
			strcpy(str[num++],inst[i]);
		}
	}

	for(int i=end;i>=start;i--)
	{
		if(strcmp(inst_temp[i],"invariant")!=0)
		{
			new_place[i]=n;
			strcpy(inst_temp[n],inst[i]);
			n--;
		}
	}
	for(int i=start;i<start+num;i++)
		strcpy(inst_temp[i],str[i-start]);
	rewrite_regnum(start,end);
}


void change_while(int &start,int &end,int f)
{
	copy_inst(start+2,end);//提取循环中的指令
	del_useless(start+2,end);//删除非基本表达式的所有语句
	find_def_var(start+2);//寻找所有的基本赋值表达式和 赋值变量
	judge_invariant(f);//寻找循环不变量

	int start1=start;
	find_expr_start(start);//确定循环的最开始位置

	for(int i=start;i<=end;i++)
		strcpy(inst[i],inst_temp[i]);
	for(int i=1;i<num_temp;i++)
		new_place[i]=i;

	for(int i=0;i<num_iw;i++)
		strcpy(inst_temp[i+start1+2],inst_while[i]);

	rewrite_while(start,end);
}

bool find_while(int &start,int &end)
{
	int i=end;
	char op[100],fuc1[100],fuc2[100]="";
	for(i=end;i>=start;i--)
	{
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"blbc")!=0&&strcmp(op,"blbs")!=0)
			continue;
		int dst=0;
		sscanf(fuc2,"[%d]",&dst);
		sscanf(inst_temp[dst-1],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"br")!=0)
			continue;
		int br=0;
		sscanf(fuc1,"[%d]",&br);
		if(br<dst)
		{
			start=i-1;
			end=dst-1;
			return true;
		}
	}
	return false;
}

void invariant_move_fuc(int f)
{
	for(int i=fuction[f].end;i>=fuction[f].start;i--)
	{
		int start=fuction[f].start;
		int end=i;
		if(find_while(start,end))//找到循环的范围
		{
			change_while(start,end,f);//更改循环内容
			i=start-1;
		}
	}
}

void pre_process()
{
	char op[100],fuc1[100],fuc2[100]="";
	char op1[100],fuc3[100],fuc4[100]="";
	for(int i=1;i<num_temp;i++)
	{
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"move")==0&&strcmp(fuc1,fuc2)==0)
			sprintf(inst_temp[i],"nop");
		else if(strcmp(op,"load")==0)
		{
			sscanf(inst_temp[i+2],"%s %s %s",op1,fuc3,fuc4);
			if(strcmp(op1,"store")!=0)
				continue;

			sscanf(inst_temp[i-1],"%s %s %s",op,fuc1,fuc2);
			sscanf(inst_temp[i+1],"%s %s %s",op1,fuc3,fuc4);
			if(strcmp(fuc1,fuc3)==0)
			{
				for(int c=i-1;c<=i+2;c++)
					sprintf(inst_temp[c],"nop");
			}
		}
	}
}

void write_invariant(char filename[])
{
	FILE *p=fopen(filename,"w");
	for(int i=1;i<num_temp;i++)
	{
		fprintf(p,"    instr %d: %s\n",i,inst_temp[i]);
	}
	fclose(p);
}

void invariant_report_write(char filename[])
{
	FILE *p=fopen(filename,"w");
	for(int i=0;i<num_fuc;i++)
	{
		fprintf(p,"Fuction: %d\n",fuction[i].start);
		fprintf(p,"Number of statement hoisted: %d\n",fuction[i].ntemp);
	}
	fclose(p);
}

void invariant_move()
{
	pre_process();
	for(int i=0;i<num_fuc;i++)
	{
		fuction[i].ntemp=0;
		invariant_move_fuc(i);
	}	
}