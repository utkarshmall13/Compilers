%scanner Scanner.h
%scanner-token-function d_scanner.lex()

%token VOID INT FLOAT FLOAT_CONSTANT INT_CONSTANT AND_OP OR_OP STRUCT PTR_OP OTHERS
%token EQ_OP NE_OP LE_OP GE_OP STRING_LITERAL IF ELSE WHILE FOR RETURN IDENTIFIER INC_OP

%polymorphic eAst : ExpAst* ; sAst : StmtAst*; rAst : RefAst*; Int : int; Float : float; String : string; symbolTableEntry : SymTabEntry*;

%type <eAst> expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression primary_expression postfix_expression logical_or_expression logical_and_expression constant_expression expression_list
%type <sAst> function_definition compound_statement statement_list statement selection_statement iteration_statement assignment_statement
%type <rAst> l_expression
%type <Int> INT_CONSTANT parameter_list
%type <Float> FLOAT_CONSTANT
%type <String> STRING_LITERAL IDENTIFIER unary_operator struct
%type <symbolTableEntry> declarator fun_declarator identifier_left;

%%

E: translation_unit{
		if(print_symtab_flag){
			CurrSymTab->print();
		}
		if(print_structtab_flag){
			structtable->print();
		}
};


translation_unit:
    struct_specifier|
	function_definition{
		if(print_ast_flag) $1->print();
        if(print_symtab_flag)CurrSymTab->print();
        CurrSymTab = SymTabStack.back();
		SymTabStack.pop_back();
		curr_scope=Scope::Global;
    }
	| translation_unit struct_specifier
    | translation_unit function_definition{
		if(print_ast_flag) $2->print();
        if(print_symtab_flag)CurrSymTab->print();
        CurrSymTab = SymTabStack.back();
		SymTabStack.pop_back();
		curr_scope=Scope::Global;
    }
	;

struct_specifier:
    struct '{' declaration_list '}' ';'{is_func_not_struct=true;};

struct:
	STRUCT IDENTIFIER
		{
			curr_offset=0;
			current_struct_name=$2;
			is_func_not_struct=false;
			struct_redef=false;
			bool check_dup=structtable->Check_dupST($2);
			if(!check_dup){
				struct_redef=true;
				cerr<<"Error at line "<<line<<" : Struct "<<$2<<" Redefined\n";
			}
			else{
				StructTabEntry* temp=new StructTabEntry($2);
				structtable->ste.push_back(temp);
			}
		};

function_definition:
    type_specifier fun_declarator compound_statement{
        $$=$3;
    };

type_specifier:
    VOID{curr_type=new Type(BasicTypes::Void,NULL,-1,-1);}
    | INT {curr_type=new Type(BasicTypes::Int,NULL,-1,-1);}
	| FLOAT {curr_type=new Type(BasicTypes::Float,NULL,-1,-1);}
    | STRUCT IDENTIFIER{
		if(structtable->struct_exists($2)){
			curr_type=new Type($2);
		}
		else{
			cerr<<"Error at line "<<line<<" : Struct\'"<<$2<<"\' does not exists\n";
			curr_type=new Type(BasicTypes::Invalid,NULL,-1,-1);
		}
	};

fun_declarator:
    identifier_left parameter_list ')'
    {
		CurrSymTab->offset_calc();
        curr_scope = Scope::Local;
	    CurrSymTab->return_offset = global_offset;
	    global_offset = -4;
        offset_multiplier = -1;
	    curr_offset=-4;
	}
	| IDENTIFIER '(' ')'
	{
		string temp_str=$1;
		$$ = new SymTabEntry();
		$$->symbol = temp_str;
		$$->scp = curr_scope;
		$$->VoF = Var_or_Func::Func;
		$$->type = curr_type;

		curr_scope = Scope::Local;

		SymTab *temp = new SymTab($1);
		//temp->nested_level = SymbolTableStack.size();
		$$->table = temp;
		temp->returnType = curr_type;
		return_type_func=curr_type;
		temp->parent = CurrSymTab;

		bool check_dup=CurrSymTab->Check_dup($$);

		if(!check_dup)
			cerr<<"Error at line "<<line<<" : Function "<<$1<<" Redefined\n";

		else CurrSymTab->AddEntry($$,line);

		SymTabStack.push_back(CurrSymTab);
		CurrSymTab = temp;
		CurrSymTab->offset_calc();
		offsetStack.push_back(global_offset);
		global_offset = 8;
		offset_multiplier = 1;
        curr_offset = -4;
	}


	| '*' fun_declarator{
		SymTabEntry* temp=$2;
		$$=$2;
		return_type_func=new Type(BasicTypes::Pointer,temp->table->returnType,-1,-1);
		$$->table->returnType=new Type(BasicTypes::Pointer,temp->table->returnType,-1,-1);
	};

