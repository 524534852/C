#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

struct var {
	char* key;
	char* value;
	struct var* next;
};
typedef struct var Var;

struct prog {
	char** wds;
	int cw;
	int aw;
	Var* head;
};
typedef struct prog Program;



void Prog(Program *p);
Program* read_file(char* s);
Program* initial_prog();
Var* initial_Var();
void INSTRS(Program* p);
void INSTRUCT(Program* p);
int PRINT(Program* p);
int VARCON(Program* p);
int VAR(Program *p);
int CON(Program *p);
int STRVAR(Program *p);
int NUMVAR(Program* p);
int STRCON(Program* p);
int NUMCON(Program* p);
int JUMP(Program* p);
int stringToNumber(char* str);
int RND(Program* p);
int SET(Program* p);
int File(Program* p);
int ABORT(Program* p);
int nFILE(Program* p);
int INPUT(Program* p);
int IFCOND(Program* p);
int INC(Program* p);
void searchNum(Program* p,char* s,int tmp);
void searchStr(Program* p,char* s,int tmp);
void printHelp(Program* p,char* str,int tmp);
char hash(char c);
void insert(Var* v,Program *p);
char* numToString(int num);
void numInc(char* s);
void searchNumInc(Program* p,char* s);
void setVariable(Program* p,int keyNum,int valNum);
void inputVariable(Program* p,int keyNum);
char * searchVariable(Program* p,char* s);
int judgeCond(Program* p,int keyNum1,int keyNum2,int condNum);
#define strsame(A,B)  (strcmp(A,B)==0)
#define TRUE 1
#define FALSE 0
#define ERROR(PHRASE) {fprintf(stderr,\
	                               "Fatal Error %s occured in %s, line %d\n",PHRASE, __FILE__, __LINE__); exit(2); }

int main(int argc,char** argv) {
	Program* prog=NULL;
	srand((unsigned)time(NULL));
	prog=read_file(argv[1]);
	Prog(prog);
	printf("PARSED OK!");
	return 0;

}

Program* initial_prog() {
	Program* prog;
	prog=(Program*)malloc(sizeof(Program));
	prog->cw=0;
	prog->aw=0;
	prog->wds=NULL;
	prog->head=NULL;
	return prog;
}

Var* initial_Var() {
	Var* v;
	v=(Var*)malloc(sizeof(Var));
	v->key=NULL;
	v->value=NULL;
	v->next=NULL;
	return v;
}

Program* read_file(char* s) {
	FILE* fp;
	Program* prog;
	int i,k;
	char buf[256]= {0};
	if(!(fp=fopen(s,"r"))) {
		fprintf(stderr,"Cannot open %s\n",s);
		exit(2);
	}
	prog=initial_prog();
	while(fscanf(fp,"%s",buf)==1) {
		prog->wds=(char**)realloc(prog->wds,(prog->aw+1)*sizeof(char*));
		if(buf[0]=='\"') {
			i=strlen(buf);
			if(buf[i-1]!='\"') {
				while((k=getc(fp))!='\"'&&k!=EOF) {
					if(k!='\0')
						buf[i++]=k;
				}
				buf[i++]=k;
				buf[i++]='\0';
			}
		} else if(buf[0]=='#') {
			i=strlen(buf);
			if(buf[i-1]!='#') {
				while((k=getc(fp))!='#'&&k!=EOF) {
					if(k!='\0')
						buf[i++]=k;
				}
				buf[i++]=k;
				buf[i++]='\0';
			}
		}
		prog->wds[prog->aw]=(char*)malloc((strlen(buf)+1)*sizeof(char));
		strcpy(prog->wds[prog->aw],buf);
		prog->aw++;
	}
	return prog;

}


void Prog(Program *p) {
	if(!strsame(p->wds[p->cw],"{")) {
		ERROR("No { at the beginning?");
	}
	p->cw++;
	INSTRS(p);
	if(!strsame(p->wds[p->cw],"}")) {
		ERROR("No } at the endding?");
	}
	return;
}

void INSTRS(Program* p) {
	if(p->cw>=p->aw) {
		ERROR("No } at the ending?");
	}
	if(strsame(p->wds[p->cw],"}")) {
		return;
	}
	INSTRUCT(p);
	INSTRS(p);
	return;
}

void INSTRUCT(Program* p) {
	PRINT(p);
	JUMP(p);
	RND(p);
	SET(p);
	nFILE(p);
	ABORT(p);
	nFILE(p);
	INPUT(p);
	IFCOND(p);
	INC(p);
	//如果都不是，返回错误，不存在这样的指令
}


