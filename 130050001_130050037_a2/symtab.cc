#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <cstdlib>
#include <utility>
#include <fstream>
using namespace std;

ofstream out_symtab("symtab.out", ios::out);
ofstream out_struct("structtab.out", ios::out);

bool print_symtab_flag = 1;
bool print_ast_flag = 1;
bool print_structtab_flag = 1;
Scope curr_scope = Scope::Global;

SymTab* CurrSymTab = new SymTab("Global");
SymTab * GlobalSymTab=new SymTab("Global");
vector<SymTab*> SymTabStack(1,CurrSymTab);
vector<int> offsetStack(1,0);

Type* curr_type;
int global_offset = 8;
int offset_multiplier = 1;
int valid_const;
int dimension;
int curr_offset=4;
int tot_param_size=0;
int val_of_IntConst=-1;
bool flag=0;

StructTab* structtable= new StructTab();
bool is_func_not_struct=true;
bool struct_redef=false;
string current_struct_name="";

Type* return_type_func=NULL;

bool same(Type* x,Type* y){
	if(x==NULL || y==NULL) return false;
	bool mismatch=false;
	if(x->basic!=y->basic || x->structname!=y->structname) return false;
	while(x->subtype!=NULL && y->subtype!=NULL){
		if(x->basic!=y->basic) mismatch=true;
		if(x->size!=y->size && !(x->size<0&&y->size<0)) mismatch=true;
		if(x->arraylen!=y->arraylen && !(x->arraylen<0&&y->arraylen<0)) mismatch=true;
		if(x->structname!=y->structname) mismatch=true;
		x=x->subtype;
		y=y->subtype;
	}
	return x->subtype==NULL && y->subtype==NULL && !mismatch && x->basic==y->basic;
}

Type* compatible_types(Type* x, Type* y){
	if(x==NULL || y== NULL) return NULL;
	if(x->subtype !=NULL || y->subtype != NULL) return NULL;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Int) return x;
	if(y->basic == BasicTypes::Float && x->basic == BasicTypes::Int) return y;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::Int) return x;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Float) return x;

	if(x->basic == BasicTypes::FloatConstant && y->basic == BasicTypes::Int) {Type* t=new Type(BasicTypes::Float,NULL,-1,-1);return t;}
	if(y->basic == BasicTypes::FloatConstant && x->basic == BasicTypes::Int) {Type* t=new Type(BasicTypes::Float,NULL,-1,-1);return t;}
	if(x->basic == BasicTypes::IntConstant && y->basic == BasicTypes::Int) return y;
	if(x->basic == BasicTypes::FloatConstant && y->basic == BasicTypes::Float) return y;

	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::IntConstant) return x;
	if(y->basic == BasicTypes::Float && x->basic == BasicTypes::IntConstant) return y;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::IntConstant) return x;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::FloatConstant) return x;

	if(x->basic == BasicTypes::FloatConstant && y->basic == BasicTypes::IntConstant) return x;
	if(y->basic == BasicTypes::FloatConstant && x->basic == BasicTypes::IntConstant) return y;
	if(x->basic == BasicTypes::IntConstant && y->basic == BasicTypes::IntConstant) return x;
	if(x->basic == BasicTypes::FloatConstant && y->basic == BasicTypes::FloatConstant) return x;
	return NULL;
}

Type* compatible_additive_types(Type* x, Type* y){ //pointer artihmetic allowed
	Type* temp=compatible_types(x,y);
	if(temp!=NULL) return temp;
	if(x==NULL || y== NULL) return NULL;
	if(x->basic==BasicTypes::Pointer && (y->basic==BasicTypes::Int||y->basic==BasicTypes::IntConstant)){
		return x;
	}
	if(y->basic==BasicTypes::Pointer && (x->basic==BasicTypes::Int||x->basic==BasicTypes::IntConstant)){
		return y;
	}
	return NULL;
}

Type* compatible_relative_types(Type* x, Type* y){ //pointer comparisons allowed, string literal comparison allowed
	Type* temp=compatible_types(x,y);
	Type* result=new Type(BasicTypes::Int,NULL,-1,-1);
	if(temp!=NULL) return result;
	if(x==NULL || y== NULL) return NULL;
	if(x->basic==BasicTypes::Pointer && y->basic==BasicTypes::Pointer){
		return result;
	}
	return NULL;
}


