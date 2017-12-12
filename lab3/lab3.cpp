#include"lab3.h"
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

void change_filename(char name[],char dst[],char end[])
{
	int i=0;
	int l=strlen(name);
	strcpy(dst,name);
	for(i=l-1;i>=0;i--)
		if(name[i]=='.')
			break;
	l=strlen(end);
	for(int c=0;c<=l;c++)
		dst[i+c]=end[c];
}

void copy_inst()
{
	for(int i=1;i<num_temp;i++)
	{
		strcpy(inst[i],inst_temp[i]);
	}
	num_inst=num_temp;
}

void debug()
{
	char filename[300]="g:\\asm\\test\\loop.asm";
	load_inst(filename);
	char f1[300];
	change_filename(filename,f1,".cfg");
	cfg=fopen(f1,"w");
	CFG();
	ssa_cal();

	//	write_phi("test");

	recovery_phi();
//	write_new_phi("g:\\asm\\test\\gcd_without_phi.asm");
	const_spread();
//	write_const_spread("g:\\asm\\test\\collatz_const3.asm");
	invariant_move();	
//  write_invariant("g:\\asm\\test\\collatz_move.asm");

	//write_const_rep(char filename[]);
//invariant_report_write(char filename[]);

}

void param(int argc,char **argv)
{
	char filename[300]="",f1[300];
	if(argc>2)
	{
		strcpy(filename,argv[1]);
		load_inst(filename);
		if(argc==3)
		{
			if(strcmp(argv[2],"-backend=c")==0)
			{
				change_filename(filename,f1,"_normal.c");
				Recompiler(f1);
			}
			else if(strcmp(argv[2],"-backend=cfg")==0)
			{
				change_filename(filename,f1,".cfg");
				cfg=fopen(f1,"w");
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
			char f1[300];
			change_filename(filename,f1,".cfg");
			cfg=fopen(f1,"w");
			CFG();
			ssa_cal();
			if(strcmp(argv[2],"-opt=ssa")==0)
			{
				if(strcmp(argv[3],"-backend=ssa,3addr")==0)
				{
					change_filename(filename,f1,"_ssa.3addr");
					write_phi(f1);
				}
				else if(strcmp(argv[3],"-backend=3addr")==0)
				{
					change_filename(filename,f1,"_non_ssa.3addr");
					recovery_phi();
					write_new_phi(f1);
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					change_filename(filename,f1,"_ssa.c");
					recovery_phi();
					copy_inst();
					Recompiler(f1);
				}
				else
				{
					cout <<argv[3]<<" is not supported!"<<endl;
				}
			}
			else if(strcmp(argv[2],"-opt=ssa,scp")==0)
			{
				recovery_phi();
				const_spread();
				change_filename(filename,f1,"_ssa_scp.rep");
				write_const_rep(f1);

				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					change_filename(filename,f1,"_nonssa_scp.3addr");
					write_const_spread(f1);
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					change_filename(filename,f1,"_ssa_scp.c");
					copy_inst();
					Recompiler(f1);
				}
				else
				{
					cout <<argv[3]<<" is not supported!"<<endl;
				}
			}
			else if(strcmp(argv[2],"-opt=ssa,licm")==0)
			{
				recovery_phi();

				invariant_move();
				change_filename(filename,f1,"_ssa_licm.rep");
				invariant_report_write(f1);

				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					change_filename(filename,f1,"_nonssa_licm.3addr");
					write_invariant(f1);
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					change_filename(filename,f1,"_ssa_licm.c");
					copy_inst();
					Recompiler(f1);
				}
				else
				{
					cout <<argv[3]<<" is not supported!"<<endl;
				}
			}
			else if(strcmp(argv[2],"-opt=ssa,scp,licm")==0)//ssa scp licm 
			{
				recovery_phi();

				const_spread();
				change_filename(filename,f1,"_ssa_scp.rep");
				write_const_rep(f1);

				invariant_move();
				change_filename(filename,f1,"_ssa_licm.rep");
				invariant_report_write(f1);

				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					change_filename(filename,f1,"_nonssa_scp_licm.3addr");
					write_invariant(f1);
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					change_filename(filename,f1,"_ssa_scp_licm.c");
					copy_inst();
					Recompiler(f1);
				}
				else
				{
					cout <<argv[3]<<" is not supported!"<<endl;
				}
			}

			else if(strcmp(argv[2],"-opt=ssa,licm,scp")==0)//ssa licm scp
			{
				recovery_phi();

				invariant_move();
				change_filename(filename,f1,"_ssa_licm.rep");
				invariant_report_write(f1);

				const_spread();
				change_filename(filename,f1,"_ssa_scp.rep");
				write_const_rep(f1);

				if(strcmp(argv[3],"-backend=3addr")==0)
				{
					change_filename(filename,f1,"_nonssa_licm_scp.3addr");
					write_const_spread(f1);
				}
				else if(strcmp(argv[3],"-backend=c")==0)
				{
					change_filename(filename,f1,"_ssa_licm_scp.c");
					copy_inst();
					Recompiler(f1);
				}
				else
				{
					cout <<argv[3]<<" is not supported!"<<endl;
				}
			}

			else
			{
				cout <<argv[2]<<" is not supported!"<<endl;
			}
		}
	}
	else
	{
		cout <<"param wrong!"<<endl;
		cout <<"Example: .//run filename -opt=ssa,scp -backend=c"<<endl;
	}
}

int main(int argc,char **argv)
{
	//debug();
	param(argc,argv);
	//pause();
	return 0;
}

