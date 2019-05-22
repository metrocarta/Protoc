#pragma once
#include <list>
using namespace std;

class SundType;
class Scanner;
/*
	source: A retargetable C compiler: design and implementation
	David Hanson

	initializer:
		assigment-expresion
		'{' initializer {,initializer } [,] '}'
*/
class SundType;
class SundParser;
class ChainExpression;
struct sund_type_node;

struct initializer_node{
	list<initializer_node *> * array_list;
	int size,depth;
	int num_children;
	ChainExpression * chain;
	initializer_node();
};
class InitializerPacket {
private:
	int size,segment;
	int position;
	//int max_array_size;
	int fill_mode;
	ChainExpression ** packet;
public:
	InitializerPacket();
	InitializerPacket(int size,int segment);
	int Fill(struct initializer_node * node, list<int> * dim, int start_index);
	int RefreshStatus(struct initializer_node * node, list<int> * dim);
	ChainExpression ** GetPacket(){ return packet; }
	friend class SundInitializer;
};
class SundInitializer
{
private:
	Scanner * scanner;
	SundParser * parser;
	SundType * object_sund_type;
	int init_list_size;
	ChainExpression ** init_list;
	// store array[d0][d1]...[dn]; d0,d1...dn in list
	list<int> * dimension;
	list<InitializerPacket *> packets;
	struct initializer_node * root_node;
	// is d0 -1 or defined
	int dim0;
	struct initializer_node * ScanRootArray();
	struct initializer_node * ScanArray(list<int> * dim);
public:
	enum init_msg { ARRAY_IS_TOO_BIG = 0,PACKET_IS_FULL,OK };
	SundInitializer(Scanner * scanner,SundParser * parser,SundType * object_sund_type);
	void Build();
	int GetInitListSize(){ return init_list_size; };
	ChainExpression ** GetInitList(){ return init_list; };
	~SundInitializer();
	static int GetValue(list<int> * dim);
};

