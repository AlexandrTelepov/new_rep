//4 стека : один для типов, другой для строк, третий для чисел, четвертый для bool
#include <stack>
#include <forward_list>
#include "Table.h"
#include "C_Where.h"
#include "Lexem.h"
#include <cstring>
#include "Execute_Errors.h"
extern forward_list<Lexem> :: iterator iter;
extern forward_list<Lexem> :: iterator end_iter;
stack <FieldType> Stack_Of_Types;
stack <bool> Stack_Bool;
stack <char *> Stack_Text;
stack <long> Stack_Long;
THandle id;
bool log_flag = false;

bool WHERE::List_Of_Constant(char* str, bool not_flag) {
	Lexem cur;
	char *cur_str;
	bool flag = false;

	cur = *iter;
	iter++;
	if (cur.get_type() == NUMBER)
		throw (Execute_Errors(18));
	if (cur.get_type() == CONST_STR) {
		cur_str = cur.get_str();
		if (strcmp(str, cur_str) == 0)
			flag = true;
		delete [] cur_str;
		cur = *iter;
		iter++;
		while (cur.get_type() != BRACKETS) {
			cur = *iter;
			iter++;
			cur_str = cur.get_str();
			if (strcmp(str, cur_str) == 0)
				flag = true;
			delete [] cur_str;
			cur = *iter;
			iter++;
		}
		delete [] str;
		if (not_flag == false)
			return(flag);
		else
			return(not flag);
	}
}

bool WHERE::List_Of_Constant(int value, bool not_flag) {
	Lexem cur;
	int cur_num;
	bool flag = false;
	
	Skip_Probels();
	iter++;
	Skip_Probels();
	cur = *iter;
	iter++;
	if (cur.get_type() == CONST_STR) {
		throw (Execute_Errors(18));
	}
	if (cur.get_type() == NUMBER) {
		cur_num = cur.get_number();
		if (cur_num == value)
			flag = true;
		cur = *iter;
		iter++;
		while (cur.get_type() != BRACKETS) {
			cur = *iter;
			iter++;
			cur_num = cur.get_number();
			if (cur_num == value)
				flag = true;
			cur = *iter;
			iter++;
		}
		if (not_flag == false)
			return(flag);
		else
			return(not flag);
	}
}

bool WHERE::C_WHERE(THandle ID) {
	char *str;
	Errors err;
	FieldType type;
	long res_value;
	char *field_name, *like_str, *text;
	char * res_str;
	long value;
	bool res_bool;
	FieldType type_of_expr;
	bool not_flag = false;
	id = ID;
	bool flag = false;
	Lexem cur;

	iter++;
	cur = *iter;
	auto pred = iter;
	iter++;
	if (cur.get_type() == KEYWORD) {
		return(true);
	}
	if (cur.get_type() == IDENTIFICATOR) {
		field_name  = cur.get_str();
		err = getFieldType(id, field_name, &type);
		if ((field_name != nullptr) and (err != OK)) {
			throw (Execute_Errors(6));
		}
		if ((field_name != nullptr) and (type == Text)) {
		//NOT LIKE | LIKE некоторая строка
			cur = *iter;
			iter++;
			if (cur.get_number() == 16) {
				not_flag = true;
				cur = *iter;
				iter++;
			}
			if ((cur.get_type() == KEYWORD) or (cur.get_number() == 17)) {
				cur = *iter;
				iter++;
				like_str = cur.get_str();
				err = getText(id, field_name, &text);
				if (err != OK) {
					throw(Execute_Errors(2));
				}
				return(LIKE(text, like_str));
			}
		}
	}
	iter = pred;
	type_of_expr = C_EXPRESSION(log_flag, res_value, res_str, res_bool);
	if (log_flag == true) {
		while (not Stack_Bool.empty()) {
			Stack_Bool.pop();
		}
		while (not Stack_Of_Types.empty()) {
			Stack_Of_Types.pop();
		}
		while (not Stack_Text.empty()) {
			Stack_Text.pop();
		}
		while (not Stack_Long.empty()) {
			Stack_Long.pop();
		}
		return(res_bool);
	}
	cur = *iter;
	if ((cur.get_type() == KEYWORD) and (cur.get_number() == 12)) {
		not_flag = true;
		iter++;
	}
	iter++;
	while (not Stack_Bool.empty()) {
		Stack_Bool.pop();
	}
	while (not Stack_Of_Types.empty()) {
		Stack_Of_Types.pop();
	}
	while (not Stack_Text.empty()) {
		Stack_Text.pop();
	}
	while (not Stack_Long.empty()) {
		Stack_Long.pop();
	}
	if (type_of_expr == Long) 
		return(List_Of_Constant(res_value, not_flag));
	else
		return(List_Of_Constant(res_str, not_flag));
}

