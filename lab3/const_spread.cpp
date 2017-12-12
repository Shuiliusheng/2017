#include"ssa.h"
#define GP 2
#define FP 1
Phi_expr ephi[200];
int num_ep;

typedef struct{
	char name[100];
	int val;
	int type;
}Const_var;

Const_var cvar[200];
int num_cvar=0;

bool spread_flag;

int num_spread;

void init(int f)
{
	num_ep=0;
	for(int i=0;i<fuction[f].n_b;i++)
	{
		int block=fuction[f].block[i].phi;
		if(block!=-1)
		{
			for(int c=0;c<phib[block].n_expr;c++)
			{
				ephi[num_ep++]=phib[block].pexpr[c];
				memset(ephi[num_ep-1].place,-1,sizeof(ephi[num_ep-1].place));
				memset(ephi[num_ep-1].flag,0,sizeof(ephi[num_ep-1].flag));
			}
		}
	}
}

int var_type(char name[])
{
	int l=strlen(name);
	for(int i=0;i<l-5;i++)
	{
		if(name[i]=='_'&&name[i+1]=='b'&&name[i+2]=='a')
			return GP;
	}
	return FP;
}

void add_cvar(char name[],int value)
{
	int i=0;
	for(i=0;i<num_ep;i++)
		if(strcmp(ephi[i].name,name)==0)
			return ;
	strcpy(cvar[num_cvar].name,name);
	cvar[num_cvar].type=var_type(name);
	cvar[num_cvar++].val=value;
}

void const_var(int f)
{
	num_cvar=0;
	char name[100]="";
	int value;
	for(int i=fuction[f].start;i<=fuction[f].end;i++)
	{
		int t=judge_expr1(i,name,value);
		if(t==1)
			add_cvar(name,value);
	}
}

int get_value(char vname[])
{
	for(int i=0;i<num_cvar;i++)
		if(strcmp(cvar[i].name,vname)==0)
			return i;
	return -1;
}

bool judge_phi_fuc(int n)
{
	for(int j=0;j<ephi[n].n_param;j++)
	{
		for(int c=0;c<num_cvar;c++)
		{
			if(strcmp(ephi[n].param[j],cvar[c].name)==0)
			{
				ephi[n].place[j]=cvar[c].val;
				ephi[n].flag[j]=1;
				break;
			}
		}
	}
	int t=ephi[n].place[0];
	for(int j=0;j<ephi[n].n_param;j++)
		if(ephi[n].flag[j]!=1||t!=ephi[n].place[j])
			return false;
	return true;
}

void judge_phi_fuc()
{
	for(int i=0;i<num_ep;i++)
	{
		if(judge_phi_fuc(i))
		{
			add_cvar(ephi[i].name,ephi[i].place[0]);
		}
	}
}

void change_load_const(int n,int v,int f)
{

	sprintf(inst_temp[n],"nop");
	sprintf(inst_temp[n+1],"nop");
	
	char temp[20]="";
	sprintf(temp,"(%d)",n+1);
	char op[100]="",fuc1[100]="",fuc2[100]="";
	for(int i=n+2;i<=fuction[f].end;i++)
	{
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(fuc1,temp)==0)
		{
			sprintf(fuc1,"%d",v);
			sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		}
		if(strcmp(fuc2,temp)==0)
		{
			sprintf(fuc2,"%d",v);
			sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
			break;
		}
	}
}

void change_cvar(int f)
{
	for(int i=fuction[f].start;i<=fuction[f].end;i++)
	{
		char op[100]="",fuc1[100]="",fuc2[100]="";
		int v1=0,v2=0;
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		v1=get_value(fuc1);
		int temp=0;
		if(v1!=-1)
		{
			if(cvar[v1].type==FP)
			{
				temp=1;
				spread_flag=true;
				sprintf(fuc1,"%d",cvar[v1].val);
			}
			else if(cvar[v1].type==GP)
			{
				char src1[100],src2[100],src3[100]="";
				sscanf(inst_temp[i+1],"%s %s %s",src1,src2,src3);
				if(strcmp(src1,"load")==0)
				{
					temp=1;
					spread_flag=true;
					change_load_const(i,cvar[v1].val,f);
					i++;
					continue;
				}
			}
		}

		v2=get_value(fuc2);
		if(v2!=-1)
		{
			if(strcmp(op,"move")!=0)
			{
				temp=1;
				spread_flag=true;
				sprintf(fuc2,"%d",cvar[v2].val);
			}
		}
		fuction[f].ntemp+=temp;
		sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
	}
}

bool judge_const(char name[],int &v)
{
	if(name[0]<='9'&&name[0]>='0'||name[0]=='-')
	{
		sscanf(name,"%d",&v);
		return true;
	}
	return false;
}

void change_expr(int f,int n,int value)
{
	sprintf(inst_temp[n],"nop");

	char temp[20]="";
	sprintf(temp,"(%d)",n);
	char op[100]="",fuc1[100]="",fuc2[100]="";
	for(int i=n+1;i<=fuction[f].end;i++)
	{
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		int t1=0;
		if(strcmp(fuc1,temp)==0)
		{
			sprintf(fuc1,"%d",value);
			spread_flag=true;
			t1=1;
			sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		}
		if(strcmp(fuc2,temp)==0)
		{
			sprintf(fuc2,"%d",value);
			sprintf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
			spread_flag=true;
			t1=1;
			fuction[f].ntemp+=t1;
			break;
		}
	}
}