identifier_left:	IDENTIFIER '('
    {
		$$ = new SymTabEntry();
		$$->symbol = $1;
		$$->scp = curr_scope;
		$$->VoF = Var_or_Func::Func;
		$$->type = curr_type;

		curr_scope = Scope::Param;

		SymTab *temp = new SymTab($1);
		//temp->nested_level = SymbolTableStack.size();
		$$->table = temp;
		temp->returnType = curr_type;
		return_type_func=curr_type;
		temp->parent = CurrSymTab;

		bool check_dup=CurrSymTab->Check_dup($$);

		if(!check_dup)
			cerr<<"Error at line "<<line<<" : Function "<<$1<<" Redefined\n";

		else CurrSymTab->AddEntry($$,line);

		SymTabStack.push_back(CurrSymTab);
		CurrSymTab = temp;
		offsetStack.push_back(global_offset);
		global_offset = 8;
		offset_multiplier = 1;

    };


parameter_list:
    parameter_declaration
	| parameter_list ',' parameter_declaration;

parameter_declaration:
    type_specifier	declarator{
		bool check_dup=CurrSymTab->Check_dup($2);
        if(!check_dup)
			cerr<<"Error at line "<<line<<" : duplicate parameter \'"<<$2->symbol<<"\'\n";
        else{
			$2->offset=curr_offset;
			CurrSymTab->AddEntry($2,line);
			curr_offset+=$2->size;
			tot_param_size+=$2->size;
		}
    };


declarator:
	IDENTIFIER{
		$$ = new SymTabEntry();
		$$->symbol = $1;
		$$->scp = curr_scope;
		$$->VoF = Var_or_Func::Var;
		$$->type = curr_type;
		$$->offset = global_offset;
		global_offset = global_offset + $$->size * offset_multiplier;
	}

	| '*' declarator {
		$$=$2;
		$$->type=new Type(BasicTypes::Pointer,$2->type,-1,-1);
    }

	| declarator '[' primary_expression ']'{
		$$=$1;
		if(valid_const) $$->type=new Type(BasicTypes::Array,$1->type,-1,((IntConst*)$3)->get_val());
		else cerr<<"Error at line "<<line<<": Array dimension must be an int constant"<<endl;
	}
	 // check separately that it is a constant


    ;

primary_expression:
    IDENTIFIER{
        string IDname=$1;
        $$=new Identifier($1);
        GlobalSymTab=SymTabStack.back();
        bool is_global=true,is_local=true;
        if(GlobalSymTab->getTypeofID(IDname)==NULL) is_global=false;
        if(CurrSymTab->getTypeofID(IDname)==NULL) is_local=false;
        if(is_local) $$->type=CurrSymTab->getTypeofID(IDname);
        else if(is_global) $$->type=CurrSymTab->getTypeofID(IDname);
        else {
            cerr<<"Error at line "<<line<<": Variable or Function name undefined"<<endl;
            $$->type= new Type(BasicTypes::Invalid,NULL,-1,-1);
        }
        valid_const=false;
		$$->lvalue=true;
    }

    | INT_CONSTANT{
        $$=new IntConst($1);
		val_of_IntConst=$1;
        $$->type=new Type(BasicTypes::IntConstant,NULL,-1,-1);
        valid_const=true;
		$$->lvalue=false;
    }
	| FLOAT_CONSTANT{
        $$=new FloatConst($1);
        $$->type=new Type(BasicTypes::FloatConstant,NULL,-1,-1);
        valid_const=false;
		$$->lvalue=false;
    }
    | STRING_LITERAL{
        $$=new StringConst($1);
        $$->type=new Type(BasicTypes::StringConstant,NULL,-1,-1);
        valid_const=false;
		$$->lvalue=false;
    }
	| '(' expression ')'{
        $$=$2;
        valid_const=false;
		$$->lvalue=$2->lvalue;
    }
	;


