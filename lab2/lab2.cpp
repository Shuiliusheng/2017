#include"lab2.h"

Fuction fuction[100];
int num_fuc;

char inst[MAX][expr_size];
int num_inst;
int Flag[MAX][3];

char gen[500][100];
char kill[500][100];
char in[500][100];
char out[500][100];
char constant_num[500];
FILE *cfg;
FILE *const_rep;
FILE *inst_new;
FILE *dead_rep;
Expr expr[500];
int num_expr;

void debug()
{
	char filename[300]="g:\\asm\\scp.asm";
	load_inst(filename);

	char f1[300];
	strcpy(f1,filename);
	strcat(f1,".cfg");
	cfg=fopen(f1,"w");
	CFG();

	strcpy(f1,filename);
	strcat(f1,".const.rep");
	const_rep=fopen(f1,"w");
	constant_spread();

	//strcpy(f1,filename);
	//strcat(f1,".dead.rep");
	//dead_rep=fopen(f1,"w");
	//elimination_deadcode();

	strcpy(f1,filename);
	strcat(f1,".3addr");
	inst_new=fopen(f1,"w");
	write_inst();

	strcpy(f1,filename);
	strcat(f1,".new.c");
	Recompiler(f1);

}

void param(int argc,char **argv)
{
	char filename[300]="";
	if(argc>2)
	{
		strcpy(filename,argv[1]);
		load_inst(filename);
		if(argc==3)
		{
			if(strcmp(argv[2],"-backend=c")==0)
			{
				strcat(filename,".new.c");
				Recompiler(filename);
			}
			else if(strcmp(argv[2],"-backend=cfg")==0)
			{
				strcat(filename,".cfg");
				cfg=fopen(filename,"w");
				CFG();
			}
			else
			{
				cout <<"param wrong!"<<endl;
				cout <<"Example: .//run filename -backend=c"<<endl;
			}
		}
		else
		{
			if(strcmp(argv[2],"-opt=scp")==0)
			{
				char f1[300];
				strcpy(f1,filename);
				strcat(f1,".cfg");
				cfg=fopen(f1,"w");
				CFG();

				strcpy(f1,filename);
				strcat(f1,".const.rep");
				const_rep=fopen(f1,"w");
				constant_spread();


				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					strcpy(f1,filename);
					strcat(f1,".3addr");
					inst_new=fopen(f1,"w");
					write_inst();
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					strcpy(f1,filename);
					strcat(f1,".new.c");
					Recompiler(f1);
				}
			}
			else if(strcmp(argv[2],"-opt=dse")==0)
			{
				char f1[300];
				strcpy(f1,filename);
				strcat(f1,".cfg");
				cfg=fopen(f1,"w");
				CFG();

				strcpy(f1,filename);
				strcat(f1,".dead.rep");
				dead_rep=fopen(f1,"w");
				elimination_deadcode();

				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					strcpy(f1,filename);
					strcat(f1,".3addr");
					inst_new=fopen(f1,"w");
					write_inst();
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					strcpy(f1,filename);
					strcat(f1,".new.c");
					Recompiler(f1);
				}
				else
				{
					cout <<"param wrong!"<<endl;
					cout <<"Example: -backend=3addr//c//cfg//rep"<<endl;
				}
			}
		}
	}
	else
	{
		cout <<"param wrong!"<<endl;
		cout <<"Example: .//run filename -opt=wcp -backend=c"<<endl;
	}
}

int main(int argc,char **argv)
{
	//debug();
	param(argc,argv);
//	pause();
	return 0;
}

