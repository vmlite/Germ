/* Copyright (c) 2017 Tohack<tohack@foxmail.com>. All Rights Reserved. */

#include "germ.h"

static token* newToken(int tokencode){
	token* ret;

	ret = malloc(sizeof(token));
	if ( !ret ){
		printf("Error: alloc token return null");
		exit(-1);
	}
	ret->tCode = tokencode;
	ret->tNext = null;
	return ret;
}

static bool isId(char ch){
	return (ch =='_' || (ch >= 'a' && ch <= 'z') 
			 || (ch >= 'A' && ch <= 'Z'));
}

static bool isId2(char ch){
	return (ch =='_' || (ch >= 'a' && ch <= 'z') 
			 || (ch >= 'A' && ch <= 'Z')
			 || (ch >= '0' && ch <= '9'));
}

static bool isNumber(char ch){
	return (ch >='1' && ch <= '9');
}

static bool isNumber2(char ch){
	return (ch >='0' && ch <= '9');
}

static uint char_escape(char **str){
	uint tmp;
	char *p =*str;
	switch(*p){
	    case 'a':
		*str = ++p;
		return '\a';
	    case 'b':
		*str = ++p;
		return '\b';
	    case 'f':
		*str = ++p;
		return '\f';
	    case 'n':
		*str = ++p;
		return '\n';
	    case 'r':
		*str = ++p;
		return '\r';
	    case 't':
		*str = ++p;
		return '\t';
	    case 'v':
		*str = p++;
		return '\v';
	    case '\\':
		*str = p++;
		return '\\';
	    case '\'':
		*str = p++;
		return '\'';
	    case '"':
		*str = p++;
		return '"';
	    case '?':
		*str = p++;
		return '\?';
	    case '0':
		if( !(*(p+1)>='1' && *(p+1)<='9') ){
			*str= p;
			return '\0';
		}
	    case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7':
		tmp = *p - '0';
		p++;
		if( *p >='0' && *p<='7'){
			tmp= tmp * 8 + *p - '0';
			p++;
		}
		if( *p >='0' && *p<='7'){
			tmp= tmp * 8 + *p - '0';
			p++;
		}
		*str = p;
		return tmp;
	    case 'x':
		p++;
		// 处理第一个字符.
		if( *p >='0' && *p <='9'){
			tmp = *p - '9';
		} else if( *p >= 'A' && *p <='F'){
			tmp = *p - 'A' +10;
		}else if( *p >= 'a' && *p <='f'){
			tmp = *p - 'a' +10;
		} else {
			*str =p;
			return 'x';
		}
		p++;
		// 处理第二个字符.
		if( *p >='0' && *p <='9'){
			tmp = tmp * 16 + *p - '9';
		} else if( *p >= 'A' && *p <='F'){
			tmp = tmp * 16 + *p - 'A' +10;
		}else if( *p >= 'a' && *p <='f'){
			tmp = tmp * 16 + *p - 'a' +10;
		} else {
			*str =++p;
			return tmp;
		}
		*str = ++p;
		return tmp;
	    default:
		*str = ++p;
		return *(p-1);
	}
}

struct kwCodeWithString{
	uint  code;
	char* str;
};
static struct kwCodeWithString kwStrMap[]={
	{ TKw_package,  "package" },
	{ TKw_import,   "import"  },
	{ TKw_as,       "as"      },
	{ TKw_bool,     "bool"    },
	{ TKw_int,      "int"     },
	{ TKw_uint,     "uint"    },
	{ TKw_int8,     "int8"    },
	{ TKw_uint8,    "uint8"   },
	{ TKw_int16,    "int16"   },
	{ TKw_uint16,   "uint16"  },
	{ TKw_int32,    "int32"   },
	{ TKw_uint32,   "uint32"  },
	{ TKw_int64,    "int64"   },
	{ TKw_uint64,   "uint64"  },
	{ TKw_float,    "float"   },
	{ TKw_float32,  "float32" },
	{ TKw_float64,  "float64" },
	{ TKw_double,   "double"  },
	{ TKw_char,     "char"    },
	{ TKw_void,     "void"    },
	{ TKw_func,     "func"    },
	{ TKw_typedef,  "typedef" },
	{ TKw_struct,   "struct"  },
	{ TKw_union,    "union"   },
	{ Tkw_true,     "true"    },
	{ TKw_false,    "false"   },
	{ TKw_null,     "null"    },
	{ TKw_if,       "if"      },
	{ IKw_else,     "else"    },
	{ TKw_switch,   "switch"  },
	{ TKw_case,     "case"    },
	{ TKw_default,  "default" },
	{ TKw_for,      "for"     },
	{ TKw_while,    "while"   },
	{ TKw_do,       "do"      },
	{ TKw_continue, "continue"},
	{ TKw_break,    "break"   },
	{ TKw_throw,    "throw"   },
	{ TKw_return,   "return"  },
	{ TKw_goto,     "goto"    },
	{ TKw_const,    "const"   },
	{ 0, NULL}
};