compound_statement:
    '{' '}'{
        $$=new Seq();
    }
	| '{' statement_list '}'{
        $$=$2;
    }
    | '{' declaration_list statement_list '}'{
        $$=$3;
    };

statement_list:
    statement{
        StmtAst* tmp=$1;
        $$=new Seq();
        ((Seq*)$$)->insertStmt(tmp);
    }
    | statement_list statement{
        $$=$1;
        StmtAst* tmp=$2;
        ((Seq*)$$)->insertStmt(tmp);
    }
	;

statement:
    '{' statement_list '}'{
        $$=$2;
    }  //a solution to the local decl problem
    | selection_statement{
        $$=$1;
    }
    | iteration_statement{
        $$=$1;
    }
	| assignment_statement{
        $$=$1;
    }
    | RETURN expression ';'{
		Type* temp=$2->type;
		if(return_type_func->basic==BasicTypes::Void && temp->basic!=BasicTypes::Void){
			cerr<<"Error at line "<<line<<": \'return\' with a value, in function returning void"<<endl;
		}
		else if(compatible_assign_types(return_type_func,temp)==NULL){
			cerr<<"Error at line "<<line<<":return from incompatible type"<<endl;
		}
		else if(temp->basic==BasicTypes::Array){
			cerr<<"Warning at line "<<line<<":returning array is dangerous"<<endl;
		}
		//else if()
		$$=new Return($2);
	}
    ;

assignment_statement:
    ';'{
        $$=new Empty();
    }
	|  expression ';'{
        $$=new Ass($1);
    }
	;

expression:
    logical_or_expression{
        $$=$1;
    }
    | unary_expression '=' expression{
		Type *result = compatible_assign_types($1->type,$3->type);
			if($1->lvalue==false){
				cerr<<"Error at line "<<line<<": lvalue required for assignment"<<endl;
			}
			if(result==NULL){
				cerr<<"Error at line "<<line<<":assignment from incompatible type"<<endl;
			}
			$$=new Assign((RefAst*)$1,$3);
			$$->type=result;
			$$->lvalue=false;
		};