Type* compatible_assign_after_pointer_types(Type* x, Type* y){
	if(x==NULL || y== NULL) return NULL;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Float) return x;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Int) return NULL;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::Int) return x;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::Float) {return NULL;}
	//void cases
	if(x->basic == BasicTypes::Void) return x;
	if(y->basic == BasicTypes::Void) return x;
	if(x->basic == BasicTypes::Pointer && y->basic == BasicTypes::Pointer){
		if(!same(x->subtype,y->subtype)) return NULL;
		Type* childtype=compatible_assign_after_pointer_types(x->subtype,y->subtype);
		if(childtype==NULL){return NULL;}
		return new Type(BasicTypes::Pointer,childtype,-1,-1);
	}
	return NULL;
}
Type* compatible_assign_types(Type* x, Type* y){ //pointer comparisons allowed, string literal comparison allowed
	if(x==NULL || y== NULL) return NULL;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Float) return x;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::Int) return x;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::Int) return x;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::Float) return x;

	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::IntConstant) return x;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::FloatConstant) return x;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::IntConstant) return x;
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::FloatConstant) return x;
	if(x->basic == BasicTypes::Array) return NULL;
	if(same(x,y)) return x;
	if(x->basic == BasicTypes::Pointer && y->basic == BasicTypes::IntConstant){
		if(val_of_IntConst==0)return x;
		return NULL;
	}
	if(x->basic == BasicTypes::Pointer && y->basic == BasicTypes::Array){
		Type* childtype=compatible_assign_after_pointer_types(x->subtype,y->subtype);
		if(childtype==NULL){return NULL;}
		return new Type(BasicTypes::Pointer,childtype,-1,-1);
	}

	if(x->basic == BasicTypes::Pointer && y->basic == BasicTypes::Pointer){
		Type* childtype=compatible_assign_after_pointer_types(x->subtype,y->subtype);
		if(childtype==NULL){return NULL;}
		return new Type(BasicTypes::Pointer,childtype,-1,-1);
	}

	return NULL;
}


bool similar(Type* x,Type* y){
	bool mismatch=false;
	while(x->subtype!=NULL && y->subtype!=NULL){
		if(x->basic!=y->basic) mismatch=true;
		x=x->subtype;
		y=y->subtype;
	}
	return x->subtype==NULL && y->subtype==NULL && !mismatch ;
}

bool first_dimension_mismatch(Type* x,Type* y){
	if(x->basic==BasicTypes::Array && y->basic==BasicTypes::Array){
		return same(x->subtype,y->subtype);
	}
	return false;
}
bool extra_compatible(Type* x,Type* y){
	if(first_dimension_mismatch(x,y)) return true;
	if(x->basic==BasicTypes::Array && x->subtype->basic!=BasicTypes::Array && y->basic==BasicTypes::Pointer && similar(x->subtype,y->subtype)) return true;
	if(y->basic==BasicTypes::Array && y->subtype->basic!=BasicTypes::Array && x->basic==BasicTypes::Pointer && similar(y->subtype,x->subtype)) return true;
	return false;
}

bool void_check(Type* x){
	if(x->basic==BasicTypes::Void) return true;
	while(1){
		if(x->subtype==NULL) return false;
		if(x->basic==BasicTypes::Array && x->subtype->basic==BasicTypes::Void) return true;
		x=x->subtype;
	}

}

void SymTab::AddEntry(SymTabEntry* e,int line){
	e->type->invert();
	if(void_check(e->type)&&e->scp!=Scope::Global){cerr<<"Error at line "<<line<<" : Variable \'"<<e->symbol<<"\' of type void is not allowed"<<endl; return;}
	e->size=e->type->size;
	Entries.push_back(e);
}

bool SymTab::Check_dup(SymTabEntry* e){
	for(int i=0;i<Entries.size();i++){
		if(Entries[i]->symbol==e->symbol) return false;
	}
	return true;
}

SymTabEntry* SymTab::GetEntry(string s){
	for(int i=0;i<Entries.size();i++){
		if(Entries[i]->symbol==s) return Entries[i];
	}
	return NULL;
}

void SymTab::print(){
	if(name!="Global"){
		out_symtab<<name<<" "<<Entries.size()<<endl;
		if(Entries.size()!=0) out_symtab<<"variable\tscope\t\tVaroFunc\t\tsize\t\toffset\t\ttype\t\t"<<endl;
		for(int i=0;i<Entries.size();i++){
			Entries[i]->print();
		}
		out_symtab<<endl;
	}
	else{
		out_symtab<<name<<" "<<Entries.size()<<endl;
		if(Entries.size()!=0) out_symtab<<"variable\tscope\t\tVaroFunc\t\tsize\t\toffset\t\ttype\t\t"<<endl;
		for(int i=0;i<Entries.size();i++){
			out_symtab<<Entries[i]->symbol<<"\t\t";
			switch(Entries[i]->scp){
				case Scope::Global : out_symtab<<"Global"; break;
				case Scope::Local  : out_symtab<<"Local"; break;
				case Scope::Param  : out_symtab<<"Param"; break;
			}
			out_symtab<<"\t\t";
			switch(Entries[i]->VoF){
				case Var_or_Func::Var  : out_symtab<<"Variable"; break;
				case Var_or_Func::Func : out_symtab<<"Function"; break;
			}
			out_symtab<<"\t\t"<<Entries[i]->table->returnType->size<<"\t\t"<<Entries[i]->offset<<"\t\t";
			if(Entries[i]->table->returnType->basic!=Invalid) Entries[i]->table->returnType->print();
			out_symtab<<endl;
		}
		out_symtab<<endl;
	}
}


