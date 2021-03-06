#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H
#include <fstream>
#include <iostream>
#include <cstring>

#include <fstream>
#include <sstream>
using namespace std;

extern int lineNum;
extern int colNum;
extern string sourceFile;
class MIPS_ASM
{
private: 
		  
public:
	static int strings_count ;
	 static char alltext2[20000];
	// static char data[1000];
	 static int MIPS_ASM::lines;
	 static int MIPS_ASM::cols;

	static int MIPS_ASM::static_size;
	static char current_chars[1000];
	static int MIPS_ASM::o;
	static char* alltext;
	static stringstream MIPS_ASM::code;
	static stringstream MIPS_ASM::data;
	static stringstream MIPS_ASM::main;

static	FILE * file;
static string getStringAdressLabel(string str);

static void writeData();
static void writeCode();
static void writeMain();

static void add_data(string);
static void addMainInstruction(string);

	static void add_instruction(string c);
	static char* add_data_char(char c);
	static char* add_data_char(char *c);
	

	static void operation(string reg1,string reg2,string reg3,int flag);
	static void operationf(string reg1,string reg2,string reg3,int flag);
	static void slt(string reg1,string reg2,string reg3);

	static void li(string reg,int v);
	static void la(string reg,string);
	static void lif(string reg,float v);

	static void reserveStack(int size);
	static void releaseStack(int size);

	static void push(string source);
	static void pushf(string source);

	static void pop(string dest);
	static void top(string dest);
	static void pop();
	static void popf(string dest);

	static void beq(string reg1,string reg2,string label);
	static void bne(string reg1,string reg2,string label);
	static void label(string);

	static void jump(string);

	static void jr(string ra="ra");

	static void jal(string);

	static void lw_fp(string,int);
	static void sw_fp(string,int);

	static void lw_fpf(string,int);
	static void sw_fpf(string,int);

	static void lw_gp(string,int);
	static void sw_gp(string,int);

	static void lw(string dest_reg,int offset,string mem_adress);
	static void sw(string source_reg,int offset,string mem_adress);

	static void move(string dest_reg,string from_reg);

	static void printComment(string comment);

};

#endif