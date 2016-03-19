//Member function(s) for "Empty"
void Empty::print(){
	cout<<"(Empty)";
}

//Member function(s) for "Seq"
void Seq::insertStmt(StmtAst* s){
		v.push_back(s);
}

SymTab* Seq::get_symbol_table(){
	return table;
}

void Seq::print(){
	cout<<"(Block [";
	for(int i=0; i<v.size(); i++){
		v[i]->print();
		if(i!=v.size()-1)	cout<<endl;
	}
	cout<<"])\n";
}

//Member function(s) for "Ass"
void Ass::print(){
	l->print();
}

//Member function(s) for "Return"

void Return::print(){
	bool b=0,bb=0;
	Type *t=compatible_assign_types(return_type_func,e->type);
	if(t==NULL && same(return_type_func,e->type)){ t=e->type; }
	if(t==NULL){ cout<<" RETURN TYPE MISMATCH "; return; }
	cout<<"(Return";
	if(e->type->basic==BasicTypes::Int || e->type->basic==BasicTypes::Float || e->type->basic==BasicTypes::IntConstant || e->type->basic==BasicTypes::FloatConstant){
		cout<<"_";return_type_func->print_casm();cout<<" ";
		bb=1;
	}
	if(!same(return_type_func,e->type) && !var_const_same_type(e->type,return_type_func) && bb) {cout<<"(";e->type->print_casm();cout<<"_to_"; return_type_func->print_casm();cout<<" ";b=1;}
	e->print();
	if(b){cout<<") ";}
	else cout<<"";
	cout<<")";
}

//Member function(s) for "If"
void If::print(){
	cout<<"(If ";
	e->print();
	cout<<endl;
	s1->print();
	cout<<endl;
	s2->print();
	cout<<")";
}

//Member function(s) for "While"
void While::print(){
	cout<<"(While ";
	e->print();
	cout<<endl;
	s->print();
	cout<<")";
}

//Member function(s) for "For"
void For::print(){
	cout<<"(For ";
	e1->print();
	cout<<endl;
	e2->print();
	cout<<endl;
	e3->print();
	cout<<endl;
	s->print();
	cout<<")";
}

//Member function(s) for "Binary_op"
void Binary_op::print(){
	bool b=0,bb=0;
	///l->type->print_casm();r->type->print_casm();
	Type *t=NULL;
	if(op=="LT" || op=="GT" || op=="LE" || op=="GE" || op=="EQ" || op=="NE" || op=="And" || op=="Or") t=compatible_relative_types(l->type,r->type);
	if(op=="Plus" || op=="Minus") t=compatible_additive_types(l->type,r->type);
	if(op=="Mult" || op=="Div") t=compatible_types(l->type,r->type);
	if(t==NULL){ cout<<" INVALID OPERATION "; return; }
	cout<<"(";
	if((l->type->basic==BasicTypes::Int || l->type->basic==BasicTypes::Float || l->type->basic==BasicTypes::IntConstant || l->type->basic==BasicTypes::FloatConstant)
	&& (r->type->basic==BasicTypes::Int || r->type->basic==BasicTypes::Float || r->type->basic==BasicTypes::IntConstant || r->type->basic==BasicTypes::FloatConstant)) {
		cout<<op<<"_";t->print_casm();cout<<" ";
		bb=1;
	}
	if(op=="And" || op=="Or"){
		bb=0;
	}
	if(!same(t,l->type) && !var_const_same_type(l->type,t) && bb) {cout<<"(";l->type->print_casm();cout<<"_to_"; t->print_casm();cout<<" ";b=1;}
	l->print();
	if(b){cout<<") ";}
	else cout<<" ";
	b=0;
	if(!same(t,r->type) && !var_const_same_type(t,r->type) && bb) {cout<<"(";r->type->print_casm();cout<<"_to_"; t->print_casm();cout<<" ";b=1;}
	r->print();
	if(bb){cout<<")) ";}
	else cout<<") ";
}

//Member function(s) for "Unary_op"
void Unary_op::print(){
	cout<<"("<<op<<" ";
	e->print();
	cout<<")";
}