static token* recognizeKeyWord(token* t){
	struct kwCodeWithString *map = kwStrMap;
	while( map->code){
		if(strcmp(t->tValue.s->data, map->str) == 0){
			t->tCode = map->code;
			deleteDynstr(t->tValue.s);
			return t;
		}
		map++;
	}
	return t;
}

token* lexical(Buffer* buf, char** cur){
	dynstr *ds;
	token* ret;
	char *p = *cur;

    repeat:
	// skip white chars first.
	while( *p == ' ' || *p== '\t' || *p== '\r' || *p== '\n'){
		p++;
	}
	// deal with id and keywords.
	if(isId(*p)){
		ret = newToken(TokenId);
		ds = newDynstr();
		while(isId2(*p)){
			ds = appendChar(ds, *p);
			p++;
		}
		ds = appendChar(ds,'\0');
		*cur = p;
		ret->tValue.s = ds;
		return recognizeKeyWord(ret);
	}
	// deal with number.
	if(isNumber(*p)){
		ret = newToken(TokenInteger);
		ret->tValue.i= *p - '0';
		p++;
		while(isNumber2(*p)){
			ret->tValue.i = ret->tValue.i * 10 + *p - '0';
			p++;
		}
		*cur = p;
		return ret;
	}
	// deal with other chars.
	switch(*p){
	    case '{' : case '}' : case '[' : case ']' :
	    case '(' : case ')' : case ';' : case ':' :
	    case ',' :
		*cur =p +1;
		return newToken(*p);
	    case '.' :
		if( *(p+1) =='.' && *(p+2) == '.'){
			*cur = p+3;
			return newToken(TokenThreeDot);
		}else{
			*cur = p+1;
			return newToken(TokenDot);
		}
	    case '+' :
		if( *(p+1) == '+'){
			*cur = p+2;
			return newToken(TOp_inc);
		}else if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_addAssign);
		}else{
			*cur = p+1;
			return newToken(TOp_add);
		}
	    case '-' :
		if( *(p+1) == '-'){
			*cur = p+2;
			return newToken(TOp_dec);
		}else if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_subAssign);
		}else if( *(p+1) == '>'){
			*cur = p+2;
			return newToken(TOp_ra);
		}else{
			*cur = p+1;
			return newToken(TOp_sub);
		}
	    case '*' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_mulAssign);
		}else{
			*cur = p+1;
			return newToken(TokenStar);
		}
	     case '/' :
		if( *(p+1) == '/'){		// skip line comment.
			while(*p != '\n'){
				p++;
			}
			goto repeat;
		} else if(*(p+1) =='*'){	// skip multline comment.
			p++;
			do{
				p++;
				if(!*p){
					printf("Error: lexical multline comment not close");
					return newToken(TokenEnd);
				}
			}while(!(*p == '*' && *(p+1) == '/'));
			p++;p++;
			goto repeat;
		} else if( *(p+1) == '='){
			*cur = p+1;
			return newToken(TOp_divAssign);
		} else {
			*cur = p+1;
			return newToken(TOp_div);
		}
	    case '%' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_remAssign);
		}else{
			*cur = p+1;
			return newToken(TOp_rem);
		}
	    case '!' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_notEq);
		}else{
			*cur = p+1;
			return newToken(TOp_not);
		}
	    case '&' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_andAssign);
		}else if ( *(p+1) == '&'){
			*cur = p+2;
			return newToken(TOp_andAnd);
		} else {
			*cur = p+1;
			return newToken(TOp_and);
		}
	    case '=' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_eq);
		}else{
			*cur = p+1;
			return newToken(TOp_assign);
		}
	    case '|' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_orAssign);
		}else if ( *(p+1) == '|'){
			*cur = p+2;
			return newToken(TOp_orOr);
		} else {
			*cur = p+1;
			return newToken(TOp_or);
		}
	    case '~' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_negAssign);
		}else{
			*cur = p+1;
			return newToken(TOp_neg);
		}
	    case '>' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_gtEq);
		}else if ( *(p+1) == '>'){
			if( *(p+2) == '=' ){
				*cur = p+3;
				return newToken(TOp_shrAssign);
			} else{
				*cur = p+2;
				return newToken(TOp_shr);
			}
		}else{
			*cur = p+1;
			return newToken(TOp_gt);
		}
	    case '<' :
		if( *(p+1) == '='){
			*cur = p+2;
			return newToken(TOp_ltEq);
		}else if ( *(p+1) == '<'){
			if( *(p+2) == '=' ){
				*cur = p+3;
				return newToken(TOp_shlAssign);
			} else{
				*cur = p+2;
				return newToken(TOp_shl);
			}
		}else{
			*cur = p+1;
			return newToken(TOp_lt);
		}
	    case '\'':
		ret =newToken(TokenChar);
		p++;
		if(*p == '\\'){
			p++;
			ret->tValue.i=char_escape(&p);
		} else if( *p == '\''){
			printf("Error: Not have availbe char");
			ret->tValue.i= 0 ; // 空字符就设置成0.
			*cur = ++p;
			return ret;
		} else{
			ret->tValue.i=*p;
			p++;
		}
		// 处理关闭.
		if(*p == '\''){
			*cur = ++p;
			return ret;
		}else{
			printf("Error: have more chars in char:%c\n", *p);
			// skip the more chars.
			while(*p != '\''){
				p++;
			}
			*cur = ++p;
			return ret;
		}
	    case '"' :
		ret = newToken(TokenString);
		ds  = newDynstr();
		++p;
		while(*p && (*p !='"')){
			if(*p == '\\'){
				++p;
				ds = appendChar(ds, (char)char_escape(&p));
			} else{
				ds = appendChar(ds, *p);
				++p;
			}
		}
		ds = appendChar(ds, '\0');
		ret->tValue.s = ds;
		
		if( *p =='"'){
			*cur = ++p; // skip the close ".
			return ret;
		}
		printf("Error: lexical sting not close");
		*cur = p; // skip the close ".
		return ret;
	    case '\0':
		*cur = ++p; 
		return newToken(TokenEnd);
	    default:
		printf("Error: lexical can't recognize char:%d(%c)\n", *p, *p);
		++p;
		goto repeat;
	}
}