string SymTab::get_name(){
	return name;
}

void SymTabEntry::print(){
	out_symtab<<symbol<<"\t\t";
	switch(scp){
		case Scope::Global : out_symtab<<"Global"; break;
		case Scope::Local  : out_symtab<<"Local"; break;
		case Scope::Param  : out_symtab<<"Param"; break;
	}
	out_symtab<<"\t\t";
	switch(VoF){
		case Var_or_Func::Var  : out_symtab<<"Variable"; break;
		case Var_or_Func::Func : out_symtab<<"Function"; break;
	}
	out_symtab<<"\t\t"<<size<<"\t\t"<<offset<<"\t\t";
	if(type->basic!=Invalid) type->print();
	out_symtab<<endl;
}

bool StructTab::Check_dupST(string id){
	for(int i=0;i<ste.size();i++){
		if(ste[i]->name==id) return false;
	}
	return true;
}


void Type::print(){
	if(basic==Array)	{	out_symtab<<"Array("<<arraylen<<", ";subtype->print();out_symtab<<")"; }
	if(basic==Pointer)  {	out_symtab<<"Pointer(";subtype->print();out_symtab<<")";               }
	if(basic==Int)      {	out_symtab<<"Int";                                               }
	if(basic==Void)     {	out_symtab<<"Void";                                              }
	if(basic==Float)    {	out_symtab<<"Float";                                             }
	if(basic==Struct)   {	out_symtab<<"Struct "<<structname;                               }
	if(basic==Invalid)  {	out_symtab<<"Invalid";                                           }
	if(basic==IntConstant)    {	out_symtab<<"IntConst";                                      }
	if(basic==FloatConstant)  {	out_symtab<<"FloatConst";                                    }
	if(basic==StringConstant) {	out_symtab<<"StringConst";                                   }
}

void Type::print_type_struct(){
	if(basic==Array)	{	out_struct<<"Array("<<arraylen<<", ";subtype->print_type_struct();out_struct<<")"; }
	if(basic==Pointer)  {	out_struct<<"Pointer(";subtype->print_type_struct();out_struct<<")";               }
	if(basic==Int)      {	out_struct<<"Int";                                               }
	if(basic==Void)     {	out_struct<<"Void";                                              }
	if(basic==Float)    {	out_struct<<"Float";                                             }
	if(basic==Struct)   {	out_struct<<"Struct "<<structname;                               }
	if(basic==Invalid)  {	out_struct<<"Invalid";                                           }
	if(basic==IntConstant)    {	out_struct<<"IntConst";                                      }
	if(basic==FloatConstant)  {	out_struct<<"FloatConst";                                    }
	if(basic==StringConstant) {	out_struct<<"StringConst";                                   }
}

void Type::print_cerr(){
	if(basic==Array)	{	cerr<<"Array("<<arraylen<<", ";subtype->print_cerr();cerr<<")"; }
	if(basic==Pointer)  {	cerr<<"Pointer(";subtype->print_cerr();cerr<<")";               }
	if(basic==Int)      {	cerr<<"Int";                                               }
	if(basic==Void)     {	cerr<<"Void";                                              }
	if(basic==Float)    {	cerr<<"Float";                                             }
	if(basic==Struct)   {	cerr<<"Struct "<<structname;                               }
	if(basic==Invalid)  {	cerr<<"Invalid";                                           }
	if(basic==IntConstant)    {	cerr<<"IntConst";                                      }
	if(basic==FloatConstant)  {	cerr<<"FloatConst";                                    }
	if(basic==StringConstant) {	cerr<<"StringConst";                                   }
}

void Type::print_casm(){
	if(basic==Array)	{	cout<<"Array("<<arraylen<<", ";subtype->print_casm();cout<<")"; }
	if(basic==Pointer)  {	cout<<"Pointer(";subtype->print_casm();cout<<")";               }
	if(basic==Int)      {	cout<<"Int";                                               }
	if(basic==Void)     {	cout<<"Void";                                              }
	if(basic==Float)    {	cout<<"Float";                                             }
	if(basic==Struct)   {	cout<<"Struct "<<structname;                               }
	if(basic==Invalid)  {	cout<<"Invalid";                                           }
	if(basic==IntConstant)    {	cout<<"IntConst";                                      }
	if(basic==FloatConstant)  {	cout<<"FloatConst";                                    }
	if(basic==StringConstant) {	cout<<"StringConst";                                   }
}

