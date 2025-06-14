
/*
Written by Antoine Savine in 2018

This code is the strict IP of Antoine Savine

License to use and alter this code for personal and commercial applications
is freely granted to any person or company who purchased a copy of the book

Modern Computational Finance: Scripting for Derivatives and XVA
Jesper Andreasen & Antoine Savine
Wiley, 2018

As long as this comment is preserved at the top of the file
*/

#pragma once

#include <string>
#include <vector>

using namespace std;

#include <regex>
#include <algorithm>

#include "visitorHeaders.h"

Event parse( const string& eventString);
vector<string> tokenize( const string& str);

struct script_error : public runtime_error
{
	script_error( const char msg[]) : runtime_error( msg) {}
};

template <class TokIt>
class Parser
{
	//	Helpers

	//	Find matching closing char, for example matching ) for a (, skipping through nested pairs
	//		does not change the cur iterator, assumed to be on the opening, 
	//		and returns an iterator on the closing match
	template <char OpChar, char ClChar>
	static TokIt findMatch( TokIt cur, const TokIt end)	
	{
		unsigned opens = 1;

		++cur;
		while( cur != end && opens > 0)
		{
			opens += ((*cur)[0] == OpChar) - ((*cur)[0] == ClChar);
			++cur;
		}

		if( cur == end && opens > 0)
			throw script_error( (string( "Opening ") + OpChar + " has not matching closing " + ClChar).c_str());

		return --cur;
	}

	//	Parentheses

	typedef Expression (*ParseFunc)( TokIt&, const TokIt);

	template <ParseFunc FuncOnMatch, ParseFunc FuncOnNoMatch>
	static Expression parseParentheses( TokIt& cur, const TokIt end)
	{	
        Expression tree;

		//	Do we have an opening '('?
		if( *cur == "(")
		{
			//	Find match
			TokIt closeIt = findMatch<'(',')'>( cur, end);

			//	Parse the parenthesed condition/expression, including nested parentheses, 
			//		by recursively calling the parent parseCond/parseExpr
			tree = FuncOnMatch( ++cur, closeIt);

			//	Advance cur after matching )
			cur = ++closeIt;
		}
		else
		{
			//	No (, so leftmost we move one level up
			tree = FuncOnNoMatch( cur, end);
		}

		return tree;
	}

	//	Expressions

	//	Parent, Level1, '+' and '-'
	static Expression parseExpr( TokIt& cur, const TokIt end)
	{
		//	First exhaust all L2 ('*' and '/') and above expressions on the lhs
		auto lhs = parseExprL2( cur, end);

		//	Do we have a match?
		while( cur != end && ((*cur)[0] == '+' || (*cur)[0] == '-'))
		{
			//	Record operator and advance
			char op = (*cur)[0];
			++cur;
			
			//	Should not stop straight after operator
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Exhaust all L2 ('*' and '/') and above expressions on the rhs
			auto rhs = parseExprL2( cur, end);

			//	Build node and assign lhs and rhs as its arguments, store in lhs
			lhs = op == '+'? make_base_binary<NodeAdd>( lhs, rhs) : make_base_binary<NodeSub>( lhs, rhs);
		}

		//	No more match, return lhs
		return lhs;
	}

	//	Level2, '*' and '/'
	static Expression parseExprL2( TokIt& cur, const TokIt end)
	{
		//	First exhaust all L3 ('^') and above expressions on the lhs
		auto lhs = parseExprL3( cur, end);

		//	Do we have a match?
		while( cur != end && ((*cur)[0] == '*' || (*cur)[0] == '/'))
		{
			//	Record operator and advance
			char op = (*cur)[0];
			++cur;
			
			//	Should not stop straight after operator
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Exhaust all L3 ('^') and above expressions on the rhs
			auto rhs = parseExprL3( cur, end);

			//	Build node and assign lhs and rhs as its arguments, store in lhs
			lhs = op == '*'? make_base_binary<NodeMult>( lhs, rhs) : make_base_binary<NodeDiv>( lhs, rhs);
		}

		//	No more match, return lhs
		return lhs;
	}

	//	Level3, '^'
	static Expression parseExprL3( TokIt& cur, const TokIt end)
	{
		//	First exhaust all L4 (unaries) and above expressions on the lhs
		auto lhs = parseExprL4( cur, end);

		//	Do we have a match?
		while( cur != end && (*cur)[0] == '^')
		{
			//	Advance
			++cur;
			
			//	Should not stop straight after operator
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Exhaust all L4 (unaries) and above expressions on the rhs
			auto rhs = parseExprL4( cur, end);

			//	Build node and assign lhs and rhs as its arguments, store in lhs
			lhs = make_base_binary<NodePow>( lhs, rhs);
		}

		//	No more match, return lhs
		return lhs;
	}