void WHERE::Push_Values(bool &log_flag, long &value, char * &str, bool &res) {
	if (Stack_Of_Types.top() == Logic)
		res = Stack_Bool.top();
	if (Stack_Of_Types.top() == Long)
		value = Stack_Long.top();
	if (Stack_Of_Types.top() == Text)
		str = Stack_Text.top();
	log_flag = ::log_flag;
	return;
}

FieldType WHERE::C_EXPRESSION(bool &log_flag, long & value, char* & str, bool & res) {
	Lexem cur;
	FieldType type1, type2;

	A();
	cur = *iter;
	log_flag = ::log_flag;
	//cout<<"after A() in EXPR()\n";
	if (iter == end_iter) {
		//cout<<"end_iter\n";
		Push_Values(log_flag, value, str, res);
		return(Stack_Of_Types.top());
	}
	auto pred = iter;
	cur = *iter;
	iter++;
	while ((cur.get_type() == LOGIC_OPERATOR) and (cur.get_number() == 0)) {
		A();
		type2 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		type1 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		if ((type1 != Logic) or (type2 != Logic))
			throw(Execute_Errors(19));
		auto op2 = Stack_Bool.top();
		Stack_Bool.pop();
		auto op1 = Stack_Bool.top();
		Stack_Bool.pop();
		Logic_Computation(0, op1, op2);
		if (iter == end_iter) {
			Push_Values(log_flag, value, str, res);
			return(Stack_Of_Types.top());
		}
		cur = *iter;
		pred = iter;
		iter++;
	}
	iter = pred;
	cur = *iter;
	//cout<<"In end of C_EXPRESSION() type = "<<cur.get_type()<<cur.get_number()<<endl;
	Push_Values(log_flag, value, str, res);
	return(Stack_Of_Types.top());
}

void WHERE::A() {
	Lexem cur;
	FieldType type1, type2;

	B();
	if (iter == end_iter)
		return;
	auto pred = iter;
	cur = *iter;
	iter++;
	while ((cur.get_type() == LOGIC_OPERATOR) and (cur.get_number() == 1)) {
		B();
		type2 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		type1 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		if ((type1 != Logic) or (type2 != Logic))
			throw(Execute_Errors(19));
		auto op2 = Stack_Bool.top();
		Stack_Bool.pop();
		auto op1 = Stack_Bool.top();
		Stack_Bool.pop();
		Logic_Computation(1, op1, op2);
		if (iter == end_iter) {
			return;
		}
		cur = *iter;
		pred = iter;
		iter++;
	}
	iter = pred;
	return;
}

void WHERE::B() {
	Lexem cur;

	if (iter == end_iter)
		return;
	cur = *iter;
	auto pred = iter;
	iter++;
	if ((cur.get_type() == KEYWORD) and (cur.get_number() == 12)) {
		B();
		auto type = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		auto op = Stack_Bool.top();
		Stack_Bool.pop();
		if (type != Logic)
			throw(Execute_Errors(19));
		Stack_Of_Types.push(Logic);
		Stack_Bool.push(not op);
	}
	else {
		iter = pred;
		C();
		F();
	}
	return;
}

void WHERE::C() {
	Lexem cur;
	FieldType type1, type2;
	int num;

	D();
	if (iter == end_iter)
		return;
	auto pred = iter;
	cur = *iter;
	iter++;
	while ((cur.get_type() == OPERATION) and ((cur.get_number() == '+') or (cur.get_number() == '-'))) {
		num = cur.get_number();
		D();
		type2 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		type1 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		if ((type1 != Long) or (type2 != Long))
			throw(Execute_Errors(19));
		auto op2 = Stack_Long.top();
		Stack_Long.pop();
		auto op1 = Stack_Long.top();
		Stack_Long.pop();
		Operation_Computation(num, op1, op2);
		if (iter == end_iter) {
			return;
		}
		cur = *iter;
		pred = iter;
		iter++;
	}
	iter = pred;
	return;
}

