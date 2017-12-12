#include"ssa.h"

void show_phi(int f)
{
	cout <<"fuction:"<<f<<endl;
	for(int i=0;i<fuction[f].n_b;i++)
	{
		if(fuction[f].block[i].phi!=-1)
		{
			cout <<"block "<<fuction[f].block[i].s<<endl;
			for(int c=fuction[f].block[i].s;c<=fuction[f].block[i].e;c++)
				cout <<c<<" :"<<inst[c]<<endl;
			int t=fuction[f].block[i].phi;
			for(int j=0;j<Phi[t].n_p;j++)
				cout <<"insert phi:"<<Phi[t].phi[j]<<endl;
			cout <<endl;
		}
	}
}


void show_df(int f)
{
	for(int i=0;i<fuction[f].n_b;i++)
	{
		cout <<"df "<<fuction[f].block[i].s<<":";
		for(int j=0;j<fuction[f].n_b;j++)
			if(df[i][j])
				cout <<fuction[f].block[j].s<<" ";
		cout <<endl;
	}

}

void show_dom(int f)
{
	cout <<"fuction "<<f<<endl;
	for(int i=0;i<fuction[f].n_b;i++)
	{
		cout <<"dom "<<fuction[f].block[i].s<<":";
		for(int j=0;j<fuction[f].n_b;j++)
			cout <<(int)out[i][j]<<" ";
		cout <<"  idom:"<<fuction[f].block[idom[i]].s<<endl;
	}

}

void get_in_dom(int f,int b)
{
	for(int j=0;j<fuction[f].n_b;j++)
	{
		char flag=1;
		for(int i=0;i<fuction[f].block[b].num_pre;i++)
		{
			int t=fuction[f].block[b].pre[i];
			if(out[t][j]==0)
				flag=0;
		}
		in[b][j]=flag;
	}
}

bool cal_out_dom(int f,int b)
{
	bool flag=false;
	for(int i=0;i<fuction[f].n_b;i++)
	{
		if(in[b][i]==1||i==b)
		{
			out[b][i]=1;
		}
		else
			out[b][i]=0;
	}
	return flag;
}

void find_idom(int f,int b)
{
	for(int i=fuction[f].n_b-1;i>=0;i--)
	{
		if(out[b][i]==1&&i!=b)
		{
			idom[b]=i;
			break;
		}
	}
}

void dom_cal_fuc(int f)
{
	int i=0;
	for(i=0;i<fuction[f].n_b;i++)
		for(int j=0;j<fuction[f].n_b;j++)
			out[i][j]=1;
	memset(in,0,sizeof(in));

	
	memset(out[0],0,sizeof(out[0]));
	out[0][0]=1;
	for(i=1;i<fuction[f].n_b;i++)
	{
		get_in_dom(f,i);
		cal_out_dom(f,i);
	}
	for(i=0;i<fuction[f].n_b;i++)
		find_idom(f,i);
	//show_dom(f);
	idom[0]=-1;
}

void cal_df_block(int f,int b)
{
	if(flag_df[b]==1)
		return ;
	int i=0;
	for(i=0;i<fuction[f].block[b].n_p;i++)
	{
		int t=fuction[f].block[b].point[i];
		if(idom[t]!=b)
			df[b][t]=1;
	}
	for(i=0;i<fuction[f].n_b;i++)
	{
		if(idom[i]==b)
		{
			cal_df_block(f,i);
			for(int c=0;c<fuction[f].n_b;c++)
			{
				if(df[i][c]==1&&(idom[c]!=b||(c==b&&b!=0)))
				{
					df[b][c]=1;
				}
			}
		}
	}
	flag_df[b]=1;
}


void find_df(int f)
{
	memset(df,0,sizeof(df));
	memset(flag_df,0,sizeof(flag_df));
	memset(DF,0,sizeof(DF));
	for(int i=0;i<fuction[f].n_b;i++)
	{
		cal_df_block(f,i);
		for(int j=0;j<fuction[f].n_b;j++)
		{
			if(df[i][j]==1)
				DF[j]=1;
		}
	}
	//show_df(f);
}


void add_dvar(char name[],int place)
{
	for(int i=0;i<num_dvar;i++)
	{
		if(strcmp(name,dvar[i].name)==0)
		{
			dvar[i].block[Flag[place][0]]=1;
			Aorig[Flag[place][0]][i]=1;
			return ;
		}
	}
	strcpy(dvar[num_dvar].name,name);
	memset(dvar[num_dvar].block,0,sizeof(dvar[num_dvar].block));
	dvar[num_dvar].block[Flag[place][0]]=1;
	Aorig[Flag[place][0]][num_dvar]=1;
	num_dvar++;
}


void insert_phi(int f,int v,int n,char w[])//v var  n block   w whether
{
//	cout <<dvar[v].name<<" "<<fuction[f].block[n].s<<endl;
	for(int y=0;y<fuction[f].n_b;y++)
	{
		if(df[n][y]==0)
			continue;

		if(Aphi[y][v]==1)
			continue;

		if(fuction[f].block[y].phi==-1)
		{
			Phi[num_phi].block=y;
			Phi[num_phi].n_p=1;
			strcpy(Phi[num_phi].phi[0],dvar[v].name);
			fuction[f].block[y].phi=num_phi;
			num_phi++;
		}
		else
		{
			int t=fuction[f].block[y].phi;
			strcpy(Phi[t].phi[Phi[t].n_p++],dvar[v].name);
		}

		Aphi[y][v]=1;
		if(Aorig[y][v]!=1)
		{
			insert_phi(f,v,y,w);
		}
	}
}

void show_info1(int f)
{
	for(int i=0;i<num_dvar;i++)
	{
		cout <<"name:"<<dvar[i].name<<endl;
		for(int j=0;j<fuction[f].n_b;j++)
			if(dvar[i].block[j]==1)
				cout <<fuction[f].block[j].s<<" ";
		cout <<endl<<endl;
	}
}

bool insert_or_not(char w[],int f)
{
	int num=0;
	for(int j=0;j<fuction[f].n_b;j++)
	{
		if(w[j]==1)
			num++;
	}
	if(num<=1)
		return false;
	else
		return true;
}

void insert_phi(int f)
{
	char w[200];
//	show_info1(f);

	for(int i=0;i<num_dvar;i++)
	{
		for(int j=0;j<fuction[f].n_b;j++)
			w[j]=dvar[i].block[j];
		if(!insert_or_not(w,f))
		{
			sprintf(dvar[i].name,"111");
			continue;
		}
		for(int j=0;j<fuction[f].n_b;j++)
		{
			if(w[j]==0)continue;
			insert_phi(f,i,j,w);
		}
	}
}

void find_phi(int f)
{
	int i=0;
	char name[100];
	int value=0;
	memset(Aorig,0,sizeof(Aorig));
	memset(Aphi,0,sizeof(Aphi));
	num_dvar=0;
	memset(dvar,0,sizeof(dvar));
	for(i=fuction[f].start;i<=fuction[f].end;i++)
	{
		int t=judge_expr(i,name,value);
		if(t!=0)
		{
			add_dvar(name,i);
		}
	}
	insert_phi(f);
//	show_phi(f);
}
