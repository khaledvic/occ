#include "Symbol.h"
#include "Method.h"
#include "Interface.h"
#include<string.h>
#include "../AsmNode.h"
#include "../ClassNode.h"
#include "../ast/FunctionNode.h"
using namespace std;
int Interface::interfacesCount=0;

Interface::Interface(string name, bool is_static_twin) :Type(name)
{
	varItems=new VariableItems();
	methodsItem=new MethodItems();
	classNode = new ClassNode(globalScoop, this);
	children_ids.insert(getId());
	if (!is_static_twin){
		static_twin = new Interface(name, true);
		if (name!="NSObject")
		static_twin->setInheritInterface(dynamic_cast<Interface*>( symbolTable->getType("NSObject")));
	}
	

}
/*
Interface::Interface(Interface* interf):Type(interf->get_name())
{
	//this->setInheritInterface(interf->inherit_interface);
	this->methodsItem=interf->methodsItem;
	this->varItems=new VariableItems(interf->varItems);
	this->son_protocols=interf->son_protocols;
	this->inherit_interface=interf->getInheretInterface();
	methodsItem=new MethodItems(interf->methodsItem);
}*/
Interface* Interface::getInheretInterface(){
	return this->inherit_interface;
}
int Interface::getDataMembersSize(){
	if (getInheretInterface() != NULL){
		return classNode->getFrameSize() + getInheretInterface()->getDataMembersSize();
	}
	return classNode->getFrameSize();
}
int Interface::getObjectSize(){
	return  getObjectIdentifiersSize()+getDataMembersSize();
}
int Interface::getObjectIdentifiersSize(){
	return 4;
}
bool Interface::isDescendentOf(Interface* i){
	if (this == i)
		return true;
	return getInheretInterface() ? getInheretInterface()->isDescendentOf(i) : false;
}

string Interface::getStaticPointerStr(){
	return string("static_instance_")+std::to_string(getId())+"_"+get_name();
}

	VariableItems* Interface:: getVariableItems(){
		
		return varItems;
		
	}
	void Interface::setVariableItems(VariableItems* varItems){
		this->varItems=varItems;
	}
		MethodItems* Interface::getMethodsItem(){
			return methodsItem;
		}
	void Interface::setMethodsItem(MethodItems* methodsItems){
		this->methodsItem=methodsItem;
	}
		void Interface:: toString(){
			Type::toString();
			varItems->toString();
			methodsItem->toString();
			
			if(inherit_interface!=NULL)
			{
				cout<<"\n Inherits From: ";
				inherit_interface->toString();
				cout<<endl<<endl;
			}
			if(son_protocols.size()>0){
				cout<<"Implements: \n";
				for(hash_map<const string, Protocol*>::iterator it = this->son_protocols.begin(); it!=this->son_protocols.end(); it++ ){
					it->second->toString();
			}
			}	cout<<"end interface  "<<get_name() <<" -----------------------------------------------------\n";
		}

	void Interface ::setInheritInterface(Interface* interf)
	{
		interf->addChild(children_ids);
		this->inherit_interface=interf;
	//	this->static_twin->inherit_interface = interf->static_twin;
		classNode->_scoop = interf->classNode;
	}
	
//void Interface::add_Method(Method* S)
//	{
//		if (S!=NULL){
//			this->methods.insert(multimap<const string, Method*>::value_type(S->get_name(), S));
//		}
//	}
void Interface::add_son_protocol(Protocol * S)
	{
		if (S!=NULL){
			this->son_protocols.insert(hash_map<const string, Protocol*>::value_type(S->get_name(), S));
		}
	}
//Method* Interface::Get_Method(string name,Type* type ,vector<Variable*> parameter)
//	{
//		multimap<const string, Method*>::iterator it = this->methods.find(name);
//		Method* tempMethod=new Method(name,type);
//		tempMethod->set_variables(parameter);
//	while (it != this->methods.end()){
//			if (Method::compare(tempMethod,(Method*)it->second)){
//			return (Method*)it->second;
//		}else{
//			it++;
//		}
//	}
//	if (it==this->methods.end()){
//		return NULL;
//	}
//}

Protocol* Interface::get_son_protocol(string S){
	
	hash_map< string, Protocol*>::iterator it = this->son_protocols.find(S) ;
	if (it != this->son_protocols.end()){
			return it->second;
		}else{
			return NULL;
		}

}


string Interface::getTypeName(){
	return "Interface";
}

Method* Interface::getMethodByName(string name, Type* type, vector<DeclerationSelector*> v, bool isStatic)
{
	return methodsItem->getMethod(name, type, v, isStatic);
}
Method* Interface::getMethodByNameWithInhertance(string name, Type* type, vector<DeclerationSelector*> v, bool isStatic)
{
	Interface* vinter = this;
	while (vinter != NULL)
	{

		Method* m = vinter->methodsItem->getMethod(name, type, v, isStatic);
		;
		if (m)return m;
		vinter = vinter->getInheretInterface();
	}
	return NULL;
	
}
Variable* Interface::getVariableByName(string name)
{
	return varItems->get_variable(name);
}

string Interface::getVtableLabel(){
	return string("vt_") + std::to_string(getId());
}
string Interface::getIsALabel(){
	return string("isa_") + std::to_string(getId());
}
string Interface::getVtableString(){
	string res;
	for (auto i = this->methodsItem->methods.begin(); i != this->methodsItem->methods.end(); i++)
	{
		string ins1 = "li $t0,";
		ins1 += std::to_string(i->second->getId());
		string ins2 = "beq $t0,$a1, ";
		ins2 += i->second->getLabel();
		res += ins1 + "\n" + ins2 + "\n";
	}
	return res;
}

