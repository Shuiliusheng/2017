#include"lab1.h"


int main(int argc,char **argv)
{
	char filename[300]="";
	if(argc>1)
	{
		if(strcmp(argv[1],"-h")==0)
		{
			printf("recompiler src dst\n");
		}
		else
		{
			strcpy(filename,argv[1]);
			if(load_inst(filename))
			{
				if(argc>2)
					strcpy(filename,argv[2]);
				else
					strcpy(filename,"");
				write_file(filename);
			}
		}
	}
	else
	{
		if(load_inst(filename))
		{
			strcpy(filename,"");
			write_file(filename);
		}
	}
	pause();
	return 0;
}


//反编译
void recompiler()
{
	int i=0;
	char op[expr_size],fuc1[expr_size],fuc2[expr_size];
	for(i=1;i<num_inst;i++)
	{
		extra_judge(i);//额外的判断，目前为增加  }
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"nop")==0)//空指令
			continue;
		else if(strcmp(op,"entrypc")==0)//指示main函数
		{
			fprintf(fw,"void main(");
			local_define(i+1);//局部变量检测与定义
			i++;
		}
		else if(strcmp(op,"enter")==0)//函数的定义
		{
			fprintf(fw,"void fuction_%d(",i);
			local_define(i+1);//函数的局部变量检测
			continue;
		}
		else if(strcmp(op,"cmpeq")==0)//比较相等
		{
			cmp_inst(op,fuc1,fuc2,"==","!=",i);//判断语句的识别，包括了while
		}
		else if(strcmp(op,"cmple")==0)//比较小于等于
		{
			cmp_inst(op,fuc1,fuc2,"<=",">",i);
		}
		else if(strcmp(op,"cmplt")==0)//比较小于
		{
			cmp_inst(op,fuc1,fuc2,"<",">=",i);
		}
		else if(strcmp(op,"load")==0)//加载内存数据，全局变量，结构体，数组
		{
			int reg=0;
			sscanf(fuc1,"(%d)",&reg);
			transform_load(reg,i);//load指令的转化，转化为一个简单的表达式，用于之后的使用
		}
		else if(strcmp(op,"store")==0)//存储内存数据，全局变量，结构体，数组
		{
			char v1[expr_size],v2[expr_size];
			int reg1,reg2=0;
			int t1=get_op(fuc1,v1,reg1);//获取当前参数是 常数，变量（全局/局部），表达式
			int t2=get_op(fuc2,v2,reg2);

			if(t1==2)
				get_expr(reg1,v1);//获取表达式
			if(t2==2)
				get_expr(reg2,v2);

			sprintf(inst[i]," %s=%s",v2,v1);
			add_tab();//添加tab
			fprintf(fw,"%s=%s;\n",v2,v1);
		}
		else if(strcmp(op,"param")==0)//参数，用于函数调用
		{
			translate_fuc(i);//函数调用转化，带参数，返回时为call指令
		}
		else if(strcmp(op,"call")==0)//无参数的函数调用
		{
			int reg=0;
			sscanf(fuc1,"[%d]",&reg);
			add_tab();
			fprintf(fw,"fuction_%d();\n");
		}
		else if(strcmp(op,"else")==0)//辅助指令，用于else
		{
			add_tab();
			fprintf(fw,"else\n");
			add_tab();
			fprintf(fw,"{\n");
			num_big++;
			int t=0;
			sscanf(fuc1,"%d",&t);
			extra[t]++;
		}
		else if(strcmp(op,"write")==0)//输出
		{
			char v1[expr_size];
			int reg=0;
			int t=get_op(fuc1,v1,reg);
			if(t==2)
				get_expr(reg,v1);
			add_tab();
			fprintf(fw,"WriteLong(%s);\n",v1);
		}
		else if(strcmp(op,"wrl")==0)//换行
		{
			add_tab();
			fprintf(fw,"WriteLine();\n");
		}
		else if(simple_expr(op))//简单的表达式，add,sub,mul,div,mod,neg,move
		{
			char dst[expr_size];
			get_expr(i,dst);
			if(strcmp(op,"move")==0)
			{
				add_tab();
				fprintf(fw,"%s;\n",dst);
			}
		}
		else if(strcmp(op,"ret")==0)//函数返回指令
		{
			num_big--;
			add_tab();
			fprintf(fw,"}\n");
		}
	}
}



