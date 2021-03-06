#include"SymbolTable.h"
#include"Symbol.h"
#include "../Streams.h"
#include "../CallNode.h"
#include "../ast/ConstantNode.h"
#include "../AsmNode.h"
#include "../ast/AssignNode.h"
#include "../TryNode.h"
#include "../ast/FunctionNode.h"
#include "../ast/IdentifierNode.h"
#include <set>
#include "../Program.h";
#include "../SemanticError.h";
#include "../RegAccessNode.h"
#include "../ast/CastNode.h"

extern std::ofstream ofs;
extern ScoopNode* globalScoop;
extern Method * mainMethod;
extern int lineNum;
extern int colNum;
extern string sourceFile;
extern int Iskernal = 0;
extern bool Garbage_Collect=0;
int  TryNode::count = 0;
int CastNode::count=0;
extern bool Optimize=0;

int CatchNode::lbl_count = 0;

SymbolTable::SymbolTable(void)
{
	this->add_type(new Type("int"));
	this->add_type(new Type("bool"));
	this->add_type(new Type("NSString"));
	this->add_type(new Type("float"));
	this->add_type(new Type("void"));
	this->add_type(new Type("char"));
	this->add_type(new Type("id"));
	this->add_type(new Type("null"));
	this->add_type(new Type("error_type"));

}
SymbolTable::~SymbolTable(void)
{


}
//void SymbolTable::addSymbol(Type * S)
//	{
//		if (S!=NULL){
//			this->symbolTable.insert(hash_map<const string, Type*>::value_type(S->get_name(), S));
//		}
//	}

//
//Type* SymbolTable::getSymbol(string S){
//	
//	hash_map< string, Type*>::iterator it = this->symbolTable.find(S) ;
//	if (it != this->symbolTable.end()){
//			return it->second;
//		}else{
//			return NULL;
//		}
//
//}

void SymbolTable::toString(){

	for(TypesMap::iterator i = types.begin(); i != types.end(); i++)
	{
		i->second->toString(); 
	}
}
void SymbolTable ::add_type(Type* type){
	if ((type!=NULL)&&(this!=NULL) ){
		//Method* temp = this->getMethod(S->get_name(), S->getReturnType(),S->parameters);
		if (types.find(type->get_name()) != types.end())
		{
			Streams::WTF() << "duplicated addd\n\n";
			throw new exception("deal breaking error,WTF duplicated add");
		}
		types[type->get_name()]=type;

	}
}


Type* SymbolTable::getType(string name)
{

	if((this!=NULL)){
		if (types[name]!=NULL)
		return types[name]; // even if it's not implemented yet it should return object with the default constructer expecting to be defined later (multiparse)
		else
		{
			Streams::verbose() << "Making new interface: "<<name<<"\n\n\n\n";
			Interface* interface = new Interface(name);
			add_interfaceState(name, false);
			return types[name] = interface;
		}
	}
}
bool SymbolTable::checkTypeProtocol(string name)
{
	if ((this != NULL))
	{
		if (types.find(name)==types.end())
			return false;
		else
			return true;

	}

}
bool SymbolTable::checkTypeExist(string name)
{
	Type *type = getType(name);
	if (type != NULL)
		return true;
	return false;
}
void SymbolTable::generateStatics()
{
	//MIPS_ASM::add_data("top_catcher:    .byte   0:4");
	globalScoop->add_variable(new Variable("top_catcher", getType("NSExceptionCatcher"), 0));

	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			 globalScoop->add_variable(new Variable(ifs->get_name(),ifs->static_twin,1));
			 globalScoop->addNode(new AsmNode(globalScoop, "la $t0," + ifs->getStaticPointerStr() + "\n"));
			 globalScoop->addNode(new AsmNode(globalScoop, "li $t1," + std::to_string(ifs->static_twin->getId()) + "\n"));

			 Variable* var = globalScoop->get_variable(ifs->get_name());
			 globalScoop->addNode(new AsmNode(globalScoop, string("sw $t0,") + std::to_string(-var->getOffset()) + "($"
				 + var->getOffsetRegister() + ")\n"
				 ));
			 globalScoop->addNode(new AsmNode(globalScoop, string("sw $t1,") + std::to_string(0) + "($"
				 + "t0"+ ")\n"
				 ));

			 ifs->static_twin->setStatus(completness::implemented);

			 MIPS_ASM::add_data(ifs->getStaticPointerStr() + ":    .byte   0:" + std::to_string(ifs->static_twin->getObjectSize()) + "\n");
			 Method* method = new Method("alloc", ifs);
			 auto fs = ScoopHelper::createNewFunctionNode(method, ifs->static_twin);
			 fs->_has_return = true;
			 fs->addNode(new AsmNode(fs, "move $s0,$ra"));
			 fs->addNode(new AsmNode(fs, string("li $a0,") + std::to_string(ifs->getObjectSize())));
			 fs->addNode(new AsmNode(fs, "jal malloc"));
			 fs->addNode(new AsmNode(fs, "move $ra,$s0"));
			 fs->addNode(new AsmNode(fs, "li $t0," + std::to_string(ifs->getId())));
			 fs->addNode(new AsmNode(fs, "sw $t0,0($v0)"));
			 for (auto vm : ifs->getScoop()->_variables){
				 auto vt = dynamic_cast<Interface*> (vm.second->getType());
				 if (vt){
					 fs->addNode(new AsmNode(fs, "sw $0," + std::to_string(vm.second->getOffset()) + "($v0)"));

				 }
			 }
			 ifs->static_twin->getMethodsItem()->addMethod(method);
			 method = new Method("", getType("bool"));
			 method->addSelector(new DeclerationSelector("isA", { new Variable("obj", getType("NSObject")) }));
			 fs = ScoopHelper::createNewFunctionNode(method, ifs->static_twin);
			 fs->_has_return = true;

			 fs->addNode(new AsmNode(fs, "sub $sp,$sp,8"));
			 fs->addNode(new AsmNode(fs, "sw $ra,0($sp)"));
			 fs->addNode(new AsmNode(fs, "sw $fp,4($sp)"));
			 fs->addNode(new IdentifierNode("obj",fs));

			 fs->addNode(new AsmNode(fs, "lw $t0,-4($sp)"));
			 fs->addNode(new AsmNode(fs, "lw $a0,0($t0)"));
			 fs->addNode(new AsmNode(fs, "jal " + ifs->getIsALabel()));
			 fs->addNode(new AsmNode(fs, "lw $ra,0($sp)"));
			 fs->addNode(new AsmNode(fs, "lw $fp,4($sp)"));
			 fs->addNode(new AsmNode(fs, "add $sp,$sp,8"));

			 ifs->static_twin->getMethodsItem()->addMethod(method);

		}

	}

	
		
}