logical_or_expression:
    logical_and_expression { $$=$1; }
    | logical_or_expression OR_OP logical_and_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'==\'"<<endl;
		}
        $$=new Binary_op("Or",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	;

logical_and_expression:
    equality_expression{
        $$=$1;
    }
    | logical_and_expression AND_OP equality_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'==\'"<<endl;
		}
        $$=new Binary_op("And",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	;

equality_expression:
    relational_expression{
        $$=$1;
    }
    | equality_expression EQ_OP relational_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'==\'"<<endl;
		}
        $$=new Binary_op("EQ",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	| equality_expression NE_OP relational_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'!=\'"<<endl;
		}
        $$=new Binary_op("NE",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	;

relational_expression:
    additive_expression{
        $$=$1;
    }
    | relational_expression '<' additive_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'<\'"<<endl;
		}
        $$=new Binary_op("LT",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	| relational_expression '>' additive_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'>\'"<<endl;
		}
        $$=new Binary_op("GT",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	| relational_expression LE_OP additive_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'<=\'"<<endl;
		}
        $$=new Binary_op("LE",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
    | relational_expression GE_OP additive_expression{
		Type *result = compatible_relative_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary \'>=\'"<<endl;
		}
        $$=new Binary_op("GE",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	;

additive_expression:
    multiplicative_expression{
        $$=$1;
    }
	| additive_expression '+' multiplicative_expression{
		Type *result = compatible_additive_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary +"<<endl;
		}
        $$=new Binary_op("Plus",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	| additive_expression '-' multiplicative_expression{
		Type *result = compatible_additive_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary -"<<endl;
		}
		$$=new Binary_op("Minus",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	;

multiplicative_expression:
    unary_expression{
        $$=$1;
    }
	| multiplicative_expression '*' unary_expression{
		Type *result = compatible_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary *"<<endl;
		}
		$$=new Binary_op("Mult",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    }
	| multiplicative_expression '/' unary_expression{
		Type *result = compatible_types($1->type,$3->type);
		if(result==NULL){
			cerr<<"Error at line "<<line<<": invalid operands to binary /"<<endl;
		}
		$$=new Binary_op("Div",$1,$3);
		$$->type=result;
		$$->lvalue=false;
    };

unary_expression:
    postfix_expression{
        $$=$1;
    }
	| unary_operator unary_expression{
        Type* temp=$2->type;
        $$=new Unary_op($1,$2);
        $$->type=NULL;
        if($2->type!=NULL){
            if($1=="Uminus"){
				$$->lvalue=false;
                if($2->type->subtype!=NULL){
                    cerr<<"Error at line "<<line<<": Invalid unary operator \'-\' at line "<<line<<" \n";
                }
                else if($2->type->basic!=BasicTypes::Float&&$2->type->basic!=BasicTypes::Int){
                    cerr<<"Error at line "<<line<<": Invalid unary operator \'-\' at line "<<line<<" \n";
                }
                else{
                    $$->type=temp;
                }
            }
            else if($1=="Not"){
				$$->lvalue=false;
                if($2->type->subtype!=NULL){
                    cerr<<"Error at line "<<line<<": Invalid unary operator \'!\' at line "<<line<<" \n";
                }
                else if($2->type->basic!=BasicTypes::Float&&$2->type->basic!=BasicTypes::Int){
                    cerr<<"Error at line "<<line<<": Invalid unary operator \'!\' at line "<<line<<" \n";
                }
                else{
                    $$->type=temp;
                }
            }
            else if($1=="Pointer"){
				$$->lvalue=false;
                $$->type=new Type(BasicTypes::Pointer,temp,-1,-1);
            }
            else if($1=="Deref"){
				if($2->type->basic==BasicTypes::Pointer){
					$$->lvalue=true;
				}
				else{
					$$->lvalue=false;
				}
                if($2->type->basic!=BasicTypes::Pointer){
                    cerr<<"Error at line "<<line<<": invalid type argument of unary ‘\'*\'\n";
                }
                else {
					if($2->type->subtype->basic==BasicTypes::Void){
						cerr<<"Error at line "<<line<<": dereferencing \'void *\' pointer\n";
					}
					else $$->type=temp->subtype;
				}
            }
        }
    };

postfix_expression:
    primary_expression{
		bool lvalue=$1->lvalue;
        $$=$1;
		$$->lvalue=lvalue;
    }

    | IDENTIFIER '(' ')'{
		GlobalSymTab=SymTabStack.back();
		string IDname=$1;
		SymTab * functionTable;
		Type* temp= new Type(BasicTypes::Invalid,NULL,-1,-1);
		if(GlobalSymTab->getTypeofID(IDname)==NULL){
		    cerr<<"Error at line "<<line<<": undefined reference to \'"<<IDname<<"\'\n";
		}
		else{
		    functionTable = (GlobalSymTab->GetEntry(IDname))->table;
			vector<Type*> v=functionTable->get_ret_params();
			if(functionTable->Entries.size()!=0){
				if(functionTable->Entries[0]->scp==Scope::Param){
					cerr<<"Error at line "<<line<<": expected "<<v.size()-1<<" arguments, but passed 0."<<endl;
				}
				else{
					temp=v[0];
				}
			}
			else{
				temp=v[0];
			}
		}
		$$=new Funcall(new Identifier($1));
		$$->type=temp;
		$$->lvalue=false;
	}
	| IDENTIFIER '(' expression_list ')'{
		GlobalSymTab=SymTabStack.back();
        string IDname=$1;
		Identifier *tmp=new Identifier($1);
		SymTab * functionTable;
		Type* temp= new Type(BasicTypes::Invalid,NULL,-1,-1);
		vector<ExpAst*> explist=((Funcall*)$3)->getExpList();

        if(GlobalSymTab->getTypeofID(IDname)==NULL){
            cerr<<"Error at line "<<line<<": undefined reference to \'"<<IDname<<"\'\n";
        }
		else{
			functionTable = (GlobalSymTab->GetEntry(IDname))->table;
			vector<Type*> v=functionTable->get_ret_params();
			if(v.size()!=explist.size()+1){
				cerr<<"Error at line "<<line<<": expected "<<v.size()-1<<" argument(s), but "<<explist.size()<<" passed."<<endl;
			}
			else{
				bool mismatch=false;
				for(int i=0;i<explist.size();i++){
					if(compatible_assign_types(v[i+1],explist[i]->type)==NULL){
						if(same(explist[i]->type,v[i+1])) continue;
						if(extra_compatible(explist[i]->type,v[i+1])){
							cerr<<"Warning line "<<line<<": Pointer passed instead of array"<<endl;
							continue;
						}
						mismatch=true;
						cerr<<"Error at line "<<line<<": expected \'";
						v[i+1]->print_cerr();
						cerr<<"\' but argument is of type \'";
						explist[i]->type->print_cerr();
						cerr<<"\'\n";
					}
				}
				if(!mismatch)temp=v[0];
			}
		}
		$$=$3;
        ((Funcall*)$$)->setName(tmp);
		$$->type=temp;
		$$->lvalue=false;
    }
    | postfix_expression '[' expression ']'{
		Type *tmp=$1->type;
		Type *temp=new Type(BasicTypes::Invalid,NULL,-1,-1);
		if(tmp==NULL){
			cerr<<"Error at line "<<line<<": subscripted value is not an array\n";
		}
		else if(tmp!=NULL && tmp->basic!=BasicTypes::Array){
			cerr<<"Error at line "<<line<<": subscripted value is not an array\n";
		}
		else{
			temp=tmp->subtype;
		}
		if($3->type!=NULL){
			if($3->type->basic!=BasicTypes::Int&&$3->type->basic!=BasicTypes::IntConstant){
				cerr<<"Error at line "<<line<<": array subscript is not an integer\n";
				temp=new Type(BasicTypes::Invalid,NULL,-1,-1);
			}
		}
		else{
			cerr<<"Error at line "<<line<<": array subscript is not an integer\n";
			temp=new Type(BasicTypes::Invalid,NULL,-1,-1);
		}
		bool lvalue=$1->lvalue;
		$$=new ArrayRef((RefAst*)$1,$3);
		$$->type=temp;
		$$->lvalue=lvalue;
    }

    | postfix_expression '.' IDENTIFIER{
        Identifier* tmp=new Identifier($3);
		Type* temp= new Type(BasicTypes::Invalid,NULL,-1,-1);
		if($1->type->basic!=BasicTypes::Invalid){
			if($1->type->basic!=BasicTypes::Struct){
				cerr<<"Error at line "<<line<<": request for member \'"<<$3<<"\' in something not a structure or union\n";
			}
			else if($1->type->basic==BasicTypes::Struct){
				string structname=$1->type->structname;
				if(!structtable->is_attr($3,structname)){
					cerr<<"Error at line "<<line<<": \'struct "<<structname<<"\' has no member named \'"<<$3<<"\'\n";
				}
				else{
					temp=structtable->get_attr($3,structname);
				}
			}
		}
		bool lvalue=$1->lvalue;
        $$=new Member($1,tmp);
		$$->type=temp;
		$$->lvalue=lvalue;
    }
    | postfix_expression PTR_OP IDENTIFIER{
        Identifier* tmp=new Identifier($3);
		Type* temp= new Type(BasicTypes::Invalid,NULL,-1,-1);
		if($1->type->basic!=BasicTypes::Invalid){
			if($1->type->basic!=BasicTypes::Pointer){
				cerr<<"Error at line "<<line<<": invalid type argument of \'->\'\n";
			}
			else if($1->type->basic==BasicTypes::Pointer&& ($1->type->subtype->subtype!=NULL || $1->type->subtype->basic!=BasicTypes::Struct)){
				cerr<<"Error at line "<<line<<": request for member \'"<<$3<<"\' in something not a structure or union\n";
			}
			else if($1->type->basic==BasicTypes::Pointer&& $1->type->subtype->subtype==NULL && $1->type->subtype->basic==BasicTypes::Struct){
				string structname=$1->type->subtype->structname;
				if(!structtable->is_attr($3,structname)){
					cerr<<"Error at line "<<line<<": \'struct "<<structname<<"\' has no member named \'"<<$3<<"\'\n";
				}
				else{
					temp=structtable->get_attr($3,structname);
				}
			}
		}
		bool lvalue=$1->lvalue;
        $$=new Arrow($1,tmp);
		$$->type=temp;
		$$->lvalue=lvalue;
    }
	| postfix_expression INC_OP{
		Type *tmp=new Type(BasicTypes::Invalid,NULL,-1,-1);
		if($$->lvalue==true){
			if($1->type->basic!=BasicTypes::Invalid){
				if(($1->type->basic==BasicTypes::Int||$1->type->basic==BasicTypes::Float)&&$1->type->subtype==NULL){
					tmp=$1->type;
				}
				else if($1->type->basic==BasicTypes::Pointer){
					tmp=$1->type;
				}
			}
		}
		else{
			cerr<<"Error at line "<<line<<": lvalue required as increment operand\n";
		}
        $$=new Unary_op("PP",$1);
		$$->type=tmp;
		$$->lvalue=false;
    }
	;

expression_list:
    expression{
        ExpAst* tmp=$1;
        $$=new Funcall();
        ((Funcall*)$$)->insertExp(tmp);
    }
    | expression_list ',' expression{
        $$=$1;
        ExpAst* tmp=$3;
        ((Funcall*)$$)->insertExp(tmp);
    }
	;

unary_operator:
    '-' {	$$="Uminus"; }
	| '!' {$$="Not"; }
    | '&' { $$="Pointer"; }
    | '*' { $$="Deref"; }
	;

selection_statement:
    IF '(' expression ')' statement ELSE statement{
        $$=new If($3,$5,$7);
    }
	;

iteration_statement:
    WHILE '(' expression ')' statement{
        $$=new While($3,$5);
    }
	| FOR '(' expression ';' expression ';' expression ')' statement{  //modified this production
        $$=new For($3,$5,$7,$9);
    }
    ;

declaration_list:
    declaration
    | declaration_list declaration
	;

declaration:
    type_specifier declarator_list ';'
	;

declarator_list:
    declarator{
		bool check_dup=CurrSymTab->Check_dup($1);
		bool check_dup_in_struct=structtable->Check_dup($1);
        if(is_func_not_struct){
			if(!check_dup)
				cerr<<"Error at line "<<line<<" : variable \'"<<$1->symbol<<"\' redefined\n";
        	else{
    			$1->offset=curr_offset;
    			CurrSymTab->AddEntry($1,line);
    			curr_offset-=$1->size;
    		}
		}
		else if(!is_func_not_struct && !struct_redef){
			if(!check_dup_in_struct)
				cerr<<"Error at line "<<line<<" : variable \'"<<$1->symbol<<"\' redefined\n";
        	else{
				if($1->type->basic==BasicTypes::Struct && $1->type->structname==current_struct_name){
					cerr<<"Error at line "<<line<<" : field \'"<<$1->symbol<<"\' has incomplete type\n";
				}
				else{
    				$1->offset=curr_offset;
    				structtable->AddEntry($1,line);
    				curr_offset+=$1->type->size;
				}
    		}
		}
    }
	| declarator_list ',' declarator {
		bool check_dup=CurrSymTab->Check_dup($3);
		bool check_dup_in_struct=structtable->Check_dup($3);
        if(is_func_not_struct){
			if(!check_dup)
				cerr<<"Error at line "<<line<<" : variable \'"<<$3->symbol<<"\' redefined\n";
        	else{
    			$3->offset=curr_offset;
    			CurrSymTab->AddEntry($3,line);
    			curr_offset-=$3->size;
    		}
		}
		else if(!is_func_not_struct){
			if(!check_dup_in_struct)
				cerr<<"Error at line "<<line<<" : variable \'"<<$3->symbol<<"\' redefined\n";
        	else{
				if($3->type->basic==BasicTypes::Struct && $3->type->structname==current_struct_name){
					cerr<<"Error at line "<<line<<" : field \'"<<$3->symbol<<"\' has incomplete type\n";
				}
				else{
    				$3->offset=curr_offset;
    				structtable->AddEntry($3,line);
    				curr_offset+=$3->type->size;
				}
    		}
		}
    }
 	;
