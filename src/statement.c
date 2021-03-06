/* Copyright (c) 2017 Tohack<tohack@foxmail.com>. All Rights Reserved. */

#include "main.h"

Statement* _newStatement(int type){
	Statement* ret = Xmalloc(sizeof(Statement), __FILE__, __LINE__);
	ret->stmtType = type;
	return ret;
}

// 所以的检查都由调用函数处理。
Statement* ParseLabelStmt(ParseState* ps, String* label){
	Statement* ret = _newStatement(Stmt_lable);
	ret->labelString = label;
	eatToken(ps);
	return ret;
}

Statement* ParseExpressionStmt(ParseState* ps){
	Statement* ret = _newStatement(Stmt_expression);
	ret->expression = ParseExpression(ps);
	if(ParseExceptToken(ps, ';')){
		return ret;
	}
	ParseFatal(ps, ";");
	return NULL; //[-Wreturn-type]
}

Statement* ParseIfStmt(ParseState* ps){
	CondElement* ce;
	Statement* ret = _newStatement(Stmt_if);

	if(!ParseExceptToken(ps, Token_if)){
		Debug(__FILE__, __LINE__, "not have if");
		exit(-1);
	}

    repeat:
	ce = Xmalloc(sizeof(CondElement), __FILE__, __LINE__);
	ParseMatchToken(ps, '(');
	ce->expression = ParseExpression(ps);
	ParseMatchToken(ps, ')');
	ParseMatchToken(ps, '{');
	while(!ParseExceptToken(ps,'}')){
		//VectorPush(&ce->statement, ParseInternalStmt(ps, NULL));
	}
	VectorPush(&ret->ifCondElements, ce);

	if(ParseExceptToken(ps, Token_elif)){
		goto repeat;
	}

	if(!ParseExceptToken(ps, Token_else)){
		return ret;
	}
	ParseMatchToken(ps, '{');
	while(!ParseExceptToken(ps, '}')){
		//VectorPush(&ret->elseStmt, ParseInternalStmt(ps));
	}
	return ret;
}

Statement* ParseForStmt(ParseState* ps){
	Statement* ret = _newStatement(Stmt_for);

	ParseMatchToken(ps, Token_if);
	ParseMatchToken(ps, '(');
	if(!ParseExceptToken(ps, ';')){
		ret->forExpr1 = ParseExpression(ps);
		ParseMatchToken(ps, ';');
	}
	if(!ParseExceptToken(ps, ';')){
		ret->forExpr2 = ParseExpression(ps);
		ParseMatchToken(ps, ';');
	}
	if(!ParseExceptToken(ps, ';')){
		ret->forExpr3 = ParseExpression(ps);
		ParseMatchToken(ps, ';');
	}
	ParseMatchToken(ps, ')');
	ParseMatchToken(ps, '{');
	while(!ParseExceptToken(ps, '}')){
		//VectorPush(&ret->forStmt, ParseInternalStmt(ps));
	}
	return ret;
}

void ParseInternalStmt(ParseState* ps, Symbol* func){
	Dtype* dt;
	//String* name;

	eatToken(ps);
	return;

	dt = ParseDtype(ps);
	if(dt){
		ParseInternalDeclare(ps, dt, &func->funcLocal);
	} else{
		eatToken(ps);
	}
}

void ParseExternalStmt(ParseState* ps, Symbol* module){
	Statement* stmt = _newStatement(Stmt_declare);
	Symbol* decl = ParseExternalDeclare(ps);

	stmt->declSymbol = decl;
	VectorPush(&module->modStmts, stmt);

	if(decl->sType == ST_Function ){
		VectorPush(&module->modSymbols, decl);
	}
}