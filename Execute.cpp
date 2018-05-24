#include "Table.h"
#include "C_Where.h"
#include "Execute.h"
#include "Syntax.h"
#include "Lexem.h"
#include <cstring>
#include <unistd.h>
#include <stack>
#include <forward_list>
#include "Execute_Errors.h"
#include "Decart.h"
using namespace std;

THandle table_id;
Errors err;
FieldType type;
struct TableStruct *table_struct;
extern forward_list<Lexem> :: iterator iter;

void CARRY_OUT::Carry_Out(int sd) {
	char *table_name;
	Lexem cur;

	cur = *iter;
	iter++;
	if (cur.get_number() == 0)
		C_CREATE();
	if (cur.get_number() == 1)
		C_SELECT(sd);
	if (cur.get_number() == 2)
		C_UPDATE();
	if (cur.get_number() == 8)
		C_INSERT();
	if (cur.get_number() == 11)
		C_DELETE();
	if (cur.get_number() == 6)
		C_DROP();
	return;
}

void CARRY_OUT::C_SELECT(int sd) {
	bool flag_of_all = false;
	Lexem cur;
	char *table_name, *save, *buf;
	WHERE WHERE_OBJ;
	char *cur_name, *str1, *str2;
	unsigned num_of_fields;
	char *str, c;
	int i, len, count_of_tables = 1;
	long num;
	char *field_name;
	forward_list <char*> List_Of_Select;
	forward_list <char*> List_Of_Tables;
	forward_list<char*> :: iterator table_name_iter;
	forward_list<char*> :: iterator table_name_end_iter;

	cur = *iter;
	iter++;
	if (cur.get_type() == OPERATION) {
		flag_of_all = true;
	}
	else {
		field_name = cur.get_str();
		List_Of_Select.push_front(field_name);
		cur = *iter;
		while (cur.get_type() != KEYWORD) {
			iter++;
			cur = *iter;
			field_name = cur.get_str();
			List_Of_Select.push_front(field_name);
			iter++;
			cur = *iter;
		}
		List_Of_Select.reverse();
	}
	iter++;
	cur = *iter;
	table_name = cur.get_str();
	List_Of_Tables.push_front(table_name);
	iter++;
	cur = *iter;
	while (cur.get_type() == DELIMETER) {
		iter++;
		cur = *iter;
		List_Of_Tables.push_front(cur.get_str());
		iter++;
		count_of_tables++;
		cur = *iter;
	}
	List_Of_Tables.reverse();
	table_name_iter = List_Of_Tables.begin();
	table_name_end_iter = List_Of_Tables.end();
	table_name = *table_name_iter;
	table_name_iter++; 	
	if (count_of_tables > 1) {
		count_of_tables = 1;
		while (table_name_iter != table_name_end_iter) {
			save = new char[strlen(table_name) + 1];
			strcpy(save, table_name);
			table_name = Decart(table_name, *table_name_iter);
			cout<<"after Decart() = "<<table_name<<endl;
			if (table_name == nullptr)
				throw(Execute_Errors(1));
			table_name_iter++;
			count_of_tables++;
			if (count_of_tables > 2) {
				deleteTable(save);
			}
			delete [] save;
		}
	}
	//cout<<"there\n";
	err = openTable(table_name, &table_id);
	
	if (err != OK)
		throw (Execute_Errors(1));
	if (flag_of_all) {
		getFieldsNum(table_id, &num_of_fields);
		for (i = 1; i <= num_of_fields ; i++) {
			getFieldName(table_id, num_of_fields - i, &field_name);
			cout<<"field_name_in_select_list = "<<field_name<<endl;
			List_Of_Select.push_front(field_name);
		}
	}
	auto pos = iter;
	moveFirst(table_id);
	forward_list<char*> :: iterator begin_iter_sel = List_Of_Select.begin();
	forward_list<char*> :: iterator end_iter_sel = List_Of_Select.end();
	while (not afterLast(table_id)) {
		if (WHERE_OBJ.C_WHERE(table_id)) {
			cout<<"where success\n";
			auto iter_sel = begin_iter_sel;
			while (iter_sel != end_iter_sel) {
				cur_name = *iter_sel;
				cout<<"field_name = "<<cur_name<<endl;
				iter_sel++;
				err = getFieldType(table_id, cur_name, &type);
				if (err != OK) {
					throw (Execute_Errors(2));
				}
				if (type == Text) {
					err = getText(table_id, cur_name, &str);
					cout<<"str sending = "<<str<<endl;
					if (err != OK) {
						throw (Execute_Errors(3));
					}
					str1 = "STR = ";
					write(sd, str1, strlen(str1));
					write(sd, str, strlen(str));
					c = ' ';
					write(sd, &c, 1);
				}
				else {
					err = getLong(table_id, cur_name, &num);
					cout<<"num sending = "<<num<<endl;
					if (err != OK) {
						throw (Execute_Errors(3));
					}
					str2 = "NUM = ";
					write(sd, str2, strlen(str2));
					buf = new char[200];
					sprintf(buf, "%d", num);
					write(sd, buf, strlen(buf));
					delete [] buf;
					c = ' ';
					write(sd, &c, 1);
				}
			}
			c = '\n';
			write(sd, &c, 1);
		}
		iter = pos;
		moveNext(table_id);
	}
	if (count_of_tables > 1) {
		closeTable(table_id);
		deleteTable(table_name);
	}
	else {
		closeTable(table_id);
	}
	List_Of_Tables.clear();
	List_Of_Select.clear();
	cout<<"asf\n";
	cout<<table_name<<endl;
	delete [] table_name;
	return;
}