	//	Level 4, unaries
	static Expression parseExprL4( TokIt& cur, const TokIt end)
	{		
		//	Here we check for a match first
		if( cur != end && ((*cur)[0] == '+' || (*cur)[0] == '-'))	
		{
			//	Record operator and advance
			char op = (*cur)[0];
			++cur;
			
			//	Should not stop straight after operator
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Parse rhs, call recursively to support multiple unaries in a row
			auto rhs = parseExprL4( cur, end);

			//	Build node and assign rhs as its (only) argument
			auto top = op == '+'? make_base_node<NodeUplus>(): make_base_node<NodeUminus>();
			top->arguments.resize( 1);
			//	Take ownership of rhs
			top->arguments[0] = move( rhs);

			//	Return the top node
			return top;
		}

		//	No more match, we pass on to the L5 (parentheses) parser
		return parseParentheses<parseExpr,parseVarConstFunc>( cur, end);
	}

	//	Level 6, variables, constants, functions
	static Expression parseVarConstFunc( TokIt& cur, const TokIt end)
	{
		//	First check for constants, if the char is a digit or a dot, then we have a number
		if( (*cur)[0] == '.' || ((*cur)[0] >= '0' && (*cur)[0] <= '9'))
		{
			return parseConst( cur);
		}
                //      Lists
                if( *cur == "[")
                {
                        return parseList(cur, end);
                }


		//	Check for functions, including those for accessing simulated data
        Expression top;
		unsigned minArg, maxArg;
		if( *cur == "SPOT")
		{
			top = make_base_node<NodeSpot>();
			minArg = maxArg = 0;
		}
		else if( *cur == "LOG")
		{
			top = make_base_node<NodeLog>();
			minArg = maxArg = 1;
		}
		else if( *cur == "SQRT")
		{
			top = make_base_node<NodeSqrt>();
			minArg = maxArg = 1;
		}
		else if( *cur == "MIN")
		{
			top = make_base_node<NodeMin>();
			minArg = 2;
			maxArg = 100;
		}
		else if( *cur == "MAX")
		{
			top = make_base_node<NodeMax>();
			minArg = 2;
			maxArg = 100;
		}
		else if( *cur == "SMOOTH")
		{
			top = make_base_node<NodeSmooth>();
			minArg = 4;
			maxArg = 4;
		}
		//	...

		if( top)
		{
			string func = *cur;
			++cur;

			//	Matched a function, parse its arguments and check
			top->arguments = parseFuncArg( cur, end);
			if( top->arguments.size() < minArg || top->arguments.size() > maxArg)
				throw script_error( (string( "Function ") + func + ": wrong number of arguments").c_str());

			//	Return
			return top;
		}

		//	When everything else fails, we have a variable
		return parseVar( cur);
	}

	static Expression parseConst( TokIt& cur)
        {
                //      Convert to double
                double v = stod( *cur);

                //      Build the const node
                auto top = make_base_node<NodeConst>(v);

                //      Advance over var and return
                ++cur;
                return top;
        }
        static Expression parseList(TokIt& cur, const TokIt end)
        {
                TokIt closeIt = findMatch<'[',']'>(cur, end);
                vector<Expression> vals;
                ++cur;
                while(cur != closeIt)
                {
                        vals.push_back(parseExpr(cur, end));
                        if(*cur == ",") ++cur;
                        else if(cur != closeIt)
                                throw script_error("List elements must be separated by commas");
                }
                cur = ++closeIt;
                auto top = make_base_node<NodeList>();
                top->arguments = move(vals);
                return top;
        }


	static vector<Expression> parseFuncArg( TokIt& cur, const TokIt end)
	{
		//	Check that we have a '(' and something after that
		if( (*cur)[0] != '(')
			throw script_error( "No opening ( following function name");

		//	Find matching ')'
		TokIt closeIt = findMatch<'(',')'>( cur, end);

		//	Parse expressions between parentheses
		vector<Expression> args;
		++cur;	//	Over '('
		while( cur != closeIt)
		{
			args.push_back( parseExpr( cur, end));
			if( (*cur)[0] == ',') ++cur;
			else if( cur != closeIt)
				throw script_error( "Arguments must be separated by commas");
		}

		//	Advance and return
		cur = ++closeIt;
		return args;
	}

	static Expression parseVar( TokIt& cur)
	{
		//	Check that the variable name starts with a letter
		if( (*cur)[0] < 'A' || (*cur)[0] > 'Z')
			throw script_error( (string( "Variable name ") + *cur + " is invalid").c_str());

		//	Build the var node
		auto top = make_base_node<NodeVar>(*cur);

                //      Advance over var and return
                ++cur;
                return top;
	}

	//	Conditions