void WHERE::D() {
	Lexem cur;
	FieldType type1, type2;

	E();
	if (iter == end_iter)
		return;
	auto pred = iter;
	cur = *iter;
	iter++;
	while ((cur.get_type() == OPERATION) and ((cur.get_number() == '*') or (cur.get_number() == '/'))) {
		int num = cur.get_number();
		E();
		type2 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		type1 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		if ((type1 != Long) or (type2 != Long))
			throw(Execute_Errors(19));
		auto op2 = Stack_Long.top();
		Stack_Long.pop();
		auto op1 = Stack_Long.top();
		Stack_Long.pop();
		Operation_Computation(num, op1, op2);
		if (iter == end_iter) {
			return;
		}
		cur = *iter;
		pred = iter;
		iter++;
	}
	iter = pred;
	cur = *iter;
	return;
}

void WHERE::E() {
	Lexem cur;
	bool log_expr_flag, res;
	char *field_name;
	FieldType type;
	char *str;
	long value;
	char *strnew;

	cur = *iter;
	iter++;
	if (cur.get_type() == NUMBER) {
		Stack_Of_Types.push(Long);
		Stack_Long.push(cur.get_number());
		return;
	}
	if (cur.get_type() == CONST_STR) {
		Stack_Of_Types.push(Text);
		str = cur.get_str();
		Stack_Text.push(str);
		delete [] str;
		return;
	}
	if (cur.get_type() == IDENTIFICATOR) {
		field_name = cur.get_str();
		getFieldType(id, field_name, &type);
		Stack_Of_Types.push(type);
		//cout<<"push type in E() = "<<type<<endl;
		if (type == Text) {
			getText(id, field_name, &str);
			strnew = new char [strlen(str) + 1];
			strcpy(strnew, str);
			Stack_Text.push(strnew);
		}
		else {
			getLong(id, field_name, &value);
			Stack_Long.push(value);
		}
		delete [] field_name;
		return;
	}
	if ((cur.get_type() == BRACKETS) and (cur.get_number() == '(')) {
		C_EXPRESSION(log_expr_flag, value, str, res);
		//cout<<"In A() after (S)"<<cur.get_type()<<endl;
		if (iter == end_iter)
			return;
		cur = *iter;
		iter++;
		return;
	}
	return;
}

void WHERE::F() {
	Lexem cur;
	FieldType type1, type2;
	bool op1, op2;
	long val1, val2;
	char *str1, *str2;

	if (iter == end_iter)
		return;
	cur = *iter;
	auto pred = iter;
	iter++;
	if (cur.get_type() == OPERATOR) {
		int num = cur.get_number();
		::log_flag = true;
		C();
		type2 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		type1 = Stack_Of_Types.top();
		Stack_Of_Types.pop();
		//cout<<"types in F() = "<<type1<<type2<<endl;
		if (((type1 == Long) and (type2 != Long)) or ((type1 == Text) and (type2 != Text)) or ((type1 == Logic) and (type2 != Logic)))
			throw(Execute_Errors(19));
		if (type1 == Long) {
			//cout<<"first_long\n";
			val2 = Stack_Long.top();
			Stack_Long.pop();
			//cout<<"second_long\n";
			val1 = Stack_Long.top();
			Stack_Long.pop();
			//cout<<"val2 val1 = "<<val2<<val1<<endl;
			Operator_Computation(num, val1, val2);
			//cout<<"after computation\n";
			return;
		}
		if (type1 == Text) {
			str2 = Stack_Text.top();
			Stack_Text.pop();
			str1 = Stack_Text.top();
			Stack_Text.pop();
			Operator_Computation(num, str1, str2);
			delete [] str1;
			delete [] str2;
			return;
		}
		if (type1 == Logic) {
			op2 = Stack_Bool.top();
			Stack_Bool.pop();
			op1 = Stack_Bool.top();
			Stack_Bool.pop();
			Operator_Computation(num, op1, op2);
			return;
		}
	}
	iter  = pred;
	return;
}

void WHERE::Operation_Computation(int num1, long num2, long num3) {
	if (num1 == '*')
		Stack_Long.push(num2*num3);
	if (num1 == '-')
		Stack_Long.push(num2-num3);
	if (num1 == '/')
		Stack_Long.push(num2/num3);
	if (num1 == '+')
		Stack_Long.push(num2+num3);
	Stack_Of_Types.push(Long);
}