void CARRY_OUT::C_DROP() {
	Lexem cur;
	char *table_name;
	iter++;
	cur = *iter;
	table_name = cur.get_str();
	err = deleteTable(table_name);
	if (err != OK) {
		throw (Execute_Errors(1));
	}
	delete [] table_name;
	return;
}

void CARRY_OUT::C_DELETE() {
	Lexem cur;
	char *table_name;
	WHERE WHERE_OBJ;
	
	iter++;
	cur = *iter;
	iter++;
	table_name = cur.get_str();
	err = openTable(table_name, &table_id);
	if (err != OK) {
		throw (Execute_Errors(1));
	}
	err = moveFirst(table_id);
	if (err != OK) {
		throw (Execute_Errors(8));
	}
	auto pos = iter;
	while (not afterLast(table_id)) {
		if (WHERE_OBJ.C_WHERE(table_id)) {
			cout<<"want delete\n";
			deleteRec(table_id);
		}
		moveNext(table_id);
		iter = pos;
	}
	closeTable(table_id);
	delete [] table_name;
	return;
}
	
void CARRY_OUT::C_INSERT() {
	Lexem cur;
	char *table_name;
	char *field_name;
	char *str;
	unsigned int num;
	int count = 0;
	bool flag = false;

	iter++;
	cur = *iter;
	iter++;
	table_name = cur.get_str();
	err = openTable(table_name, &table_id);
	if (err != OK) {
		throw (Execute_Errors(1));
	}
	iter++;
	err = getFieldsNum(table_id, &num);
	if (err != OK) {
		throw (Execute_Errors(7));
	}
	auto pred = iter;
	while ((flag == false) and (count < num)) {
		err = getFieldName(table_id, count , &field_name);
		if (err != OK) {
			throw (Execute_Errors(2));
		}
		err = getFieldType(table_id, field_name, &type);
		if (err != OK) {
			throw (Execute_Errors(6));
		}
		cur = *iter;
		iter++;
		if (cur.get_type() == CONST_STR) {
			if (type != Text) {
				throw (Execute_Errors(3));
			}
		}
		else {
			if (type != Long) {
				throw (Execute_Errors(3));
			}
		}
		cur = *iter;
		iter++;
		count++;
		if (cur.get_type() == BRACKETS)
			flag = true;
	}
	if (count != num) {
		throw (Execute_Errors(4));
	}
	//тогда еще такой же цикл но уже записываем
	count = 1;
	iter = pred;
	err = createNew(table_id);
	if (err != OK) {
		throw (Execute_Errors(5));
	}
	while (count <= num) {
		err = getFieldName(table_id, count - 1, &field_name);
		if (err != OK) {
			throw (Execute_Errors(9));
		}
		cur = *iter;
		iter++;
		if (cur.get_type() == CONST_STR) {
			str = cur.get_str();
			err = putTextNew(table_id, field_name, str);
			if (err != OK) {
				throw (Execute_Errors(10));
			}
		}
		else {
			err = putLongNew(table_id, field_name, cur.get_number());
			if (err != OK) {
				throw (Execute_Errors(11));
			}
		}
		iter++;
		count++;
	}
	err = insertzNew(table_id);
	if (err != OK) {
		throw (Execute_Errors(12));
	}
	closeTable(table_id);
	return;
}

