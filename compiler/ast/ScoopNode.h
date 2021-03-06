#pragma once
#include "../VoidNode.h"
#include "../ST/variable.h"
class ScoopNode :
	public VoidNode
{
public:
	map<string, Variable*> _variables;
	list<Node *> _nodes;
	int _currentInnerOffset;
	string _offsetReg = "fp";


	virtual string getOffsetRegister()
	{
		return _offsetReg;
	}
	virtual void setOffsetRegister(string o)
	{
		_offsetReg = o;
	}
	virtual int getVarsOffset()
	{
		return getParent()==NULL?0:getParent()->getFrameSize() + getParent()->getVarsOffset();
	}
	int getNextOffset(int newSize){
		int t=_currentInnerOffset;
		_currentInnerOffset+=newSize;
		return t;
	}
	void addNode(Node* node){
		if (node)
			this->_nodes.push_back(node);
		else
			Streams::WTF() << "empty node added!!! be carefull";
	}
	int getFrameSize(){
		return _currentInnerOffset;
	}
	ScoopNode(ScoopNode * parent) :VoidNode(parent)
	{
		_currentInnerOffset=0;
	}
	
	ScoopNode* getParent(){
		return this->_scoop;
	}
	bool add_variable(Variable* variable)
	{
		if ((this!=NULL)&&(variable!= NULL)&&(!checkVariableExistance(variable->get_name())))
		{
			variable->setOffset( getNextOffset(variable->getType()->getTypeSize()));
			variable->_scoop = this;
			_variables.insert(make_pair(variable->get_name(), variable));

			return true;
		}else {
			//
			////should throw error
			/////////
			return false;
		}			
	}
	Variable* get_variable(string variable_name)
	{
		if ((this!=NULL)||(variable_name!="")){
			if(variable_name.find(".")==-1){
				map<string, Variable*>::iterator it = this->_variables.find(variable_name);
				ScoopNode* father = this;
				while(father!=NULL){
					it = father->_variables.find(variable_name);
					if ((it!=father->_variables.end())&&(it->first==variable_name)){
						return it->second;
					}
					father = father->_scoop;
				}
				if(father==NULL)
					return NULL;				
			}else{
				return NULL;			
			}

		}else {
			return NULL;
		}
	}
	bool checkVariableExistance(string name){
		if(name!="")
		{
			map<string, Variable*>::iterator it = this->_variables.find(name);
			if (it != this->_variables.end()){
				return true;
			}else{
				return false;
			}

		}
		return true;
	}
	//virtual Type* generateType()
	//{
	//return symbolTable->getType("void");
	//}

	virtual ~ScoopNode(void)
	{
	}
	void toString(){
cout<<"begin Scoop name: "<<"\n variables:\n";
for(map<string, Variable*>::iterator it=_variables.begin();it!=_variables.end();it++)
	it->second->toString();
for(auto i=_nodes.begin();i!=_nodes.end();i++){
	(*i)->toString();
}
cout<<"end scooooooooooooop\n";

}
	virtual void generateCode();
	virtual void nullVars();
	virtual void gcVars();
	virtual bool typeCheck()
	{
		return 1;
	}	
};