Method* Interface::getMethodOverloaded (string message, vector<CallSelector*> selectors)
{
	Interface* vinter = this;
	while (vinter != NULL)
	{

		Method* m = vinter->methodsItem->getOverloadedMethod(message, selectors);
		if (m)return m;
		vinter = vinter->getInheretInterface();
			}
	return NULL;
}
void Interface::preprocess(){
	for (auto i = this->methodsItem->methods.begin(); i != this->methodsItem->methods.end(); i++)
	{
		if (getInheretInterface()){
			auto tm = getInheretInterface()->getMethodByName(i->second->get_name(), i->second->getReturnType(), i->second->get_variables(), i->second->get_static());
			if (tm != NULL)
			{
				
				//todo not here
				tm->hasBeenInhereted = true;
			}
		}

	}
}

	void Interface::generateCode(){
		MIPS_ASM::printComment("#########################################");
	MIPS_ASM::printComment(string("Generating code for class ") + this->get_name());
	MIPS_ASM::add_instruction("\n\n\n\n");
	MIPS_ASM::printComment(string("vtable: "));
	MIPS_ASM::add_instruction("\n\n");
	MIPS_ASM::label(getVtableLabel());
	//MIPS_ASM::add_instruction(getVtableString());
	Interface* vinter = this;
	while (vinter != NULL)
	{
		for (auto i = vinter->methodsItem->methods.begin(); i != vinter->methodsItem->methods.end(); i++)
		{
			if (i->second->getFunctionNode() != NULL){

				MIPS_ASM::printComment(i->first + i->second->to_string());
				MIPS_ASM::add_instruction(string("li $t0,") + std::to_string(i->second->getId()) + "\n");
				MIPS_ASM::add_instruction(string("beq $t0,$a1,") + i->second->getLabel() + "\n");
			}
		}

		vinter = vinter->getInheretInterface();
		MIPS_ASM::printComment("super");
		if (vinter != NULL)
			MIPS_ASM::printComment(vinter->get_name());

	}


	MIPS_ASM::jump("method_not_found");

	MIPS_ASM::label("dispose_" + to_string(getId()));
	MIPS_ASM::push("ra");
	MIPS_ASM::push("a0");
	MIPS_ASM::jal("free");
	MIPS_ASM::add_instruction("bne $v0,1,""dispose_end_" + to_string(getId())+"\n");
	for (auto i : varItems->variables)
	{
		auto ifs = dynamic_cast<Interface*> (i.second->getType());

		if (ifs){
			MIPS_ASM::lw("a0", i.second->getOffset(),"a0" );
			MIPS_ASM::jal("global_dispose");
			MIPS_ASM::printComment("checking rc ifs");
			MIPS_ASM::top("a0");

		}
	}
	MIPS_ASM::label("dispose_end_" + to_string(getId()));

	MIPS_ASM::pop();
	MIPS_ASM::pop("ra");
	MIPS_ASM::jr();
	MIPS_ASM::add_instruction("\n\n");

	for (auto i = this->methodsItem->methods.begin(); i != this->methodsItem->methods.end(); i++)
	{
		MIPS_ASM::add_instruction("\n\n");

		MIPS_ASM::printComment(string("generating code for Method:") + i->second->to_string());
		MIPS_ASM::add_instruction("\n");
		if (getInheretInterface()){
			auto tm = getInheretInterface()->getMethodByName(i->second->get_name(), i->second->getReturnType(), i->second->get_variables(), i->second->get_static());
			if (tm != NULL)
			{
				if (tm->getReturnType() != i->second->getReturnType())
				{
					if (i->second->getFunctionNode() != NULL){
						i->second->getFunctionNode()->addError("inherted"  + i->second->to_string() +  "in interface "+get_name()+ " method should keep the same return type");
					}
				}
			//todo not here
				tm->hasBeenInhereted = true;
			}
		}
		if (i->second->getFunctionNode() != NULL){
			i->second->getFunctionNode()->generateCode();
		}
		else{
			Program::addError(new SemanticError("method " + i->second->to_string() +  "in interface "+get_name()+" not implemented", 0, 0, " "));
			MIPS_ASM::printComment(string("Something wrong this method's node is NULL :") + i->first);

		}
	}
	MIPS_ASM::printComment("#########################################");
	if (static_twin != NULL){
		MIPS_ASM::printComment("###STATIC");

		static_twin->generateCode();
		MIPS_ASM::printComment("###End STATIC");


		MIPS_ASM::printComment(string("is aaa: "));
		MIPS_ASM::add_instruction("\n\n");
		MIPS_ASM::label(getIsALabel());
		MIPS_ASM::li("v0", 0);

		for (auto i : children_ids){
			MIPS_ASM::li("t0", i);

			MIPS_ASM::beq("a0", "t0", getIsALabel() + "yes");
		}
		MIPS_ASM::jr();
		MIPS_ASM::label(getIsALabel() + "yes");
		MIPS_ASM::li("v0", 1);
		MIPS_ASM::jr();


	}
	MIPS_ASM::add_instruction("\n\n\n");

}

Method* Interface::getMethod(string message, vector<CallSelector*> v )
{
	return methodsItem->getMethod(message, v);
}
int Interface::getId(){
	if (_id == -1){
		_id = ++interfacesCount;
	}
	return _id;
}