/*********************************全局变量的定义************************************/

void global_def()//全局变量的定义，包括数组，结构体，变量
{
	detect_array();//检测数组，并定义全局数组

	global_struct();//检测结构体体，并定义

	int list[1000],num=0;
	define_var(var_name,num_v,list,num);//全局变量定义
	for(int i=0;i<num;i++)
		fprintf(fw,"long %s; \n",var_name[list[i]]);
}


/*********************************数组的定义和转换************************************/

void transform_array(int n)//数组转换
{
	char op[expr_size],fuc1[expr_size],type[expr_size],op1[expr_size],expr[expr_size];
	sscanf(inst[n],"%s %s %s",op,expr,type);
	sscanf(inst[n+1],"%s %s %s",op1,fuc1,type);
	
	char name[expr_size];
	if(find_base(fuc1,name))//有数组的标志，带有base
	{
		if(strcmp(type,"GP")==0)
		{
			strcpy(var_name[num_v++],name);
		}

		sprintf(inst[n]," nop ");
		sprintf(inst[n+1]," nop ");

		int reg=0;
		int t=get_op(expr,fuc1,reg);
		
		if(t==1)
			sprintf(inst[n+2]," array %s[%s] %s %s ",name,fuc1,name,type);
		else if(t==3)
		{
			sprintf(inst[n+1]," var %s ",expr);
			sprintf(inst[n+2]," array %s[%s] %s %s ",name,fuc1,name,type);
		}
		else if(t==2)
		{
			sprintf(inst[n+2]," array %s[(%s)] %s %s %s %d",name,expr,name,type,"reg",reg);
		}
	}

}

void detect_array()//检测数组，并定义
{
	int i=0,j=0;
	char op[expr_size],fuc1[expr_size],fuc2[expr_size],op1[expr_size],op2[expr_size];
	num_v=0;
	for(i=0;i<num_inst-2;i++)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);//数组的格式mul，add，add
		sscanf(inst[i+1],"%s %s %s",op1,fuc1,fuc2);
		sscanf(inst[i+2],"%s %s %s",op2,fuc1,fuc2);
		if(strcmp(op,"mul")==0&&strcmp(op1,"add")==0&&strcmp(op2,"add")==0)
		{
			transform_array(i);
			i=i+2;
		}
	}

	int list[expr_size],num=0;
	define_var(var_name,num_v,list,num);
	for(i=0;i<num;i++)
		fprintf(fw,"long %s[%d]; \n",var_name[list[i]],array_size);

}

/*********************************局部数组的定义************************************/

void local_array(int start)//获取局部数组，并定义
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size];
	int i=0;
	for(i=start;i<num_inst;i++)
	{
		sscanf(inst[i],"%s",op);
		if(strcmp(op,"ret")==0)
			break;
		if(strcmp(op,"array")==0)//检测到辅助指令array
		{
			sscanf(inst[i],"%s %s %s %s",op,op,fuc1,fuc2);
			if(strcmp(fuc2,"FP")==0)
			{
				strcpy(var_name[num_v++],fuc1);
			}
		}
	}

	int list[expr_size],num=0;
	define_var(var_name,num_v,list,num);
	for(i=0;i<num;i++)
	{
		add_tab();
		fprintf(fw,"long %s[%d]; \n",var_name[list[i]],array_size);
	}
}



/*********************************结构体的转化和定义************************************/

void def_strcut_global()//定义全局结构体变量
{
	int i=0,j=0;
	for(i=0;i<num_s;i++)
	{
		for(j=i-1;j>=0;j--)
		{
			if(strcmp(struct_name[j],struct_name[i])==0)
				break;
		}
		if(j<0)
		{
			fprintf(fw,"long %s; \n",struct_name[i]);
		}
	}
}