int PRINT(Program* p) {
	int tmp;
	if(strsame(p->wds[p->cw],"PRINT")||strsame(p->wds[p->cw],"PRINTN")) {
		tmp=p->cw;
		p->cw++;
		if(p->cw<p->aw&&VARCON(p)) {
			printHelp(p,p->wds[p->cw],tmp);
		} else
			ERROR("Invalid PRINT!");
	} else return FALSE;
	p->cw++;
	return TRUE;
}

void printHelp(Program* p,char* str,int tmp) {
	char buf[256]= {0};
	int i=0;
	char* s;
	if(str[0]=='\"') {
		for(i=1; str[i]!='\"'; i++) {
			buf[i-1]=str[i];
		}
		if(strsame(p->wds[tmp],"PRINT")) {
			printf("%s\n",buf);
		} else {
			printf("%s",buf);
		}
	} else if(str[0]=='#') {
		for(i=1; str[i]!='#'; i++) {
			buf[i-1]=hash(str[i]);
		}
		if(strsame(p->wds[tmp],"PRINT")) {
			printf("%s\n",buf);
		} else {
			printf("%s",buf);
		}
	} else if(str[0]=='%') {
		searchNum(p,str,tmp);
	} else if(str[0]=='$') {
		searchStr(p,str,tmp);
	} else if(isdigit(str[0])) {
		if(strsame(p->wds[tmp],"PRINT")) {
			printf("%s\n",str);
		} else {
			printf("%s",str);
		}
	}
}

void searchNum(Program* p,char* s,int tmp) {
	Var* v=p->head;
	while(v!=NULL&&!strsame(s,v->key)) {
		v=v->next;
	}
	if(v==NULL) {
		ERROR("Undefined var");
	}
	if(v->value[0]=='%') {
		searchNum(p,v->value,tmp);
	} else {
		printHelp(p,v->value,tmp);
	}
	return;
}

void searchStr(Program* p,char* s,int tmp) {
	Var* v=p->head;
	while(v!=NULL&&!strsame(s,v->key)) {
		v=v->next;
	}
	if(v==NULL) {
		ERROR("Undefined var");
	}
	if(v->value[0]=='$') {
		searchStr(p,v->value,tmp);
	} else {
		printHelp(p,v->value,tmp);
	}
}

char hash(char c) {
	if(c-'A'>=13&&c-'A'<26) {
		return c-13;
	} else if(c-'A'>=0&&c-'A'<13) {
		return c+13;
	} else if(c-'a'>=13&&c-'a'<26) {
		return c-13;
	} else if(c-'a'>=0&&c-'a'<13) {
		return c+13;
	} else if(c-'0'>=0&&c-'0'<5) {
		return c+5;
	} else if(c-'0'>=5&&c-'0'<=9) {
		return c-5;
	}
	return c;
}
int VARCON(Program* p) {
	return VAR(p)||CON(p);
}

int VAR(Program *p) {
	return STRVAR(p)||NUMVAR(p);
}

int CON(Program *p) {
	return STRCON(p)||NUMCON(p);
}

int STRVAR(Program *p) {
	int i=1;
	if(p->wds[p->cw][0]!='$') {
		return FALSE;
	}
	while(isupper(p->wds[p->cw][i])) {
		i++;
	}
	if(p->wds[p->cw][i]!='\0') {
		ERROR("Invalid STRVAR!");
	}
	return TRUE;
}

int NUMVAR(Program* p) {
	int i=1;
	if(p->wds[p->cw][0]!='%') {
		return FALSE;
	}
	while(isupper(p->wds[p->cw][i])) {
		i++;
	}
	if(p->wds[p->cw][i]!='\0') {
		ERROR("Invalid NUMVAR!");
	}
	return TRUE;
}

int STRCON(Program* p) {
	int i=1;
	if(p->wds[p->cw][0]!='#'&&p->wds[p->cw][0]!='\"') {
		return FALSE;
	}
	if(p->wds[p->cw][0]=='#') {
		while(p->wds[p->cw][i]!='#'&&p->wds[p->cw][i]!='\0') {
			i++;
		}
		if(p->wds[p->cw][i]=='\0') {
			ERROR("Invalid STRCON");
		}
		i++;
		if(p->wds[p->cw][i]!='\0') {
			ERROR("Invalid STRCON");
		}
	}
	i=1;
	if(p->wds[p->cw][0]=='\"') {
		while(p->wds[p->cw][i]!='\"'&&p->wds[p->cw][i]!='\0') {
			i++;
		}

		if(p->wds[p->cw][i]=='\0') {
			ERROR("Invalid STRCON");
		}
		i++;
		if(p->wds[p->cw][i]!='\0') {
			ERROR("Invalid STRCON");
		}
	}
	return TRUE;
}