	//	Parent, Level 1, 'or'
	static Expression parseCond( TokIt& cur, const TokIt end)
	{
		//	First exhaust all L2 (and) and above (elem) conditions on the lhs
		auto lhs = parseCondL2( cur, end);
	
		//	Do we have an 'or'?
		while( cur != end && *cur == "OR")
		{
			//	Advance cur over 'or' and parse the rhs
			++cur;

			//	Should not stop straight after 'or'
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Exhaust all L2 (and) and above (elem) conditions on the rhs		
			auto rhs = parseCondL2( cur, end);

			//	Build node and assign lhs and rhs as its arguments, store in lhs
			lhs = make_base_binary<NodeOr>( lhs, rhs);
		}

		//	No more 'or', and L2 and above were exhausted, hence condition is complete
		return lhs;
	}

	//	Level 2 'and'
	static Expression parseCondL2( TokIt& cur, const TokIt end)
	{	
		//	First parse the leftmost elem or parenthesed condition 
		auto lhs = parseParentheses<parseCond,parseCondElem>( cur, end);
	
		//	Do we have an 'and'?
		while( cur != end && *cur == "AND")
		{
			//	Advance cur over 'and' and parse the rhs
			++cur;

			//	Should not stop straight after 'and'
			if( cur == end) throw script_error( "Unexpected end of expression");
			
			//	Parse the rhs elem or parenthesed condition 
			auto rhs = parseParentheses<parseCond,parseCondElem>( cur, end);

			//	Build node and assign lhs and rhs as its arguments, store in lhs
			lhs = make_base_binary<NodeAnd>( lhs, rhs);
		}

		//	No more 'and', so L2 and above were exhausted, return to check for an or
		return lhs;
	}

	//	Helper function that parses the optional fuzzy parameters for conditions
	static void parseCondOptionals( TokIt& cur, const TokIt end, double& eps)
	{
		//	Default
		eps = -1.0;

		while( *cur == ";" || *cur == ":")
		{
			//	Record
			const char c = (*cur)[0];
			//	Over ;:
			++cur;
			//	Check for end
			if( cur == end) throw script_error( "Unexpected end of expression");

			//	Eps
			eps = stod( *cur);
			++cur;
		}
	}

	//	Helpers for elementary conditions
	static Expression buildEqual(Expression& lhs, Expression& rhs, const double eps)
	{
		auto expr = make_base_binary<NodeSub>( lhs,rhs);
		auto top = make_base_node<NodeEqual>();
		top->arguments.resize( 1);
		top->arguments[0] = move( expr);
		top->eps = eps;
		return move( top);
	}
	static Expression buildDifferent(Expression& lhs, Expression& rhs, const double eps)
	{
		auto eq = buildEqual( lhs, rhs, eps);
		auto top = make_base_node<NodeNot>();
		top->arguments.resize( 1);
		top->arguments[0] = move( eq);
		return top;
	}
	static Expression buildSuperior(Expression& lhs, Expression& rhs, const double eps)
	{
		auto expr = make_base_binary<NodeSub>( lhs,rhs);
		auto top = make_base_node<NodeSup>();
		top->arguments.resize( 1);
		top->arguments[0] = move( expr);
		top->eps = eps;
		return move( top);
	}
	static Expression buildSupEqual(Expression& lhs, Expression& rhs, const double eps)
	{
		auto expr = make_base_binary<NodeSub>( lhs,rhs);
		auto top = make_base_node<NodeSupEqual>();
		top->arguments.resize( 1);
		top->arguments[0] = move( expr);
		top->eps = eps;
		return move( top);
	}

	//	Highest level elementary
	static Expression parseCondElem( TokIt& cur, const TokIt end)
	{
		//	Parse the LHS expression
		auto lhs = parseExpr( cur, end);

		//	Check for end
		if( cur == end) throw script_error( "Unexpected end of expression");

		//	Advance to token immediately following the comparator
		string comparator = *cur;
		++cur;

		//	Check for end
		if( cur == end) throw script_error( "Unexpected end of expression");

		//	Parse the RHS
		auto rhs = parseExpr( cur, end);

		//	Parse the optionals
		double eps;
		parseCondOptionals( cur, end, eps);

		//	Build the top node, set its arguments and return
		if( comparator == "=") return buildEqual( lhs, rhs, eps);
		else if( comparator == "!=") return buildDifferent( lhs, rhs, eps);
		else if( comparator == "<") return buildSuperior( rhs, lhs, eps);
		else if( comparator == ">") return buildSuperior( lhs, rhs, eps);
		else if( comparator == "<=") return buildSupEqual( rhs, lhs, eps);
		else if( comparator == ">=") return buildSupEqual( lhs, rhs, eps);
		else throw script_error( "Elementary condition has no valid comparator");
	}