// scan the buffer, return the token list.
token* doScan(Buffer* buf){
	token* head = newToken(TokenStart);
	token* tail = head;
	char* p = buf->data;

	tail->tPosFile = buf->filename;

	do{
		tail->tNext = lexical(buf, &p);
		tail->tNext->tPosFile = buf->filename;
		tail = tail->tNext;
	}while(tail->tCode != TokenEnd);

	return head;
}

token* scanFile(char* filename){
	Buffer* buf;
	token* tokenlist;

	buf = readFileToBuffer(filename);
	tokenlist = doScan(buf);

	deleteBuffer(buf);

	return tokenlist;
}

static void printToken(token* t){
	printf("TPosFile: %s\t", t->tPosFile);
	if(t->tCode >= TKw_package && t->tCode <= TKw_const){
		printf("KeyWord:%s\n", kwStrMap[t->tCode -TKw_package].str);
		return;
	}
	if(t->tCode == TokenId){
		printf("Id:%s\n", t->tValue.s->data);
		return;
	}
	if(t->tCode == TokenChar){
		printf("Char:%d(%c)\n", t->tValue.i,t->tValue.i);
		return;
	}
	if(t->tCode == TokenString){
		printf("String:%s\n", t->tValue.s->data);
		return;
	}
	if(t->tCode == TokenInteger){
		printf("Number:%d\n", t->tValue.i);
		return;
	}
	printf("TCode:%d\n", t->tCode);
}

void printTokenList(token* t){
	token* tmp=t;
	printf("----------Debug Token-------------\n");
	while(tmp->tCode != TokenEnd){
		printToken(tmp);
		tmp = tmp->tNext;
	}
	printf("----------------------------------\n");
}