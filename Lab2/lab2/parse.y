%scanner Scanner.h
%scanner-token-function d_scanner.lex()

%token VOID INT FLOAT FLOAT_CONSTANT INT_CONSTANT AND_OP OR_OP STRUCT PTR_OP OTHERS
%token EQ_OP NE_OP LE_OP GE_OP STRING_LITERAL IF ELSE WHILE FOR RETURN IDENTIFIER INC_OP

%polymorphic eAst : ExpAst* ; sAst : StmtAst*; rAst : RefAst*; Int : int; Float : float; String : string;

%type <eAst> expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression primary_expression postfix_expression logical_and_expression constant_expression expression_list
%type <sAst> translation_unit function_definition compound_statement statement_list statement selection_statement iteration_statement assignment_statement
%type <rAst> l_expression
%type <Int> INT_CONSTANT parameter_list
%type <Float> FLOAT_CONSTANT
%type <String> STRING_LITERAL IDENTIFIER unary_operator

%%

translation_unit:
    struct_specifier
    | function_definition{
        $1->print();
    }
    | translation_unit function_definition{
        $$=$1;
        $2->print();
    }
    | translation_unit struct_specifier{
        $$=$1;
    };

struct_specifier:
    STRUCT IDENTIFIER '{' declaration_list '}' ';';

function_definition:
    type_specifier fun_declarator compound_statement{
        $$=$3;
    };

type_specifier:
    base_type
    | type_specifier '*';

base_type:
    VOID
    | INT
	| FLOAT
    | STRUCT IDENTIFIER;

fun_declarator:
    IDENTIFIER '(' parameter_list ')'
	| IDENTIFIER '(' ')'
	;

parameter_list:
    parameter_declaration
	| parameter_list ',' parameter_declaration;

parameter_declaration:
    type_specifier declarator;

declarator:
    IDENTIFIER
	| declarator '[' constant_expression ']'
    ;

constant_expression:
    INT_CONSTANT{
        $$ = new IntConst($1);
    }
    | FLOAT_CONSTANT{
        $$ = new FloatConst($1);
    };

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
        $$=new Return($2);
    }
    ;

assignment_statement:
    ';'{
        $$=new Empty();
    }
	|  l_expression '=' expression ';'{
        $$=new Ass($1,$3);
    }
	;

expression:
    logical_and_expression{
        $$=$1;
    }
    | expression OR_OP logical_and_expression{
        $$=new Binary_op("Or",$1,$3);
    }
	;
logical_and_expression:
    equality_expression{
        $$=$1;
    }
    | logical_and_expression AND_OP equality_expression{
        $$=new Binary_op("And",$1,$3);
    }
	;

equality_expression:
    relational_expression{
        $$=$1;
    }
    | equality_expression EQ_OP relational_expression{
        $$=new Binary_op("EQ",$1,$3);
    }
	| equality_expression NE_OP relational_expression{
        $$=new Binary_op("NE",$1,$3);
    }
	;
relational_expression:
    additive_expression{
        $$=$1;
    }
    | relational_expression '<' additive_expression{
        $$=new Binary_op("LT",$1,$3);
    }
	| relational_expression '>' additive_expression{
        $$=new Binary_op("GT",$1,$3);
    }
	| relational_expression LE_OP additive_expression{
        $$=new Binary_op("LE",$1,$3);
    }
    | relational_expression GE_OP additive_expression{
        $$=new Binary_op("GE",$1,$3);
    }
	;

additive_expression:
    multiplicative_expression{
        $$=$1;
    }
	| additive_expression '+' multiplicative_expression{
        $$=new Binary_op("Plus",$1,$3);
    }
	| additive_expression '-' multiplicative_expression{
        $$=new Binary_op("Minus",$1,$3);
    }
	;

multiplicative_expression:
    unary_expression{
        $$=$1;
    }
	| multiplicative_expression '*' unary_expression{
        $$=new Binary_op("Mult",$1,$3);
    }
	| multiplicative_expression '/' unary_expression{
        $$=new Binary_op("Div",$1,$3);
    }
	;
unary_expression:
    postfix_expression{
        $$=$1;
    }
	| unary_operator postfix_expression{
        $$=new Unary_op($1,$2);
    }
	;

postfix_expression:
    primary_expression{
        $$=$1;
    }

    | IDENTIFIER '(' ')'{
        $$=new Funcall(new Identifier($1));
    }
	| IDENTIFIER '(' expression_list ')'{
        Identifier* tmp= new Identifier($1);
        $$=$3;
        ((Funcall*)$$)->setName(tmp);
    }
	| l_expression INC_OP{
        $$=new Unary_op("PP",$1);
    }
	;

primary_expression:
    l_expression{
        $$=$1;
    }
    | l_expression '=' expression{
        $$=new Binary_op("Ass",$1,$3);
    }
    | INT_CONSTANT{
        $$=new IntConst($1);
    }
	| FLOAT_CONSTANT{
        $$=new FloatConst($1);
    }
    | STRING_LITERAL{
        $$=new StringConst($1);
    }
	| '(' expression ')'{
        $$=$2;
    }
    |'&' l_expression {// & and * are similar
        $$=new Pointer($2);
    }
	;

l_expression:
    IDENTIFIER{
        $$=new Identifier($1);
    }
    | l_expression '[' expression ']'{
        $$=new ArrayRef($1,$3);
    }
    | '*' l_expression{
        $$=new Deref($2);
    }
    | l_expression '.' IDENTIFIER{
        Identifier* tmp=new Identifier($3);
        $$=new Member($1,tmp);
    }
    | l_expression PTR_OP IDENTIFIER{
        Identifier* tmp=new Identifier($3);
        $$=new Arrow($1,tmp);
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
    '-'{
        $$="Uminus";
    }
	| '!'{
        $$="Not";
    }
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
    type_specifier declarator_list';'
	;

declarator_list:
    declarator
	| declarator_list ',' declarator
 	;