int NUMCON(Program* p) {
	int i=1;
	int flag=0;
	if(!isdigit(p->wds[p->cw][0])) {
		return FALSE;
	}
	while(isdigit(p->wds[p->cw][i])||p->wds[p->cw][i]=='.') {
		if(p->wds[p->cw][i]=='.')
			flag++;
		i++;
	}
	if(p->wds[p->cw][i]!='\0'||flag>1) {
		ERROR("Invalid NUMCON!");
	}
	return TRUE;
}

int JUMP(Program* p) {
	int num=0;
	if(!strsame(p->wds[p->cw],"JUMP")) {
		return FALSE;
	}
	p->cw++;
	if(p->cw>=p->aw) {
		ERROR("Lack jump number!");
	}
	num=stringToNumber(p->wds[p->cw]);
	if(num>=p->aw) {
		ERROR("Invalid jump number!");
	}
	p->cw++;
	/*******/
	p->cw=num;
	return TRUE;
}

int stringToNumber(char* str) {
	int i=0;
	int num=0;
	for(i=0; str[i]!='\0'; i++) {

		if(str[i]>='0'&&str[i]<='9') {
			num=num*10+str[i]-'0';
		} else {
			ERROR("Invalid jump number!");
		}
	}
	return num;
}

int RND(Program* p) {
	int t=0;
	int keyNum=0;
	Var* v;
	if(!strsame(p->wds[p->cw],"RND")) {
		return FALSE;
	}
	p->cw++;
	if(p->cw<p->aw&&p->wds[p->cw][0]=='(') {
		p->cw++;
	} else {
		ERROR("Lack '('!");
	}
	if(p->cw<p->aw&&NUMVAR(p)) {
		keyNum=p->cw;
		p->cw++;
	} else {
		ERROR("Invalid rnd variable!");
	}
	if(p->cw<p->aw&&p->wds[p->cw][0]==')') {
		p->cw++;
	} else {
		ERROR("Lack ')'!");
	}
	/********/
	t=rand()%100;
	v=initial_Var();
	v->key=p->wds[keyNum];
	v->value=numToString(t);
	insert(v,p);
	return TRUE;
}

char* numToString(int num) {
	char* s=(char*)malloc(3*sizeof(char));
	if(num/10>0) {
		s[0]=(num/10)+'0';
		s[1]=(num%10)+'0';
		s[2]='\0';
	} else {
		s[0]=(num%10)+'0';
		s[1]='\0';
		s[2]='\0';
	}
	return s;
}


int SET(Program* p) {
	Var* v=NULL;
	int keyNum,valNum;
	if(p->wds[p->cw][0]=='$') {
		STRVAR(p);
		keyNum=p->cw;
		p->cw++;
		if(p->cw<p->aw&&p->wds[p->cw][0]=='='&&p->wds[p->cw][1]=='\0') {
			p->cw++;
		} else {
			ERROR("Invalid set!");
		}
		valNum=p->cw;
		if(p->cw<p->aw&&(STRVAR(p)||STRCON(p))) {
			/*******/
			setVariable(p,keyNum,valNum);
			/*******/
			p->cw++;
		} else {
			ERROR("Invalid set!");
		}

	} else if(p->wds[p->cw][0]=='%') {
		NUMVAR(p);
		keyNum=p->cw;
		p->cw++;
		if(p->cw<p->aw&&p->wds[p->cw][0]=='='&&p->wds[p->cw][1]=='\0') {
			p->cw++;
		} else {
			ERROR("Invalid set!");
		}
		valNum=p->cw;
		if(p->cw<p->aw&&(NUMVAR(p)||NUMCON(p))) {
			/*******/
			setVariable(p,keyNum,valNum);
			/*******/
			p->cw++;
		} else {
			ERROR("Invalid set!");
		}
	} else {
		return FALSE;
	}
	return TRUE;
}
void setVariable(Program* p,int keyNum,int valNum) {
	Var* v=p->head;
	while(v!=NULL&&!(strsame(v->key,p->wds[keyNum]))) {
		v=v->next;
	}
	if(v==NULL) {
		v=initial_Var();
		v->key=p->wds[keyNum];
		v->value=p->wds[valNum];
		insert(v,p);
	} else {
		free(v->value);
		v->value=p->wds[valNum];
	}
	return;
}
int nFILE(Program* p) {
	char buf[256]={0};
	int i=0;
	Program* prog;
	if(!strsame(p->wds[p->cw],"FILE")) {
		return FALSE;
	}
	p->cw++;
	if(p->cw<p->aw&&STRCON(p)) {
	 for(i=1;p->wds[p->cw][i]!='\"';i++)
	 {
	 	buf[i-1]=p->wds[p->cw][i];
	 }
	 prog=read_file(buf);
	 Prog(prog);
	 p->cw++;
	} else {
		ERROR("Invalid File!");
	}
	return TRUE;
}