	//	Statements

	static Statement parseIf( TokIt& cur, const TokIt end)
	{
		//	Advance to token immediately following "if"
		++cur;

		//	Check for end
		if( cur == end)
			throw script_error( "'If' is not followed by 'then'");

		//	Parse the condition
		auto cond = parseCond( cur, end);

		//	Check that the next token is "then"
		if( cur == end || *cur != "THEN")
			throw script_error( "'If' is not followed by 'then'");
	
		//	Advance over "then"
		++cur;

		//	Parse statements until we hit "else" or "endIf"
		vector<Statement> stats;
		while( cur != end && *cur != "ELSE" && *cur != "ENDIF")
			stats.push_back( parseStatement( cur, end));

		//	Check
		if( cur == end)
			throw script_error( "'If/then' is not followed by 'else' or 'endIf'");

		//	Else: parse the else statements
		vector<Statement> elseStats;
		int elseIdx = -1;
		if( *cur == "ELSE")
		{
			//	Advance over "else"
			++cur;
			//	Parse statements until we hit "endIf"
			while( cur != end && *cur != "ENDIF")
				elseStats.push_back( parseStatement( cur, end));
			if( cur == end)
				throw script_error( "'If/then/else' is not followed by 'endIf'");
			//	Record else index
			elseIdx = int(stats.size()) + 1;
		}

		//	Finally build the top node
		auto top = make_base_node<NodeIf>();
		top->arguments.resize( 1 + stats.size() + elseStats.size());
		top->arguments[0] = move( cond);			//	Arg[0] = condition
		for( size_t i=0; i<stats.size(); ++i)		//	Copy statements, Arg[1..n-1]
			top->arguments[i+1] = move( stats[i]);
		for( size_t i=0; i<elseStats.size(); ++i)	//	Copy else statements, Arg[n..N]
			top->arguments[i+elseIdx] = move( elseStats[i]);
		top->firstElse = elseIdx;

		//	Advance over endIf and return
		++cur;
		return move( top); // Explicit move is necessary because we return a base class pointer
	}

	static Statement parseAssign( TokIt& cur, const TokIt end, Expression& lhs)
	{
		//	Advance to token immediately following "="
		++cur;

		//	Check for end
		if( cur == end) throw script_error( "Unexpected end of statement");

		//	Parse the RHS
		auto rhs = parseExpr( cur, end);

		//	Build and return the top node
		return make_base_binary<NodeAssign>( lhs, rhs);
	}

	static Statement parsePays( TokIt& cur, const TokIt end, Expression& lhs)
	{
		//	Advance to token immediately following "pays"
		++cur;

		//	Check for end
		if( cur == end) throw script_error( "Unexpected end of statement");

		//	Parse the RHS
		auto rhs = parseExpr( cur, end);

		//	Build and return the top node
		return make_base_binary<NodePays>( lhs, rhs);
	}
        static Statement parseFor( TokIt& cur, const TokIt end)
        {
                ++cur;
                if(cur==end) throw script_error("'for' must be followed by variable");
                auto var = parseVar(cur);
                if(cur==end || *cur != "IN") throw script_error("'for' must be followed by 'in'");
                ++cur;
                auto lst = parseExpr(cur, end);
                if(cur==end || *cur != "THEN") throw script_error("'for' is not followed by 'then'");
                ++cur;
                vector<Statement> stats;
                while(cur!=end && *cur != "ENDFOR") stats.push_back(parseStatement(cur,end));
                if(cur==end) throw script_error("'for' has no matching 'endFor'");
                auto top = make_base_node<NodeFor>();
                top->arguments.resize(2+stats.size());
                top->arguments[0] = move(var);
                top->arguments[1] = move(lst);
                for(size_t i=0;i<stats.size();++i) top->arguments[i+2] = move(stats[i]);
                ++cur;
                return move(top);
        }


public:

    static Expression parseExpression(TokIt& cur, const TokIt end)
    {
        return parseExpr(cur, end);
    }

	//	Statement = unique_ptr<Node>
	static Statement parseStatement( TokIt& cur, const TokIt end)
	{
                //      Check for instructions of type 1, 'if' or 'for'
                if( *cur == "IF") return parseIf( cur, end);
                if( *cur == "FOR") return parseFor( cur, end);

		//	Parse cur as a variable
		auto lhs = parseVar( cur);

		//	Check for end
		if( cur == end) throw script_error( "Unexpected end of statement");

		//	Check for instructions of type 2, so far only assignment
		if( *cur == "=") return parseAssign( cur, end, lhs);
		else if( *cur == "PAYS") return parsePays( cur, end, lhs);

		//	No instruction, error
		throw script_error( "Statement without an instruction");
		return Statement();
	}
};