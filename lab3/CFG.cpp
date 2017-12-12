#include"lab3.h"

void first_inst()
{
	int i=0,j=0;
	char op[100],fuc1[100],fuc2[100];
	for(i=1;i<num_inst;i++)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);
		if(strcmp(op,"enter")==0)
		{
			fuction[num_fuc].start=i;
			Flag[i][0]=1;
		}
		else if(strcmp(op,"ret")==0)
		{
			fuction[num_fuc].end=i;
			num_fuc++;
		}
		else if(strcmp(op,"blbc")==0||strcmp(op,"blbs")==0)
		{
			int temp=0;
			sscanf(fuc2,"[%d]",&temp);
			Flag[temp][0]=1;
			Flag[i+1][0]=1;
		}
		else if(strcmp(op,"br")==0)
		{
			int temp=0;
			sscanf(fuc1,"[%d]",&temp);
			Flag[temp][0]=1;
			Flag[i+1][0]=1;
		}
		else if(strcmp(op,"call")==0)
		{
			Flag[i+1][0]=1;
		}
	}
}

void point_analys(int n)
{
	int i=0,j=0;
	char op[100],fuc1[100],fuc2[100];
	fuction[n].n_b=0;
	int last_i=fuction[n].start;
	int num_block=-1;
	for(i=fuction[n].start;i<=fuction[n].end;i++)
	{
		if(Flag[i][0]==1)
		{	
			num_block++;
			if(num_block!=0)
				fuction[n].block[num_block-1].e=i-1;
			fuction[n].block[num_block].s=i;
			fuction[n].block[num_block].n_p=0;
			last_i=i+1;
			Flag[i][0]=num_block;
		}
		else
			Flag[i][0]=num_block;
	}
	fuction[n].block[num_block].e=fuction[n].end;

	num_block++;
	fuction[n].n_b=num_block;

	for(i=fuction[n].start;i<=fuction[n].end;i++)
	{
		sscanf(inst[i],"%s %s %s",op,fuc1,fuc2);

		int t=Flag[i][0];
		if(t!=Flag[i+1][0]&&i!=fuction[n].end)
		{
			if(!(strcmp(op,"br")==0))
			{
				int t1=fuction[n].block[t].n_p;
				fuction[n].block[t].point[t1]=Flag[i+1][0];
				fuction[n].block[t].n_p=t1+1;
			}
		}

		if(strcmp(op,"blbc")==0||strcmp(op,"blbs")==0)
		{
			int temp=0;
			sscanf(fuc2,"[%d]",&temp);
			int t1=fuction[n].block[t].n_p;
			fuction[n].block[t].point[t1]=Flag[temp][0];
			fuction[n].block[t].n_p=t1+1;
		}
		else if(strcmp(op,"br")==0)
		{
			int temp=0;
			sscanf(fuc1,"[%d]",&temp);
			int t1=fuction[n].block[t].n_p;
			fuction[n].block[t].point[t1]=Flag[temp][0];
			fuction[n].block[t].n_p=t1+1;
		}
	}
	fprintf(cfg,"Function: %d\n",fuction[n].start);
	fprintf(cfg,"Basic blocks: ");
	for(i=0;i<num_block;i++)
	{
		fprintf(cfg,"%d ",fuction[n].block[i].s);
	}
	fprintf(cfg,"\nCFG:\n");

	for(i=0;i<num_block;i++)
	{
		fprintf(cfg,"%d -> ",fuction[n].block[i].s);
		int t=fuction[n].block[i].n_p;
		for(int j=0;j<t;j++)
		{
			int t1=fuction[n].block[i].point[j];
			fprintf(cfg,"%d ",fuction[n].block[t1].s);
		}
		fprintf(cfg,"\n");
	}

}

void find_pre_block(int f)
{
	int i=0;
	for(i=0;i<fuction[f].n_b;i++)
		fuction[f].block[i].num_pre=0;
	for(i=0;i<fuction[f].n_b;i++)
	{
		for(int j=0;j<fuction[f].block[i].n_p;j++)
		{
			int t=fuction[f].block[i].point[j];
			int t1=fuction[f].block[t].num_pre;
			fuction[f].block[t].pre[t1]=i;
			fuction[f].block[t].num_pre=t1+1;
		}
	}
}

void CFG()
{
	int i=0;
	first_inst();
	for(i=0;i<num_fuc;i++)
	{
		point_analys(i);
		find_pre_block(i);
	}
	fclose(cfg);
	//cout <<"make cfg accomplish!"<<endl;
}