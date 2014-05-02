#pragma once
#include "node.h"
class ForNode :
	public Node
{
protected:
	Node* _initlizer;
	Node* _condition;
	Node* _increment;
	Node* _statment;


public:
	static	int 	for_label;
	ForNode(Node* initlizer,Node* condition,Node* increment,Node* statment,ScoopNode* scoop):
		Node(scoop),_initlizer(initlizer),_condition(condition),_increment(increment),_statment(statment)
	{

	}
	

	bool type_check()
	{

		Type* boolType=symbolTable->getType("bool");
		if (_condition->get_type()==boolType)
		{
			return true;
		}else
		{
			///////////////////////////////////////////////////
			/////////// error 
			///////////////////////////////////////////////

		}


		if (_increment->type_check()==true)
		{
			return	true;
		}else
		{
			///////////////////////////////////////////////////
			/////////// error 
			///////////////////////////////////////////////

		}
		return false;
	}


	void setStatement(Node* statement){
		if(statement!=NULL)
			this->_statment=statement;
	}
	virtual void generate_code (){
		
		string cc  = "";
		cc=std::to_string(ForNode::for_label++);

		string ccc = "For";
		ccc+=cc;

		string ccc2 = "endFor";
		ccc2+=cc;

		
		

		MIPS_ASM::printComment("Begin Initializer\n");
		if(_initlizer!=NULL)
			_initlizer->generate_code();
		MIPS_ASM::printComment("End Initializer\n");

		MIPS_ASM::label(ccc);

		MIPS_ASM::printComment("Begin Condition\n");
		if(_condition!=NULL)
			_condition->generate_code();
			MIPS_ASM::printComment("End Condition\n");

		MIPS_ASM::pop("t0");
		MIPS_ASM::beq("t0","0",ccc2);

		MIPS_ASM::printComment("Begin Statement\n");
		if(_statment!=NULL)
			_statment->generate_code();
			MIPS_ASM::printComment("End Statement\n");
				MIPS_ASM::printComment("Begin Increment\n");
		if(_increment!=NULL)
			_increment->generate_code();
			MIPS_ASM::printComment("End Increment\n");
		MIPS_ASM::jump(ccc);
		MIPS_ASM::label(ccc2);

	}
	void toString(){
		cout<<"for Noooode\n";
		if(_initlizer!=NULL)
			_initlizer->toString();
		if(_condition!=NULL)
			_condition->toString();
		if(_statment!=NULL)
		_statment->toString();
	}
	virtual ~ForNode(void)
	{
	}
};

__declspec(selectany) int ForNode::for_label;