void global_struct()//结构体定义
{
	int i=0;
	char op[expr_size],fuc1[expr_size],type[expr_size],fuc2[expr_size],dst[expr_size];

	for(i=1;i<num_inst;i++)//遍历所有指令，找到结构体的结构进行转换 结构体都变为单个变量
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,type);
		if(strcmp(op,"add")==0&&find_base(fuc1,dst))//找到base，结构体的标志1
		{
			sprintf(inst[i]," nop ");
			char temp[200];
			strcpy(temp,dst);
			int now=i;
			for(i=i+1;i<num_inst;i++)//offset代表结构体的进一步访问
			{
				sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
				int reg=0;
				get_op(fuc2,dst,reg);
				if(strcmp(op,"add")==0&&judge_offset(dst,fuc2))
				{
					sprintf(inst[i]," nop ");
					strcat(temp,"_");
					strcat(temp,fuc2);
				}
				else if(i==now+1)//如果只有base没有offset，说明是全局变量，使用了辅助指令global
				{
					sprintf(inst[i-1],"global %s ",temp);
					strcpy(var_name[num_v++],temp);//将全局变量保存下来
					break;
				}
				else if(i>now+1)//如果有offset，则说明为结构体，使用辅助指令struct，指明是结构体
				{
					sprintf(inst[i-1]," struct %s %s ",temp,type);
					if(strcmp(type,"GP")==0)
						strcpy(struct_name[num_s++],temp);//将全局结构体保存下来
					i--;
					break;
				}
				else
					break;
			}
		}
	}
	def_strcut_global();//定义检测到的全局结构体变量
}

/*********************************局部变量的定义，函数参数的定义************************************/

bool judge_two_op(char op[])//检测是否为两操作数
{
	if(strcmp(op,"add")==0||strcmp(op,"sub")==0||strcmp(op,"mul")==0||strcmp(op,"div")==0||strcmp(op,"mod")==0)
		return true;
	else if(strcmp(op,"move")==0||strcmp(op,"cmpeq")==0||strcmp(op,"cmple")==0||strcmp(op,"cmplt")==0)
		return true;
	else
		return false;
}

void sort_list(int list[],int num)
{
	int i=0,j=0,temp=0;
	for(i=0;i<num;i++)
	{
		for(j=i+1;j<num;j++)
		{
			if(param_place[list[i]]<param_place[list[j]])
			{
				temp=list[i];
				list[i]=list[j];
				list[j]=temp;
			}
		}
	}
}

void local_define(int start)//局部变量定义，变量，数组，结构体
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size];
	int i=0;
	num_p=0;
	num_v=0;

	char v1[expr_size],v2[expr_size];
	int t1,t2,reg1,reg2;
	for(i=start;i<num_inst;i++)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"ret")==0)
			break;
		if(judge_two_op(op))//双操作数，其中也可能包含变量，需要检测是否有局部变量
		{
			t1=get_op(fuc1,v1,reg1);
			t2=get_op(fuc2,v2,reg2);
			if(t1==3)
			{
				if(reg1>0)
				{
					param_place[num_p]=reg1;
					strcpy(param_name[num_p++],v1);
				}
				else
					strcpy(var_name[num_v++],v1);
			}
			if(t2==3)
			{
				if(reg2>0)
				{
					param_place[num_p]=reg1;
					strcpy(param_name[num_p++],v2);
				}
				else
					strcpy(var_name[num_v++],v2);
			}
		}
		else if(strcmp(op,"neg")==0||strcmp(op,"var")==0||strcmp(op,"param")==0||strcmp(op,"write")==0)//单操作数，其中可能包含变量，需要检测
		{
			t1=get_op(fuc1,v1,reg1);
			if(t1==3)
			{
				if(reg1>0)
				{
					param_place[num_p]=reg1;
					strcpy(param_name[num_p++],v1);
				}
				else
					strcpy(var_name[num_v++],v1);
			}
			if(strcmp(op,"var")==0)
				sprintf(inst[i]," nop ");
		}
		else if(strcmp(op,"struct")==0&&strcmp(fuc2,"FP")==0)//结构体定义
		{
			strcpy(var_name[num_v++],fuc1);
		}
	}

	int list[1000],num=0;
	define_var(param_name,num_p,list,num);//定义函数调用参数
	sort_list(list,num);
	for(i=0;i<num-1;i++)
		fprintf(fw,"long %s,",param_name[list[i]]);
	if(num-1>=0)
		fprintf(fw,"long %s",param_name[list[num-1]]);
	fprintf(fw,") \n{ \n");

	num_big++;

	num=0;
	define_var(var_name,num_v,list,num);//局部变量定义
	for(i=0;i<num;i++)
	{
		add_tab();
		fprintf(fw,"long %s; \n",var_name[list[i]]);
	}

	local_array(start);//局部数组定义

}