int ABORT(Program* p) {
	if(!strsame(p->wds[p->cw],"ABORT")) {
		return FALSE;
	}
	/****/
	exit(2);
	/*******/
	return TRUE;
}

int INPUT(Program* p) {
	int keyNum=0;
	int keyNum2=0;
	if(strsame(p->wds[p->cw],"IN2STR")) {
		p->cw++;
		if(p->cw<p->aw&&p->wds[p->cw][0]=='(') {
			p->cw++;
		} else {
			ERROR("Lack '('!");
		}
		if(p->cw<p->aw&&(STRVAR(p))) {
			keyNum=p->cw;
			p->cw++;
		} else {
			ERROR("Invalid STRVAR!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]==',') {
			p->cw++;
		} else {
			ERROR("Lack ','!");
		}
		if(p->cw<p->aw&&(STRVAR(p))) {
			keyNum2=p->cw;
			p->cw++;
		} else {
			ERROR("Invalid STRVAR!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]==')') {
			p->cw++;
		} else {
			ERROR("Lack ')'!");
		}
		/***********************/;
		printf("Please input the first str,like '\"'ABC'\" or #ABC# \n");
		inputVariable(p,keyNum);
		printf("Please input the second str,like '\"'ABC'\"' or #ABC# \n");
		inputVariable(p,keyNum2);
		return TRUE;
	} else if(strsame(p->wds[p->cw],"INNUM")) {
		p->cw++;
		if(p->cw<p->aw&&p->wds[p->cw][0]=='(') {
			p->cw++;
		} else {
			ERROR("Lack '('!");
		}
		if(p->cw<p->aw&&(NUMVAR(p))) {
			keyNum=p->cw;
			p->cw++;
		} else {
			ERROR("Invalid NUMVAR!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]==')') {
			p->cw++;
		} else {
			ERROR("Lack ')'!");
		}
		/*****************/
		printf("Please input a num like(17.4,5,9,100.1 etc)\n");
		inputVariable(p,keyNum);
		return TRUE;
	} else
		return FALSE;
}

void inputVariable(Program* p,int keyNum) {
	/***********************/;
	Var* v=p->head;
	char buf[256]= {0};
	char* s=NULL;
	scanf("%s",buf);
	s=(char*)malloc((strlen(buf)+1)*sizeof(char));
	strcpy(s,buf);
	while(v!=NULL&&!(strsame(v->key,p->wds[keyNum]))) {
		v=v->next;
	}
	if(v==NULL) {
		v=initial_Var();
		v->key=p->wds[keyNum];
		v->value=s;
		insert(v,p);
	} else {
		free(v->value);
		v->value=s;
	}
	return ;
}

int INC(Program* p) {
	int keyNum=0;
	Var* v=p->head;
	if(!strsame(p->wds[p->cw],"INC")) {
		return FALSE;
	}
	p->cw++;
	if(p->cw<p->aw&&p->wds[p->cw][0]=='(') {
		p->cw++;
	} else {
		ERROR("Lack '('!");
	}
	if(p->cw<p->aw&&(NUMVAR(p))) {
		keyNum=p->cw;
		p->cw++;
	} else {
		ERROR("Invalid NUMVAR!");
	}
	if(p->cw<p->aw&&p->wds[p->cw][0]==')') {
		p->cw++;
	} else {
		ERROR("Lack ')'!");
	}
	/**********/
	searchNumInc(p,p->wds[keyNum]);
	return TRUE;
}

void searchNumInc(Program* p,char* s) {
	Var* v=p->head;
	while(v!=NULL&&!strsame(s,v->key)) {
		v=v->next;
	}
	if(v==NULL) {
		ERROR("Undefined var");
	}
	if(v->value[0]=='%') {
		searchNumInc(p,v->value);
	} else {
		numInc(v->value);
	}
	return;
}

void numInc(char* s) {
	int flag1=0,flag2=0,num=0,k=0,len=0,i=0;
	char buf[256]= {0};
	for(i=0; s[i]!='\0'; i++) {
		if(s[i]=='.') {
			flag1=1;
			k=i;
		} else if(s[i]!='9') {
			flag2=1;
		}
	}
	len=strlen(s);
	if(flag1==0) {
		if(s[len-1]!='9') {
			s[len-1]=s[len-1]+1;
		} else if(flag2==1) {
			num=stringToNumber(s);
			num++;
			for(i=len-1; i>=0; i--) {
				s[i]=(num%10)+'0';
				num=(num/10);
			}
		} else {
			num=stringToNumber(s);
			num++;
			s=(char*)realloc(s,sizeof(char)*(len+2));
			s[len+1]='\0';
			for(i=len; i>=0; i--) {
				s[i]=(num%10)+'0';
				num=(num/10);
			}
		}
	} else if(flag1==1) {
		if(s[k-1]!='9') {
			s[k-1]=s[k-1]+1;
		} else if(flag2==1) {
			for(i=0; i<k; i++)
				buf[i]=s[i];
			num=stringToNumber(buf);
			num++;
			for(i=k-1; i>=0; i--) {
				s[i]=num%10+'0';
				num=num/10;
			}
		} else {
			for(i=0; i<k; i++)
				buf[i]=s[i];
			num=stringToNumber(buf);
			num++;
			s=(char*)realloc(s,sizeof(char)*(len+2));
			for(i=k; i>=0; i--) {
				s[i]=num%10+'0';
				num=num/10;
			}
			s[len+1]='\0';
			for(i=len; i>=k+2; i--) {
				s[i]=s[i-1];
			}
			s[k+1]='.';
		}
	}
}


int IFCOND(Program *p) {
	int keyNum1=0,keyNum2=0,condNum=0;
	condNum=p->cw;
	if(strsame(p->wds[p->cw],"IFEQUAL")||strsame(p->wds[p->cw],"IFGREATER")) {
		p->cw++;
		if(p->cw<p->aw&&p->wds[p->cw][0]=='(') {
			p->cw++;
		} else {
			ERROR("Lack '('!");
		}
		if(p->cw<p->aw&&(VARCON(p))) {
			keyNum1=p->cw;
			p->cw++;
		} else {
			ERROR("Invalid VARCON!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]==',') {
			p->cw++;
		} else {
			ERROR("Lack ','!");
		}
		if(p->cw<p->aw&&(VARCON(p))) {
			keyNum2=p->cw;
			p->cw++;
		} else {
			ERROR("Invalid VARCON!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]==')') {
			p->cw++;
		} else {
			ERROR("Lack ')'!");
		}
		if(p->cw<p->aw&&p->wds[p->cw][0]=='{') {
			p->cw++;
			if(judgeCond(p,keyNum1,keyNum2,condNum)==TRUE) {
				INSTRS(p);
			} else {
				while(p->cw<=p->aw&&!strsame(p->wds[p->cw],"}"))
					p->cw++;
			}
			if(p->cw>p->aw||!strsame(p->wds[p->cw],"}")) {
				ERROR("No } at the endding?");
			}
			p->cw++;
		}
		return TRUE;
	} else return FALSE;

}

int judgeCond(Program* p,int keyNum1,int keyNum2,int condNum) {
	char *s1=NULL,*s2=NULL;
	s1=searchVariable(p,p->wds[keyNum1]);
	s2=searchVariable(p,p->wds[keyNum2]);
	if(strsame(p->wds[condNum],"IFEQUAL")) {
		return strsame(s1,s2)==TRUE?TRUE:FALSE;
	} else if(strsame(p->wds[condNum],"IFGREATER")) {
	    if(strlen(s1)>strlen(s2))
		{
			return TRUE;
		}
		else if(strlen(s1)<strlen(s2)) 
		{
			return FALSE;
		}
		else
	    {
	    	return strcmp(s1,s2)>0?TRUE:FALSE; 
		}
	}
	return FALSE;
}


char * searchVariable(Program* p,char* s) {
	Var* v=p->head;
	char* res=NULL;
	int i=1;
	if(s[0]=='#')
	{
	   res=(char*)malloc((strlen(s)+1)*sizeof(char));
	   res[0]='\"';
	   for(i=1;s[i]!='#';i++)
	   {
	   	res[i]=hash(s[i]);
	   }
	   res[i++]='\"';
	   res[i]='\0';
	   return res;
	} 
	if(s[0]!='%'&&s[0]!='$') {
		return s;
	}
	while(v!=NULL&&!strsame(s,v->key)) {
		v=v->next;
	}
	if(v==NULL) {
		ERROR("Undefined var");
	}
	res=searchVariable(p,v->value);
	return res;
}


void insert(Var* v,Program *p) {
	v->next=p->head;
	p->head=v;
}