void SymbolTable::generateKernalCode(){
	CallNode* cn = new CallNode(globalScoop, new IdentifierNode("top_catcher", globalScoop), "");
	//+(NSExceptionCatcher*) new:(NSObject*)e:(NSExceptionCatcher*) p:(int)s :(int) f :(int)l{

	auto cs = new CallSelector("catch");
	cs->addArg(new RegAccessNode(globalScoop, "k0",getType("NSException")));

	cn->addSelector(cs);
	cn->generateCode();
}
void SymbolTable::generateStaticsCode()
{
	if (mainMethod)
	{
		
		{
			CallNode* cn = new CallNode(globalScoop, new IdentifierNode("NSExceptionCatcher", globalScoop), "");
		//+(NSExceptionCatcher*) new:(NSObject*)e:(NSExceptionCatcher*) p:(int)s :(int) f :(int)l{
		
			auto cs = new CallSelector("new");
			cs->addArg(new IdentifierNode("NSException", globalScoop));
			cs->addArg(new IdentifierNode("top_catcher", globalScoop));
			cs->addArg(new SPNode(globalScoop));
			cs->addArg(new FPNode(globalScoop));
			cs->addArg(new LabelValNode(globalScoop, "global_catch"));
			cs->addArg(new RegAccessNode(globalScoop, "ra", symbolTable->getType("int")));

			cn->addSelector(cs);
			//	cn->generateCode();
			AssignNode* asn = dynamic_cast<AssignNode*>
				((new AssignNode(globalScoop, new IdentifierNode("top_catcher", globalScoop), cn)));
			globalScoop->addNode(asn);
		}
		Variable* var = globalScoop->get_variable(mainMethod->getInterface()->get_name());
		globalScoop->addNode(new IdentifierNode(mainMethod->getInterface()->get_name(), globalScoop));
		globalScoop->addNode(new AsmNode(globalScoop, "sub $sp,$sp,4"));
		globalScoop->addNode(new AsmNode(globalScoop, string("jal ") + mainMethod->getLabel()));
		globalScoop->addNode(new AsmNode(globalScoop, "li $v0,10"));
		globalScoop->addNode(new AsmNode(globalScoop, "syscall"));

	}
	else{
		//ToDo error
		//hasson
		string error = "Error Main not Found";
		Program::addError(new SemanticError(error, colNum, lineNum, sourceFile));

		
	}

	globalScoop->generateCode();


	MIPS_ASM::add_instruction("\n\n\n\n");
	MIPS_ASM::printComment(string("Global vtable: "));
	MIPS_ASM::add_instruction("\n\n");
	MIPS_ASM::label("global_vtable");

	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			MIPS_ASM::printComment(ifs->get_name());
			MIPS_ASM::add_instruction("beq $a0," + std::to_string(ifs->getId()) + "," + ifs->getVtableLabel() + "\n");
			ifs = ifs->static_twin;
			MIPS_ASM::printComment(ifs->get_name() + " static");

			MIPS_ASM::add_instruction("beq $a0," + std::to_string(ifs->getId()) + "," + ifs->getVtableLabel() + "\n");

		}
	}
	MIPS_ASM::jump("type_not_found");

	MIPS_ASM::add_instruction("\n\n\n\n");
	MIPS_ASM::printComment(string("Global dispose table: "));
	MIPS_ASM::add_instruction("\n\n");
	MIPS_ASM::label("global_dispose"); 

		MIPS_ASM::add_instruction("beq $a0,$0,global_dispose_end\n");
	MIPS_ASM::add_instruction("blt $a0, 0x10040000,global_dispose_end\n");
	MIPS_ASM::lw("t0", -4, "a0");
	MIPS_ASM::add_instruction("bgt $t0,$0,global_dispose_end\n");

	MIPS_ASM::lw("t0",0, "a0");

	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			MIPS_ASM::printComment(ifs->get_name());
			MIPS_ASM::add_instruction("beq $t0," + std::to_string(ifs->getId()) + ",dispose_" + std::to_string(ifs->getId()) + "\n");

		}
	}
	MIPS_ASM::label("global_dispose_end");

	MIPS_ASM::jr();
	{
		MIPS_ASM::label("nullpointer_exp");
		CallNode* cn = new CallNode(globalScoop, new IdentifierNode("NSException", globalScoop), "");
		//+(NSExceptionCatcher*) new:(NSObject*)e:(NSExceptionCatcher*) p:(int)s :(int) f :(int)l{

		auto cs = new CallSelector("withMsg");
		cs->addArg(new ConstantNode(string("Null pointer exception"), globalScoop));
		cn->addSelector(cs);
		cn->generateCode();
		MIPS_ASM::li("s7", 12);// turkey number for exception
		MIPS_ASM::add_instruction("teq $t0,$t0\n");
	}
	{
		MIPS_ASM::label("arrayout_exp");
		CallNode* cn = new CallNode(globalScoop, new IdentifierNode("NSException", globalScoop), "");
		//+(NSExceptionCatcher*) new:(NSObject*)e:(NSExceptionCatcher*) p:(int)s :(int) f :(int)l{

		auto cs = new CallSelector("withMsg");
		cs->addArg(new ConstantNode(string("array out of boundry exception"), globalScoop));
		cn->addSelector(cs);
		cn->generateCode();
		MIPS_ASM::li("s7", 12);// turkey number for exception
		MIPS_ASM::add_instruction("teq $t0,$t0\n");
	}
	{
		MIPS_ASM::label("global_catch");
		MIPS_ASM::add_instruction("addi $a0,$s0,"+
			std::to_string(dynamic_cast<Interface*>(getType("NSException"))
			->getScoop()->get_variable("msg")->getOffset())+"\n");

		MIPS_ASM::add_instruction("li $v0,4\n");
		MIPS_ASM::add_instruction("syscall\n");
		MIPS_ASM::add_instruction("li $v0,10\n");
		MIPS_ASM::add_instruction("syscall\n");

	}

}
bool SymbolTable::checkInhertanceLoop()
{
	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			set<Interface*> s;
			s.insert(ifs);
			while (ifs)
			{
				ifs = ifs->getInheretInterface();
				if (s.count(ifs))
					return false;
				s.insert(ifs);
			}
		
		}
	}
	return true;
}
void SymbolTable::generateCode()
{
	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			ifs->preprocess();
		}
	}
	generateStaticsCode();
	for (auto i = this->types.begin(); i != this->types.end(); i++)
	{
		auto ifs = dynamic_cast<Interface*> (i->second);
		if (ifs){
			if (ifs->status != completness::implemented){
				Program::addError(new SemanticError("Class "+ifs->get_name()+" referenced but never implemented",0,0,""));
			}
		}
		i->second->generateCode();
	}
}
bool SymbolTable::getInterfaceState(string name)
{
	return declaredInterfaces[name];
}
void SymbolTable::add_interfaceState(string name, bool state)
{
	declaredInterfaces[name] = state;
}
void SymbolTable::set_interfaceState(string name, bool state)
{
	declaredInterfaces[name] = state;
}


//void SymbolTable::add(symbol* newItem)
//{
//	int index = GetIndex(newItem->get_name());     //////////////   we should use a getter
//	if (hashST[index] == NULL)
//	{
//		hashST[index] = newItem;
//	}
//	else
//	{
//		symbol* temp;
//		temp = hashST[index];
//		hashST[index] = newItem;
//	}
//	Size++;
//}
//int SymbolTable::GetIndex (string str)
//{
//	int h = 0;
//	for (int i=0; i<str.length(); i++)
//		h = h+((int) str[i]);
//
//	return (h % 10);
//}








