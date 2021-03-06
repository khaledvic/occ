#include "Selector.h"
#include "ST\Type.h"
#include"ast\TypeChecker.h"

Selector::Selector(string name) : Symbol(name)
{
	this->name = name;
}

bool Selector ::operator==(const Selector &b){
	if (name != b.name)
		return false;
	if (getTypesSize() != b.getTypesSize())
		return false;
	for (int i = 0; i < getTypesSize(); i++)
	{
		if (b.getTypeAt(i) != getTypeAt(i))
		{
				return false;
		}
	}
	return true;

}

bool Selector ::operator>=(const Selector &b){
	if (name != b.name)
		return false;
	if (getTypesSize() != b.getTypesSize())
		return false;
	for (int i = 0; i < getTypesSize(); i++)
	{
		if (b.getTypeAt(i) != getTypeAt(i))
		{
			if (!TypeChecker::canCast(b.getTypeAt(i), getTypeAt(i)))
				return false;
		}
	}
	return true;

}

bool Selector:: compareSelector(Selector* selector1,Selector* selector2)
{
	return (*selector1)==(*selector2);
}
bool Selector :: operator<(const Selector &b)const {
	if (name != b.name)
		return name < b.name;
	if (getTypesSize() != b.getTypesSize())
		return getTypesSize()<b.getTypesSize();
	for (int i = 0; i < getTypesSize(); i++)
	{
		if (b.getTypeAt(i) != getTypeAt(i))
			return getTypeAt(i)->get_name() < b.getTypeAt(i)->get_name();
	}
	return false;

}
Selector::~Selector(void)
{
}