//Member function(s) for "Binary_op"
void Assign::print(){
	bool b=0,bb=0;
	///l->type->print_casm();r->type->print_casm();
	Type *t=compatible_assign_types(l->type,r->type);
	if(t==NULL && same(l->type,r->type)){ t=l->type; }
	if(t==NULL){ cout<<" INVALID ASSIGNMENT "; return; }
	cout<<"(Ass";

	if((l->type->basic==BasicTypes::Int || l->type->basic==BasicTypes::Float || l->type->basic==BasicTypes::IntConstant || l->type->basic==BasicTypes::FloatConstant)
	&& (r->type->basic==BasicTypes::Int || r->type->basic==BasicTypes::Float || r->type->basic==BasicTypes::IntConstant || r->type->basic==BasicTypes::FloatConstant)) {
		cout<<"_";t->print_casm();cout<<" ";
		bb=1;
	}
	if(!same(t,l->type) && !var_const_same_type(l->type,t) && bb) {cout<<"(";l->type->print_casm();cout<<"_to_"; t->print_casm();cout<<" ";b=1;}
	l->print();
	if(b){cout<<") ";}
	else cout<<" ";
	b=0;
	if(!same(t,r->type) && !var_const_same_type(t,r->type) && bb) {cout<<"(";r->type->print_casm();cout<<"_to_"; t->print_casm();cout<<" ";b=1;}
	r->print();
	if(b){cout<<")) ";}
	else cout<<") ";
}

//Member function(s) for "Funcall"
void Funcall::insertExp(ExpAst* e){
		v.push_back(e);
}

vector <ExpAst*> Funcall::getExpList(){
	return v;
}


void Funcall::setName(Identifier* i){
		name=i;
}

void Funcall::print(){
	cout<<"(Funcall ";
	name->print();
	vector<Type*> temp;

	for(int i=0;i<GlobalSymTab->Entries.size();i++){
		if(GlobalSymTab->Entries[i]->symbol==name->val) temp=GlobalSymTab->Entries[i]->table->get_ret_params();
	}
	cout<<" [";
	if(v.size()>0){
		for(int i=0; i<v.size(); i++){
			Type *t=compatible_assign_types(temp[i+1],v[i]->type);
			if(t==NULL && same(temp[i+1],v[i]->type)){ t=temp[i+1]; }
			if(t==NULL){ cout<<" INVALID PARAM PASS "; return; }

			if((temp[i+1]->basic==BasicTypes::Int || temp[i+1]->basic==BasicTypes::Float || temp[i+1]->basic==BasicTypes::IntConstant || temp[i+1]->basic==BasicTypes::FloatConstant)
			&& (v[i]->type->basic==BasicTypes::Int || v[i]->type->basic==BasicTypes::Float || v[i]->type->basic==BasicTypes::IntConstant || v[i]->type->basic==BasicTypes::FloatConstant)) {

					cout<<" ";v[i]->type->print_casm();cout<<"_to_";temp[i+1]->print_casm();
					cout<<"(";v[i]->print();cout<<") ";
			}
			else v[i]->print();

		}
	}
	cout<<"])";
}

//Member function(s) for "FloatConst"
void FloatConst::print(){
	cout<<"(FloatConst "<<val<<")";
}

float FloatConst::get_val(){
	return val;
}

//Member function(s) for "IntConst"
void IntConst::print(){
	cout<<"(IntConst "<<val<<")";
}

int IntConst::get_val(){
	return val;
}

//Member function(s) for "StringConst"
void StringConst::print(){
	cout<<"(StringConst "<<val<<")";
}

string StringConst::get_val(){
	return val;
}

//Member function(s) for "Pointer"
void Pointer::print(){
	cout<<"(Pointer ";
	val->print();
	cout<<")";
}

//Member function(s) for "Member"
void Member::print(){
	cout<<"(Member ";
	val->print();
	cout<<" ";
	id->print();
	cout<<")";
}

//Member function(s) for "Arrow"
void Arrow::print(){
	cout<<"(Arrow ";
	val->print();
	cout<<" ";
	id->print();
	cout<<")";
}

//Member function(s) for "Deref"
void Deref::print(){
	cout<<"(Deref ";
	val->print();
	cout<<")";
}

//Member function(s) for "ArrayRef"
void ArrayRef::print(){
	cout<<"(ArrayRef ";
	ra->print();
	cout<<" ";
	ea->print();
	cout<<")";
}

//Member function(s) for "Identifier"
void Identifier::print(){
	cout<<"(Id \""<<val<<"\")";
}
