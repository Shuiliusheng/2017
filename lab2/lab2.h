#include<iostream>
#include<stdio.h>
#include<string.h>
using namespace std;
#define SIZE 500

#define MAX 20000
#define array_size 400
#define expr_size 1000

typedef struct{
	int s,e;
	int n_p;
	int num_pre;
	int pre[100];
	int point[100];
}Block;

typedef struct{
	int start,end;
	int n_b;
	Block block[100];
}Fuction;

typedef struct{
	char name[100];
	char kill[100];
	int place;
	int value;
	int type;
}Expr;


extern Fuction fuction[100];
extern int num_fuc;

extern char inst[MAX][expr_size];
extern int num_inst;
extern int Flag[MAX][3];

extern char gen[500][100];
extern char kill[500][100];
extern char in[500][100];
extern char out[500][100];
extern char constant_num[500];
extern FILE *cfg;
extern FILE *const_rep;
extern FILE *inst_new;
extern FILE *dead_rep;
extern Expr expr[500];
extern int num_expr;

void CFG();
bool load_inst(char filename[]);
void pause();
void show();
void constant_spread();
void write_inst();
void elimination_deadcode();
int judge_expr(int n,char name[],int &value);
int get_op(char src[],char dst[],int &reg);
void Recompiler(char filename[]);