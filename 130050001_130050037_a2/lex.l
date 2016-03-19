
// There may be some flexer directives here

%%
[ \t]                                   											 //skip
\n																					{ Parser::line++; }
"void"																				return Parser::VOID;
"struct"																			return Parser::STRUCT;
"->"																			    return Parser::PTR_OP;
"int"																				return Parser::INT;
"float"																				return Parser::FLOAT;
"if" 																				return Parser::IF;
"else" 																				return Parser::ELSE;
"while" 																			return Parser::WHILE;
"for" 																				return Parser::FOR;
"return" 																			return Parser::RETURN;
"\""((([\\][nt"\\])?)[^"\\]*)*"\""                                                  {Parser::d_val__ = std::string (matched());   return Parser::STRING_LITERAL;}
(([1-9][0-9]*)|(0[bB][01]*)|(0[0-7]*)|(0[xX][0-9a-fA-F]*)) 						    {
                                                                                        std::string tmp= std::string(matched());
                                                                                        Parser::d_val__=0;
                                                                                        if(tmp[0]!='0'||(tmp[0]=='0'&&tmp.length()==1)){
                                                                                            const char *hexstring = tmp.c_str();
                                                                                            int number = (int)strtol(hexstring, NULL, 10);
                                                                                            Parser::d_val__=number;
                                                                                        }
                                                                                        else if(tmp[0]=='0'&&(tmp[1]=='x'||tmp[1]=='X')){
                                                                                            tmp=tmp.substr(2);
                                                                                            const char *hexstring = tmp.c_str();
                                                                                            int number = (int)strtol(hexstring, NULL, 16);
                                                                                            Parser::d_val__=number;
                                                                                        }
                                                                                        else if(tmp[0]=='0'&&(tmp[1]=='b'||tmp[1]=='B')){
                                                                                            tmp=tmp.substr(2);
                                                                                            const char *binstring = tmp.c_str();
                                                                                            int number = (int)strtol(binstring, NULL, 2);
                                                                                            Parser::d_val__=number;
                                                                                        }
                                                                                        else if(tmp[0]=='0'){
                                                                                            tmp=tmp.substr(1);
                                                                                            const char *octstring = tmp.c_str();
                                                                                            int number = (int)strtol(octstring, NULL, 8);
                                                                                            Parser::d_val__=number;
                                                                                        }
                                                                                        return Parser::INT_CONSTANT;
                                                                                    }

((([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))((([eE][+-][0-9]+)|([eE][0-9]+))?)) 			{
                                                                                        std::string tmp=std::string(matched());
                                                                                        float float_tmp=stof(tmp);
                                                                                        Parser::d_val__=float_tmp;
                                                                                        return Parser::FLOAT_CONSTANT;
                                                                                    }
[[:alpha:]][[:alnum:]]*                     										{Parser::d_val__ = std::string (matched());   return Parser::IDENTIFIER;}
"&&" 																				return Parser::AND_OP;
"||" 																				return Parser::OR_OP;
"==" 																				return Parser::EQ_OP;
"!=" 																				return Parser::NE_OP;
"<=" 																				return Parser::LE_OP;
">=" 																				return Parser::GE_OP;
"++" 																				return Parser::INC_OP;
"+"                                         										return '+';
"."                                         										return '.';
"-"                                         										return '-';
"*"                                         										return '*';
"/"                                         										return '/';
"="                                         										return '=';
","                                         										return ',';
"{"                                         										return '{';
"}"                                         										return '}';
"["                                         										return '[';
"]"                                         										return ']';
"("                                         										return '(';
")"                                         										return ')';
";"                                         										return ';';
"<"																					return '<';
">"																					return '>';
"&"																					return '&';
"|"																					return '|';
"^"																					return '^';
"!"																					return '!';
.                                           										return Parser::OTHERS;
