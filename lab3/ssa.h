#include"lab3.h"

typedef struct{
	char name[50];
	char param[20][50];
	int place[20];
	char flag[20];
	int n_param;
}Phi_expr;

typedef struct{
	Phi_expr pexpr[100];
	int block;
	int n_expr;
}Phi_B;

typedef struct{
	int s,e;
	char name[50];
	char use[100];
	int flag;
	int type;
}Def_expr;

void insert_phi(int f);
void insert_phi(int f,int v,int n,char w[]);
void add_dvar(char name[],int place);
void find_df(int f);
void cal_df_block(int f,int b);
void dom_cal_fuc(int f);
void find_idom(int f,int b);
void find_phi(int f);
void rename_var(int f);
void recovery_phi();
void const_spread();
int find_def_varname(char name[]);
int judge_dvar(char name[]);

extern int count[200];
extern PHI Phi[200];
extern int num_phi;

extern Dvar dvar[100];
extern int num_dvar;

extern int idom[200];
extern char df[500][200];
extern char flag_df[500];
extern char DF[200];

extern char (*Aorig)[100];
extern char (*Aphi)[100];
extern Phi_B phib[100];

extern char inst_temp[MAX][200];
extern int num_temp;
extern int new_place[MAX];