void WHERE::Operator_Computation (int num1, char *num2, char *num3) {
	int res; 
	//cout<<"-----------------------------------------------------------------------------\n";
	res = strcmp(num2, num3);
	if (num1 == '=')
		Stack_Bool.push(res == 0);
	if (num1 == '<')
		Stack_Bool.push(res < 0);
	if (num1 == '>')
		Stack_Bool.push(res > 0);
	if (num1 == 0)
		Stack_Bool.push(res != 0);
	if (num1 == 1)//<=
		Stack_Bool.push(res <= 0);
	if (num1 == 2)//>=
		Stack_Bool.push(res >= 0);
	Stack_Of_Types.push(Logic);
}

void WHERE::Operator_Computation (int num1, bool num2, bool num3) {
	//cout<<"-----------------------------------------------------------------------------\n";
	if (num1 == '=')
		Stack_Bool.push(num2 == num3);
	else
		Stack_Bool.push(num2 != num3);
	Stack_Of_Types.push(Logic);
}

void WHERE::Operator_Computation (int num1, long num2, long num3) {
	//cout<<"in computation\n";
	if (num1 == '=')
		Stack_Bool.push(num2 == num3);
	if (num1 == '<')
		Stack_Bool.push(num2 < num3);
	if (num1 == '>') 
		Stack_Bool.push(num2 > num3);
	if (num1 == 0)
		Stack_Bool.push(num2 != num3);
	if (num1 == 1)//<=
		Stack_Bool.push(num2 <= num3);
	if (num1 == 2)//>=
		Stack_Bool.push(num2 >= num3);
	Stack_Of_Types.push(Logic);
}

void WHERE::Logic_Computation (int num1, bool num2, bool num3) {
	if (num1 == 0) 
		Stack_Bool.push(num2 or num3);
	else
		Stack_Bool.push(num2 and num3);
	Stack_Of_Types.push(Logic);
}

bool WHERE::LIKE(char* str, char* pattern) {
	bool not_flag = false, flag = false;
	forward_list<char> :: iterator iter;
	forward_list<char> :: iterator end_iter;
	forward_list<char> List;
	int k;
	char *c;
	char *a = str;
	char *b = pattern;
	cout<<pattern<<endl;

	while (*b) {
		while ((*a) and (*b) and (not_In_Tocens(*b) == 0)) {
			if ((*b) != (*a))
				return(false);
			else {
				b++;
				a++;
			}
		}
		if ((k = not_In_Tocens(*b)) == 0) {
			if (not (*b))
				return(not(*a));
			else {
				while ((*b) and ((*b) == '%')) {
						b++;
				} 
				return(not(*b));
			}
		}
		if (k == 3) {
			if (not (*a))
				return(false);
			else {
				return(LIKE(a + 1, b + 1));
			}
		}
		if (k == 4) {
			if ((*a) and ((*a) == '^'))
				return(LIKE(a + 1, b + 1));
			else
				return(false);
		}
		if (k == 1) {
			if (not (*a)) { 
				while ((*b) and ((*b) == '%')) {
						b++;
				} 
				return(not(*b));
			}
			else {
				c = a;
				while (*c) {
					if (LIKE(c, b + 1))
						return(true);
					else {
						c++;
					}
				}
				while ((*b) and ((*b) == '%')) {
						b++;
				} 
				return(not(*b));
			}
		}
		if (k == 2) {
			b++;
			if (not(*b)) {
				return(((*a) == '[') and not(a + 1));
			}
			if ((*b) == '^') {
				b++;
				not_flag = true;
			}
			///читаем пока символ из алфавита
			while ((*b) and (In_Symbols(*b))) {
				List.push_front(*b);
				b++;
			}
			if (not (*b)) 
				return(false);
			if ((*b) != ']')
				return(false);
			//чек есть ли в листе *a с учетом not_flag
			iter = List.begin();
			end_iter = List.end();
			while (iter != end_iter) {
				if ((*a) == *iter)
					flag = true;
				iter++;
			}
			if (not_flag) {
				if (not flag)
					return(LIKE(a + 1, b + 1));
				else 
					return(false);
			}
			else {
				if (not flag)
					return(false);
				else 
					return(LIKE(a + 1, b + 1));
			}
		}
	}
	if (*a)
		return(false);
	else
		return(true);
}

int WHERE::not_In_Tocens(char c) {
	if (c == '\0')
		return(0);
	if (c == '%')
		return(1);
	if (c == '[')
		return(2);
	if (c == '_')
		return(3);
	if (c == '^')
		return(4);
	return(0);	
}

bool WHERE::In_Symbols(char c) { 
	if (((c >= 'a') and (c <= 'z')) or ((c <= 'Z') and (c >= 'A')) or ((c <= '9') and (c >= '0')))
		return(true);
	else
		return(false);
}