void CARRY_OUT::C_UPDATE() {
	Lexem cur;
	char *table_name;
	WHERE WHERE_OBJ;
	bool res;
	WHERE EXPR_OBJ;
	FieldType type_of_expr;
	bool log_flag;
	FieldType type;
	char * field_name;
	
	cur = *iter;
	iter++;
	table_name = cur.get_str();
	err = openTable(table_name, &table_id);
	if (err != OK) {
		throw (Execute_Errors(1));
	}
	iter++;
	cur = *iter;
	field_name = cur.get_str();
	err = getFieldType(table_id, field_name, &type);
	if (err != OK) {
		throw (Execute_Errors(2));
	}
	err = moveFirst(table_id);
	iter++;
	iter++;
	auto pos_expr = iter;
	while (((*iter).get_type() != KEYWORD) and (cur.get_number()!= 3)) {
		iter++;
	}
	auto pos_where = iter;
	while (not afterLast(table_id)) {
		if (WHERE_OBJ.C_WHERE(table_id)) {
			err = startEdit(table_id);
			if (err != OK) {
				throw (Execute_Errors(13));
			}
			char *str;
			long value;
			iter = pos_expr;
			type_of_expr = EXPR_OBJ.C_EXPRESSION(log_flag, value, str, res);
			if (type_of_expr != type) {
				throw (Execute_Errors(14));
			}
			if (type_of_expr == Text) {
				err = putText(table_id, field_name, str);
				if (err != OK)
					throw (Execute_Errors(10));
				err = finishEdit(table_id);
				if (err != OK) {
					throw (Execute_Errors(15));
				}
			}
			if (type_of_expr == Long) {
				err = putLong(table_id, field_name, value);
				if (err != OK)
					throw (Execute_Errors(11));
				err = finishEdit(table_id);
				if (err != OK) {
					throw (Execute_Errors(15));
				}
			}	
		}
		iter = pos_where;
		err = moveNext(table_id);
		if (err != OK) {
			throw (Execute_Errors(16));
		}
	}
	closeTable(table_id);
	return;
}

void CARRY_OUT::C_CREATE() {
	Lexem cur;
	char *field_name;
	char *table_name;
	TableStruct *table_struct;
	int len;
	bool flag = false;
	unsigned size = 0;

	table_struct = new TableStruct;
	table_struct->fieldsDef = (FieldDef *)malloc(sizeof(struct FieldDef));
	cur = *iter;
	iter++;
	cur = *iter;
	iter++;
	table_name = cur.get_str();
	struct FieldDef *fields_def;
	iter++;
	while (flag == false) {
		cur = *iter;
		iter++;
		field_name = cur.get_str();
		cur = *iter;
		iter++;
		if (cur.get_number() == 9) {
			iter++;
			cur = *iter;
			len = cur.get_number();
			iter++;
			table_struct->fieldsDef = (FieldDef *)realloc(table_struct->fieldsDef, (size + 1) * sizeof(struct FieldDef));
			strcpy((table_struct->fieldsDef)[size].name, field_name);
			(table_struct->fieldsDef)[size].type = Text;
			(table_struct->fieldsDef)[size].len = len;
			size++;
			iter++;
		}
		else {
			if (cur.get_number() == 10) {
				table_struct->fieldsDef = (FieldDef *)realloc(table_struct->fieldsDef, (size + 1) * sizeof(struct FieldDef));
				strcpy((table_struct->fieldsDef)[size].name, field_name);
				(table_struct->fieldsDef)[size].type = Long;
				(table_struct->fieldsDef)[size].len = 8;
				size++;
			}
		}
		cur = *iter;
		if (cur.get_type() == BRACKETS)
			flag = true;
		iter++;
	}
	table_struct->numOfFields = size;
	err = createTable(table_name, table_struct);
	if (err != OK) {
		throw (Execute_Errors(17));
	}
	return;
}
