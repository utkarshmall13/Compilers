#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <cstdlib>
#include <utility>
using namespace std;

enum Scope{
	Global = 0,
	Local = 1,
	Param = 2
};

enum Var_or_Func{
	Var = 0,
	Func = 1
};

enum BasicTypes{
	Void    = 0,
	Int     = 1,
	Float   = 2,
	Struct  = 3,
	Pointer = 4,
	Array   = 5,
	IntConstant = 6,
	FloatConstant = 7,
	StringConstant = 8,
	Invalid =-1
};

class Type;

class struct_elem{
	public:
	string name;
	Type* t;
};


class Type{
	public:
	int size;
	int arraylen;
	Type* subtype;
	BasicTypes basic;
	string structname;

	Type(){}

	Type(string structname);
	Type(BasicTypes t, Type* sub, int size1, int arrlen){
		if(t==BasicTypes::Void){
			size=0;
			subtype=NULL;
			basic=BasicTypes::Void;
		}

		else if(t==BasicTypes::Int){
			size=4;
			subtype=NULL;
			basic=BasicTypes::Int;
		}

		else if(t==BasicTypes::Float){
			size=4;
			subtype=NULL;
			basic=BasicTypes::Float;
		}

		else if(t==BasicTypes::IntConstant){
			size=4;
			subtype=NULL;
			basic=BasicTypes::IntConstant;
		}

		else if(t==BasicTypes::FloatConstant){
			size=4;
			subtype=NULL;
			basic=BasicTypes::FloatConstant;
		}

		else if(t==BasicTypes::StringConstant){
			size=4;
			subtype=NULL;
			basic=BasicTypes::StringConstant;
		}

		else if(t==BasicTypes::Pointer){
			size=4;
			subtype=sub;
			basic=BasicTypes::Pointer;
		}

		else if(t==BasicTypes::Array){
			arraylen=arrlen;
			subtype=sub;
			size=sub->size*arrlen;
			basic=BasicTypes::Array;
		}

		else if(t==BasicTypes::Invalid){
			arraylen=-1;
			subtype=NULL;
			size=-1;
			basic=BasicTypes::Invalid;
		}
	}

	void print();
	void print_type_struct();
	void print_cerr();
	void print_casm();
	void invert();

};

class SymTab;

class SymTabEntry{
	public:
	string symbol;
	Scope scp;
	Var_or_Func VoF;
	Type* type;
	int size;
	int offset;
	SymTab* table;

	SymTabEntry(){
		table=NULL;
		size=0;
		offset=0;
		type=new Type(BasicTypes::Invalid,NULL,-1,-1);
		type->basic=Invalid;
	}

	void print();
};

class SymTab{
	public:
	string name;
	vector<SymTabEntry*> Entries;
	//vector<BasicTypes> arg_types;
	int return_offset;
	Type* returnType;
	SymTab* parent;

	// Constructors and desctructor of this class
	SymTab(string n = "Junk"){
		name = n;
	}
	~SymTab();

	// Methods of the class
	void AddEntry(SymTabEntry* e,int line);
	bool Check_dup(SymTabEntry* e);
	SymTabEntry* GetEntry(string s);
	void print();
	void offset_calc();
	vector<Type*> get_ret_params();
	string get_name();
	Type* getTypeofID(string symbol_name);
	//void get_local_offsets(map<int,pair<bool,int> > &locals);
};

class StructTabEntry{
public:
	string name;
	vector<Type*> attrtype;
	vector<string> attrname;
	vector<int> size;
	vector<int> offset;

	StructTabEntry(){
		name="";
	}
	StructTabEntry(string name){
		this->name=name;
	}

	void print();
};


class StructTab{
public:
	vector<StructTabEntry*> ste;
	StructTab(){
	}
	void print();
	bool struct_exists(string id);
	bool Check_dupST(string id);
	void AddEntry(SymTabEntry* e,int line);
	bool Check_dup(SymTabEntry * e);
	Type* get_attr(string id,string structname);
	bool is_attr(string id,string structname);
};