void delete_simple_expr(char op[],int v1,int v2,int n,int f)
{
	int value=0;
	if(strcmp(op,"add")==0)
		value=v1+v2;
	else if(strcmp(op,"sub")==0)
		value=v1-v2;
	else if(strcmp(op,"mul")==0)
		value=v1*v2;
	else if(strcmp(op,"div")==0)
		value=v1/v2;
	else if(strcmp(op,"mod")==0)
		value=v1%v2;
	else 
		return ;

	spread_flag=true;
	change_expr(f,n,value);
}

void delete_simple_expr(int f)
{
	for(int i=fuction[f].start;i<=fuction[f].end;i++)
	{
		char op[100]="",fuc1[100]="",fuc2[100]="";
		int v1=0,v2=0;
		sscanf(inst_temp[i],"%s %s %s",op,fuc1,fuc2);
		if(judge_const(fuc1,v1)&&judge_const(fuc2,v2))
		{
			delete_simple_expr(op,v1,v2,i,f);
		}
		else if(strcmp(op,"neg")==0&&judge_const(fuc1,v1))
		{
			spread_flag=true;
			change_expr(f,i,-1*v1);
		}
		else if(strcmp(op,"move")==0&&strcmp(fuc1,fuc2)==0)
		{
			sprintf(inst_temp[i],"nop");
		}
	}
}

void const_spread_fuc(int f)
{
	init(f);
	//cout <<"1 init"<<endl;
	const_var(f);
//	cout <<"2 find const"<<endl;
	judge_phi_fuc();
//	cout <<"3 find phi const"<<endl;
	change_cvar(f);
//	cout <<"4 change const var"<<endl;
	delete_simple_expr(f);
//	cout <<"5 delete expr"<<endl;
}

bool reconst_spread()
{
	spread_flag=false;
	for(int i=0;i<num_fuc;i++)
	{
		const_spread_fuc(i);
	}
	return spread_flag;
}

bool judge_cmp_inst(int n,bool &cmp)
{
	char op[100],fuc1[100]="",fuc2[100]="";
	sscanf(inst_temp[n],"%s %s %s",op,fuc1,fuc2);
	int v1=0,v2=0;
	if(judge_const(fuc1,v1)&&judge_const(fuc2,v2))
	{
		if(strcmp(op,"cmpeq")==0)
		{
			if(v1==v2) cmp=true;
			else cmp=false;
		}
		else if(strcmp(op,"cmple")==0)
		{
			if(v1<=v2) cmp=true;
			else cmp=false;
		}
		else if(strcmp(op,"cmplt")==0)
		{
			if(v1<v2) cmp=true;
			else cmp=false;
		}
		else
			return false;
	}
	else
		return false;
	return true;
}

bool judge_jump_inst(int n,bool cmp,int &dst)// true jump ; false no jump
{
	char op[100]="",fuc1[100]="",fuc2[100]="";
	sscanf(inst_temp[n],"%s %s %s",op,fuc1,fuc2);
	sscanf(fuc2,"[%d]",&dst);
	if(strcmp(op,"blbc")==0)
	{
		return !cmp;
	}
	if(strcmp(op,"blbs")==0)
		return cmp;
}

int judge_if_else_while(int now,int jump,int &edst)//判断是否当前的cmp为while
{
	char op[100],fuc1[100],fuc2[100];
	sscanf(inst_temp[jump-1],"%s %s %s",op,fuc1,fuc2);

	if(strcmp(op,"br")==0)
	{
		sscanf(fuc1,"[%d]",&edst);
		if(edst<=now)
			return 3;
		else if(edst>=jump)
			return 2;
	}
	return 1;//if
}

void eliminate_branch()
{
	bool cmp=false;
	for(int i=1;i<num_temp;i++)
	{
		if(judge_cmp_inst(i,cmp))
		{
			int dst=0;
			if(judge_jump_inst(i+1,cmp,dst))
			{
				for(int c=i;c<dst;c++)
				{
					sprintf(inst_temp[c],"nop");
				}
				i++;
			}
			else
			{
				int edst=0;
				int t=judge_if_else_while(i,dst,edst);
				sprintf(inst_temp[i],"nop");
				sprintf(inst_temp[i+1],"nop");
				if(t==2)
				{
					for(int c=dst-1;c<edst;c++)
					{
						sprintf(inst_temp[c],"nop");
					}
				}
				else if(t==3)
				{
					sprintf(inst_temp[dst-1],"nop");
				}
			}
		}
	}
}

void write_const_spread(char filename[])// 3addr result save
{
	FILE *p=fopen(filename,"w");
	for(int i=1;i<num_temp;i++)
	{
		fprintf(p,"    instr %d: %s\n",i,inst_temp[i]);
	}
	fclose(p);
}

void write_const_rep(char filename[])//report write
{
	FILE *p=fopen(filename,"w");
	for(int i=0;i<num_fuc;i++)
	{
		fprintf(p,"Fuction: %d\n",fuction[i].start);
		fprintf(p,"Number of constants propagated: %d times\n",fuction[i].ntemp);
	}
	fclose(p);
}

void const_spread()
{
	int i=1;
	for(int i=0;i<num_fuc;i++)
		fuction[i].ntemp=0;
	while(reconst_spread());
	eliminate_branch();
}