/***********************************分支指令和while***********************************/

bool judge_while(int now,int jump)//判断是否当前的cmp为while
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size];
	sscanf(inst[jump-1],"%s %s %s",op,fuc1,fuc2);
	if(strcmp(op,"br")==0)
	{
		int t=0;
		sscanf(fuc1,"[%d]",&t);
		if(t<=now)
			return true;
		else if(t>=jump)
		{
			sprintf(inst[jump-1],"else %d",t);
			extra[jump-1]++;
			extra[jump]--;
		}
	}
	return false;
}

//if和while的检测
void cmp_inst(char op[],char fuc1[],char fuc2[],char op1[],char op2[],int &i)
{
	char temp[expr_size];
	char v1[expr_size],v2[expr_size];
	int t1=0,t2=0,reg2=0,reg1=0;//flag用于指示是否为常数

	if(get_op(fuc1,v1,reg1)==2)
		get_expr(reg1,v1);
	if(get_op(fuc2,v2,reg2)==2)
		get_expr(reg2,v2);//获取判断语言的两个变量，或者是常数。如果是变量需要加到变量表中。
			
	sscanf(inst[i+1],"%s %s %s",op,fuc1,fuc2);
	int jump=0;
	sscanf(fuc2,"[%d]",&jump);
	bool flag_while=judge_while(i,jump);//判断是否可能为while循环

	if(strcmp(op,"blbc")==0)//正常的等于
	{
		if(flag_while)
			sprintf(temp,"while(%s%s%s) \n",v1,op1,v2);
		else
			sprintf(temp,"if(%s%s%s) \n",v1,op1,v2);
	}
	else if(strcmp(op,"blbs")==0)//等于的相反情况
	{
		if(flag_while)
			sprintf(temp,"while(%s%s%s) \n",v1,op2,v2);
		else
			sprintf(temp,"if(%s%s%s) \n",v1,op2,v2);
	}
	else
	{
		fprintf(fw,"cmpeq is wrong struct! \n");
	}
			
	extra[jump]++;//跳转的目的指令需要补全  }

	i++;//增加一条指令
	add_tab();//添加tab键
	fprintf(fw,"%s",temp);//放入缓存区
	add_tab();
	fprintf(fw,"{\n");
	num_big++;//{的个数增加一个
}

/*********************************简单运算指令************************************/

void group_expr(char dst[],char op[],int t1,int t2,char v1[],char v2[])
{
	if(t1==2)
		sprintf(dst,"(%s)%s",v1,op);
	else
		sprintf(dst,"%s%s",v1,op);
	if(t2==2)
		sprintf(dst,"%s(%s) ",dst,v2);
	else
		sprintf(dst,"%s%s ",dst,v2);
}

