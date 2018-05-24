#include <cstdio>
#include <cstring>
#include <forward_list>
#include <vector>
#include "Table.h"
#include "Decart.h"
#include <cstdlib>
#include <iostream>
using namespace std;

bool Check_If_Identical(char *str,  forward_list<char *> List, forward_list<char *> :: iterator end_iter) {
	auto iter = List.begin();
	while (iter != end_iter) {
		if (strcmp(*iter, str) == 0)
			return(false);
		iter++;
	}
	return(true);
}

char * Decart(char *table_name1, char *table_name2) {
	TableStruct *table_struct;
	FieldType type;
	long number;
	unsigned num1, num2, len, num;
	unsigned size = 0;
	char *unique, *field_name, *str, *unique1;
	THandle table_id1, table_id2, table_id;
	int i, j, k, numb1, numb2;
	forward_list<char *> List;
	forward_list<char *> :: iterator iter;
	forward_list<char *> :: iterator end_iter;
	vector<char*> fields;
	vector<FieldType> types;
	Errors err;

	if (strcmp(table_name1, table_name2) == 0)
		return(nullptr);
	err = openTable(table_name1, &table_id1);
	if (err != OK)
		return(nullptr);
	err = openTable(table_name2, &table_id2);
	if (err != OK)
		return(nullptr);
	getFieldsNum(table_id1, &num1);
	getFieldsNum(table_id2, &num2);
	cout<<"num1, num2 = "<<num1<<num2<<endl;
	table_struct = new TableStruct;
	table_struct->fieldsDef = (FieldDef *)malloc((num1 + num2) * sizeof(struct FieldDef));
	for (i = 0; i < num1; i++) {
		getFieldName(table_id1, i, &field_name);
		cout<<"field_name ="<<field_name<<endl;
		fields.push_back(field_name);
		getFieldType(table_id1, field_name, &type);
		types.push_back(type);
		if (type == Long) {
			(table_struct->fieldsDef)[i].type = Long;
			(table_struct->fieldsDef)[i].len = 8;
		}
		else {
			(table_struct->fieldsDef)[i].type = Text;
			getFieldLen(table_id1, field_name, &len);
			(table_struct->fieldsDef)[i].len = len;
			cout<<"len = "<<len<<endl;
		}
		strcpy((table_struct->fieldsDef)[i].name, field_name);
		List.push_front(field_name);////здесь
	}
	for ( ; i < num1 + num2; i++) {
		getFieldName(table_id2, i - num1, &field_name);
		cout<<"field_name ="<<field_name<<endl;
		fields.push_back(field_name);
		end_iter = List.end();
		if (not Check_If_Identical(field_name, List, end_iter)) {//здесь
			return(nullptr);
		}
		getFieldType(table_id2, field_name, &type);
		types.push_back(type);
		if (type == Long) {
			(table_struct->fieldsDef)[i].type = Long;
			(table_struct->fieldsDef)[i].len = 8;
		}
		else {
			(table_struct->fieldsDef)[i].type = Text;
			getFieldLen(table_id2, field_name, &len);
			(table_struct->fieldsDef)[i].len = len;
			cout<<"len = "<<len<<endl;
		}
		strcpy((table_struct->fieldsDef)[i].name, field_name);
	}
	(table_struct->numOfFields) = num1 + num2;
	//unique = new char [30];
	unique = tmpnam(NULL);

	createTable(unique, table_struct);
	
	numb1 = numb2 = 0;
	moveFirst(table_id1);
	moveFirst(table_id2);
	while (not afterLast(table_id1)) {
		moveNext(table_id1);
		numb1++;
		cout<<"numb1 = "<<numb1<<endl;
	}
	while (not afterLast(table_id2)) {
		moveNext(table_id2);
		numb2++;
		cout<<"numb2 = "<<numb2<<endl;
	}
	moveFirst(table_id1);
	openTable(unique, &table_id);
	for (i = 0; i < numb1; i++) {
		moveFirst(table_id2);
		for (k = 0; k < numb2; k++) {
			createNew(table_id);
			for (j = 0; j < num2 + num1; j++) {
				if (j < num1) {
					if (types[j] == Text) {
						getText(table_id1, fields[j], &str);
						putTextNew(table_id, fields[j], str);
						cout<<str<<endl;
					}
					if (types[j] == Long) {
						getLong(table_id1, fields[j], &number);
						putLongNew(table_id, fields[j], number);
						cout<<number<<endl;
					}
				}
				else {
					if (types[j] == Text) {
						getText(table_id2, fields[j], &str);
						putTextNew(table_id, fields[j], str);
						cout<<str<<endl;
					}
					if (types[j] == Long) {
						getLong(table_id2, fields[j], &number);
						putLongNew(table_id, fields[j], number);
						cout<<number<<endl;
					}
				}
			}
			insertzNew(table_id);
			moveNext(table_id2);
		}
		moveNext(table_id1);
	}

	closeTable(table_id1);
	closeTable(table_id2);
	closeTable(table_id);
	unique1 = new char[strlen(unique) + 1];
	strcpy(unique1, unique);
	cout<<"unique1 = "<<unique1<<endl;
	return(unique1);
}
