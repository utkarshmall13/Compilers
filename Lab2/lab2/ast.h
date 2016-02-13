#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <cstdlib>
#include <string>
#include <utility>
#include <assert.h>
using namespace std;

class ExpAst;
class StmtAst;
class RefAst;
class Identifier;
class IntConst;
class FloatConst;
class StringConst;


class abstract_astnode{
	public:
	virtual void print() = 0;
};

class StmtAst : public abstract_astnode{
	public:
	virtual void print() = 0;
};

class ExpAst : public abstract_astnode{
	public:
	virtual void print() = 0;
};

class RefAst : public ExpAst{
	public:
	virtual void print() = 0;
};

class Empty : public StmtAst{
	public:
	void print();
};

class Seq : public StmtAst{
	public:
	vector<StmtAst*> v;
	void insertStmt(StmtAst* s);
	void print();
};

class Ass : public StmtAst{
	ExpAst* l;
	ExpAst* r;
public:
	Ass(){}
	Ass(ExpAst* l1,ExpAst* r1){
		l=l1;
		r=r1;
	}
	void print();
};

class Return : public StmtAst{
	ExpAst* e;
public:
	Return(){}
	Return(ExpAst* e1){
		e=e1;
	}
	void print();
};

class If : public StmtAst{
	ExpAst* e;
	StmtAst* s1;
	StmtAst* s2;
public:
	If(){}
	If(ExpAst* e1,StmtAst* s11,StmtAst* s21){
		e=e1;
		s1=s11;
		s2=s21;
	}
	void print();
};

class While : public StmtAst{
	ExpAst* e;
	StmtAst* s;
public:
	While(){}
	While(ExpAst* e1,StmtAst* s1){
		e=e1;
		s=s1;
	}
	void print();
};

class For : public StmtAst{
	ExpAst* e1;
	ExpAst* e2;
	ExpAst* e3;
	StmtAst* s;
public:
	For(){}
	For(ExpAst* e11,ExpAst* e21,ExpAst* e31,StmtAst* s1){
		e1=e11;
		e2=e21;
		e3=e31;
		s=s1;
	}
	void print();
};

class Binary_op : public ExpAst{
	string op;
	ExpAst* l;
	ExpAst* r;
public:
	Binary_op(){}
	Binary_op(string op1,ExpAst* l1,ExpAst* r1){
		op=op1;
		l=l1;
		r=r1;
	}
	void print();
};

class Unary_op : public ExpAst{
	string op;
	ExpAst* e;
public:
	Unary_op(){}
	Unary_op(string op1,ExpAst* e1){
		op=op1;
		e=e1;
	}
	void print();
};

class Assign : public ExpAst{
	RefAst* l;
	ExpAst* r;
public:
	Assign(){}
	Assign(RefAst* l1,ExpAst* r1){
		l=l1;
		r=r1;
	}
	void print();
};

class Funcall : public ExpAst{
	Identifier* name;
	vector<ExpAst*> v;
public:
	Funcall(){}
	Funcall(Identifier* i){
		name=i;
	}
	void insertExp(ExpAst* e);
	void setName(Identifier* i);
	void print();
};

class FloatConst : public ExpAst{
	float val;
public:
	void print();
	FloatConst(){}
	FloatConst(float f){
		val=f;
	}
};

class IntConst : public ExpAst{
	int val;
public:
	IntConst(){}
	IntConst(int f){
		val=f;
	}
	void print();
};

class StringConst : public ExpAst{
	string val;
public:
	StringConst(){}
	StringConst(string f){
		val=f;
	}
	void print();
};

class Pointer : public ExpAst{
	RefAst* val;
public:
	Pointer(){}
	Pointer(RefAst* f){
		val=f;
	}
	void print();
};

class Member : public RefAst{
	RefAst* val;
	Identifier* id;
public:
	Member(){}
	Member(RefAst* val,Identifier* id){
		this->val=val;
		this->id=id;
	}
	void print();
};

class Arrow : public RefAst{
	RefAst* val;
	Identifier* id;
public:
	Arrow(){}
	Arrow(RefAst* val,Identifier* id){
		this->val=val;
		this->id=id;
	}
	void print();
};

class Deref : public RefAst{
	RefAst* val;
public:
	Deref(){}
	Deref(RefAst* f){
		val=f;
	}
	void print();
};

class Identifier : public RefAst{
	string val;
public:
	Identifier(){}
	Identifier(string f){
		val=f;
	}
	void print();
};

class ArrayRef : public RefAst{
	RefAst* ra;
	ExpAst* ea;
public:
	ArrayRef(){}
	ArrayRef(RefAst* ra,ExpAst* ea){
		this->ra=ra;
		this->ea=ea;
	}
	void print();
};
