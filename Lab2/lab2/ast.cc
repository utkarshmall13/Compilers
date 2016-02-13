//Member function(s) for "Empty"
void Empty::print(){
	cout<<"(Empty)";
}

//Member function(s) for "Seq"
void Seq::insertStmt(StmtAst* s){
		v.push_back(s);
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
	cout<<"(Ass ";
	l->print();
	cout<<" ";
	r->print();
	cout<<")";
}

//Member function(s) for "Return"
void Return::print(){
	cout<<"(Return ";
	e->print();
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
	cout<<"("<<op<<" ";
	l->print();
	cout<<" ";
	r->print();
	cout<<")";
}

//Member function(s) for "Unary_op"
void Unary_op::print(){
	cout<<"("<<op<<" ";
	e->print();
	cout<<")";
}

//Member function(s) for "Binary_op"
void Assign::print(){
	cout<<"(Ass ";
	l->print();
	cout<<" ";
	r->print();
	cout<<")";
}

//Member function(s) for "Funcall"
void Funcall::insertExp(ExpAst* e){
		v.push_back(e);
}

void Funcall::setName(Identifier* i){
		name=i;
}

void Funcall::print(){
	cout<<"(Funcall ";
	name->print();
	cout<<" [";
	for(int i=0; i<v.size()-1; i++){
		v[i]->print();
		cout<<" ";
	}
	v[v.size()-1]->print();
	cout<<"])";
}

//Member function(s) for "FloatConst"
void FloatConst::print(){
	cout<<"(FloatConst "<<val<<")";
}

//Member function(s) for "IntConst"
void IntConst::print(){
	cout<<"(IntConst "<<val<<")";
}

//Member function(s) for "StringConst"
void StringConst::print(){
	cout<<"(StringConst "<<val<<")";
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