void get_expr(int now,char dst[])//获取表达式，now为第几条指令，dst为结果表达式
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size],temp[expr_size];
	sscanf(inst[now],"%s %s %s",op,fuc1,fuc2);
	if(strcmp(op,"ok")==0) //表达式是否已经完成组合
	{
		strcpy(dst,fuc1);
		return ;
	}
	else if(strcmp(op,"struct")==0||strcmp(op,"global")==0)//特殊辅助指令，分别为结构体和全局变量
	{
		strcpy(dst,fuc1);
		return ;
	}
	else if(strcmp(op,"array")==0)//数组的辅助指令
	{
		int t=0;
		sscanf(inst[now],"%s %s %s %s %s %d",op,op,fuc1,temp,fuc2,&t);
		if(strcmp(fuc2,"reg")==0)
		{
			get_expr(t,fuc2);
			sprintf(dst,"%s[(%s)]",fuc1,fuc2);
		}
		else
		{ 
			sprintf(dst,"%s",op);
		}
		return ;
	}

	char v1[expr_size],v2[expr_size];
	int reg1=0,reg2=0;
	if(strcmp(op,"neg")==0)//单操作数 单独处理
	{
		if(get_op(fuc1,v1,reg1)==2)//表达式
		{
			get_expr(reg1,v1);
			sprintf(dst,"-(%s)",v1);
		}
		else
		{
			sprintf(dst,"-%s",v1);
		}
		sprintf(inst[now],"ok %s ",dst);
	}
	else
	{
		int t1=get_op(fuc1,v1,reg1);
		if(t1==2)
			get_expr(reg1,v1);

		int t2=get_op(fuc2,v2,reg2);
		if(t2==2)
			get_expr(reg2,v2);        //获取两个操作数，可能是变量，常数，表达式

		if(strcmp(op,"add")==0)
			group_expr(dst,"+",t1,t2,v1,v2);
		else if(strcmp(op,"sub")==0)
			group_expr(dst,"-",t1,t2,v1,v2);
		else if(strcmp(op,"mul")==0)
			group_expr(dst,"*",t1,t2,v1,v2);
		else if(strcmp(op,"div")==0)
			group_expr(dst,"/",t1,t2,v1,v2);
		else if(strcmp(op,"mod")==0)
			group_expr(dst,"%",t1,t2,v1,v2);
		else if(strcmp(op,"move")==0)
			group_expr(dst,"=",t2,t1,v2,v1);
		else
		{
			printf("%s is not added\n",op);
		}
		sprintf(inst[now],"ok %s ",dst); //添加ok，表示已经解析过，可以直接使用
	}
}

bool simple_expr(char op[])//判断是否为简单运算指令
{
	if(strcmp(op,"neg")==0||strcmp(op,"move")==0)
		return true;
	if(strcmp(op,"add")==0||strcmp(op,"sub")==0)
		return true;
	if(strcmp(op,"div")==0||strcmp(op,"mul")==0)
		return true;
	if(strcmp(op,"mod")==0)
		return true;
	return false;
}

/*********************************load指令************************************/

void transform_load(int reg,int now)//转化为ok expr
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size],temp[expr_size];
	sscanf(inst[reg],"%s %s %s",op,fuc1,fuc2);
	if(strcmp(op,"global")==0)
	{
		sprintf(inst[now],"ok %s ",fuc1);
	}
	else if(strcmp(op,"array")==0)
	{
		int t=0;
		sscanf(inst[reg],"%s %s %s %s %s %d",op,op,fuc1,temp,fuc2,&t);
		if(strcmp(fuc2,"reg")==0)
		{
			get_expr(t,fuc2);
			sprintf(inst[now],"ok %s[(%s)] ",fuc1,fuc2);
		}
		else
		{ 
			sprintf(inst[now],"ok %s ",op);
		}
	}
	else if(strcmp(op,"struct")==0)
	{
		sprintf(inst[now],"ok %s ",fuc1);
	}
	else
	{
		printf("not realize!\n");
	}
}

/*********************************函数调用指令************************************/

void translate_fuc(int &now)//函数调用解析
{
	char op[expr_size],fuc1[expr_size],fuc2[expr_size];
	int i=0,end=0;
	for(i=now+1;i<num_inst;i++)//找到参数定义结束后的call指令
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"call")==0)
		{
			end=i;
			int reg=0;
			sscanf(fuc1,"[%d]",&reg);
			add_tab();
			fprintf(fw,"fuction_%d(",reg);
			break;
		}
		else if(strcmp(op,"load")==0)
		{
			int reg=0;
			sscanf(fuc1,"(%d)",&reg);
			transform_load(reg,i);//load指令的转化，转化为一个简单的表达式，用于之后的使用
		}
	}
	char v1[expr_size];
	int reg=0;
	int t=0;
	for(i=now;i<end;i++)//添加调用参数
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"param")==0)
		{
			t=get_op(fuc1,v1,reg);
			if(t==2)
				get_expr(reg,v1);
			fprintf(fw,"%s",v1);
			if(i==end-1)
				fprintf(fw,");\n");
			else
				fprintf(fw,",");
		}
	}
	now=end;
}