void Type::invert(){
	//To extract basic type
	if(subtype==NULL) return;
	if(subtype->subtype==NULL) return;

	Type* curr=this;
	while(curr->subtype!=NULL){
		curr=curr->subtype;
	}
	Type* ret=curr;
	curr=this;

	while(curr->subtype!=NULL){
		ret=new Type(curr->basic,ret,curr->size,curr->arraylen);
		curr=curr->subtype;
	}

	*this=*ret;
}

void SymTab::offset_calc(){
	if(Entries.size()==0) return;
	Entries[Entries.size()-1]->offset=4;
	for(int i=Entries.size()-2;i>=0;i--){
		Entries[i]->offset=Entries[i+1]->offset+Entries[i+1]->size;
	}
}

vector<Type*> SymTab::get_ret_params(){
	vector<Type*> v;
	v.push_back(returnType);
	for(int i=0;i<Entries.size();i++){
		if(Entries[i]->scp==Scope::Param) v.push_back(Entries[i]->type);
	}
	return v;
}

Type* SymTab::getTypeofID(string symbol_name){
	for(int i=0;i<Entries.size();i++){
		if(Entries[i]->symbol==symbol_name){
			return Entries[i]->type;
		}
	}
	return NULL;
}

void StructTabEntry::print(){
	out_struct<<"STRUCT "<<name<<endl;
	if(attrname.size()!=0) out_struct<<"attr\t\tsize\t\toffset\t\ttype"<<endl;
	for(int i=0;i<attrname.size();i++){
		out_struct<<attrname[i];
		out_struct<<"\t\t"<<size[i]<<"\t\t"<<offset[i]<<"\t\t";
		attrtype[i]->print_type_struct();
		out_struct<<endl;
	}
	out_struct<<endl;
}

void StructTab::print(){
	if(ste.size()==0){
		out_struct<<"STRUCTTABLEEMPTY"<<endl;
	}
	else{
		for(int i=0;i<ste.size();i++){
			ste[i]->print();
		}
	}
}

bool StructTab::Check_dup(SymTabEntry* e){
	if(ste.size()==0) return true;
	for(int i=0;i<ste[ste.size()-1]->attrname.size();i++){
		if(ste[ste.size()-1]->attrname[i]==e->symbol) return false;
	}
	return true;
}

void StructTab::AddEntry(SymTabEntry* e,int line){
	e->type->invert();
	if(void_check(e->type)){cerr<<"Error at line "<<line<<" : Variable \'"<<e->symbol<<"\' of type void is not allowed"<<endl; return;}

	if(ste.size()==0) return;

	ste[ste.size()-1]->attrname.push_back(e->symbol);
	ste[ste.size()-1]->attrtype.push_back(e->type);
	ste[ste.size()-1]->size.push_back(e->type->size);
	ste[ste.size()-1]->offset.push_back(curr_offset);
	return;
}

bool StructTab::struct_exists(string id){
	for(int i=0;i<ste.size();i++){
		if(ste[i]->name==id)return true;
	}
	return false;
}


Type::Type(string structname){
	for(int i=0;i<structtable->ste.size();i++){
		if(structtable->ste[i]->name==structname){
			this->structname=structname;
			size=0;
			for(int j=0;j<structtable->ste[i]->attrtype.size();j++){
				size+=structtable->ste[i]->attrtype[j]->size;
			}
			basic=BasicTypes::Struct;
			arraylen=-1;
			subtype=NULL;
		}
	}
}


Type* StructTab::get_attr(string id,string structname){
	for(int i=0;i<ste.size();i++){
		if(ste[i]->name==structname){
			for(int j=0;j<ste[i]->attrname.size();j++){
				if(id==ste[i]->attrname[j]) return ste[i]->attrtype[j];
			}
		}
	}
	return NULL;
}

bool StructTab::is_attr(string id,string structname){
	for(int i=0;i<ste.size();i++){
		if(ste[i]->name==structname){
			for(int j=0;j<ste[i]->attrname.size();j++){
				if(id==ste[i]->attrname[j]) return true;
			}
		}
	}
	return false;
}

bool var_const_same_type(Type* x, Type* y){
	if(x->basic == BasicTypes::Int && y->basic == BasicTypes::IntConstant) return true;
	if(x->basic == BasicTypes::Float && y->basic == BasicTypes::FloatConstant) return true;
	if(y->basic == BasicTypes::Int && x->basic == BasicTypes::IntConstant) return true;
	if(y->basic == BasicTypes::Float && x->basic == BasicTypes::FloatConstant) return true;
	return false;
}
