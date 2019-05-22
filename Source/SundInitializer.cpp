#include "../Include/SundInitializer.h"
#include "../Include/SundTypeBuilder.h"
#include "../Include/Scanner.h"
#include "../Include/SundParser.h"
#include "../Include/ErrorPrinter.h"
#include "../Include/SundCompiler.h"
#include "../Include/ChainExpression.h"
initializer_node::initializer_node()
{ 
	chain = 0; 
	size = -1; 
	depth = 0;
	array_list = 0;
	num_children = 0;
}
// class InitializerPacket
InitializerPacket::InitializerPacket()
{
	this->packet = 0;
	this->size = 0;
	this->position =  0;
	//this->max_array_size = 0;
	this->fill_mode = 0;
}
InitializerPacket::InitializerPacket(int size,int segment)
{
	this->size = size;
	this->segment = segment;
	this->position = 0;
	this->fill_mode = 0;
	this->packet = new ChainExpression*[size];
	// init with zero
	for (int i = 0; i < size; i++){
		*(this->packet + i) = 0;
	}
}
int InitializerPacket::Fill(struct initializer_node * node, list<int> * dim,  int start_index)
{
	ChainExpression ** p = (packet + start_index);
	int index = start_index;// , front = 1;
	if (node->array_list == 0){
		*p = node->chain;
		return 1;
	}
	list<int>  new_dimension = *dim;
	new_dimension.pop_front();
	int segment = SundInitializer::GetValue(&new_dimension);
	/*if (new_dimension.size()){
	    front = new_dimension.front();
	}*/
	list<struct initializer_node *>::iterator it;
	for (it = node->array_list->begin(); it != node->array_list->end(); it++)
	{
		int result = this->Fill(*it, &new_dimension, index);
		if ((*it)->num_children > 0){
			index += segment;
		}
		else index += result;
	}
	return (index-start_index);
}
int InitializerPacket::RefreshStatus(struct initializer_node * node, list<int> * dim)
{
	int n = node->num_children;
	
	if (!this->position)
	{
		if (n > size)
			return SundInitializer::ARRAY_IS_TOO_BIG;
		if (n == 1)
		{
			this->Fill(node,dim,this->position);
			this->position += 1;
			this->fill_mode = 1;
			if (this->position == size)
				return SundInitializer::PACKET_IS_FULL;
			else return SundInitializer::OK;
		}
		if (n > 1){
			this->Fill(node, dim, this->position);
			this->position = size;
			return SundInitializer::PACKET_IS_FULL;
		}
	}
	
	if (this->fill_mode)
	{
		if (n > segment || (n > 1 && this->position%segment > 0))
			return SundInitializer::ARRAY_IS_TOO_BIG;
		else
		if (n > 1 && this->position%segment == 0){
			this->Fill(node, dim, this->position);
		    this->position = (1 + (this->position + n) / segment)*segment;
		}
		else
		if (n == 1 && this->position == size - 1){
			this->Fill(node, dim, this->position);
			return SundInitializer::PACKET_IS_FULL;
		}
		else
		if (n == 1)
		{
			this->Fill(node, dim, this->position);
			this->position += 1;
		}
	}
	return SundInitializer::OK;
}

// class SundInitializer
SundInitializer::SundInitializer(Scanner * scanner, SundParser * parser,SundType * object_sund_type)
{
	this->scanner = scanner;
	this->parser = parser;
	this->object_sund_type = object_sund_type;
	this->dimension = object_sund_type->GetArrayList();
	if (this->dimension->size())
	    this->dim0 = this->dimension->front();
	else this->dim0 = -1;
	this->packets.clear();
	this->init_list_size = 0;
}
initializer_node * SundInitializer::ScanRootArray()
{
	int array_counter = 0, sub_segment_size = 1, num_children = 0;
	struct initializer_node * array_node = new struct initializer_node();
	array_node->array_list = new list<initializer_node *>();
	list<int>  new_dimension = *(this->dimension);
	new_dimension.pop_front();
	int segment = GetValue(&new_dimension);
	if (segment > 1){
		sub_segment_size = new_dimension.front();
	}
	ChainExpression ** p = 0;
	// current number of children in node
	InitializerPacket * packet = new InitializerPacket(segment, segment/sub_segment_size);
	p = packet->GetPacket();
	this->packets.push_back(packet);
	
	//now, scan array
	do{
		// get first token
		scanner->GetToken();
		struct initializer_node * node;
		// the total number of childs
		int node_size;
		if (*scanner->token == '{')
		{
			if (new_dimension.size() == 0){
				// print error
				Err->Print(ErrorPrinter::CANNOT_CONVERT_INIT_LIST_TO_TYPE);
				return 0;
			}
			// recursion
			node = ScanArray(&new_dimension);
			node_size = node->num_children;
		}
		else{
			scanner->Putback();
			// assigment-expresion
			ScannerState * state = new ScannerState(scanner);
			node = new struct initializer_node();
			node->chain = parser->PerformExp(state);
			node_size = 1;
			node->num_children = 1;
		}
		switch (packet->RefreshStatus(node, &new_dimension)){
		case PACKET_IS_FULL:
			packet = new InitializerPacket(segment, segment / sub_segment_size);
			p = packet->GetPacket();
			this->packets.push_back(packet);
			break;
		case ARRAY_IS_TOO_BIG:
			// print error
			Err->Print(ErrorPrinter::TOO_MANY_INITIALIZERS);
			return 0;
		default:
			break;
		}
		// now, get ','
		scanner->GetToken();
		array_node->array_list->push_back(node);
		array_counter++;
		num_children += node_size;
	} while (*scanner->token == ',');

	if (*scanner->token != '}'){
		// print error
		Err->Print(ErrorPrinter::SYNTAX);
		return 0;
	}
	if (this->packets.size()){
		InitializerPacket * lp = (InitializerPacket *)this->packets.back();
		if (!lp->position){
			// remove empty packet
			this->packets.pop_back();
		}
	}
	int packets_size = this->packets.size(), l = 0;
	if (this->dim0 == -1){
		l = packets_size*segment;
	    this->init_list = new ChainExpression*[l];
		// change top array size
		this->object_sund_type->SetTopArraySize(packets_size);
	}
	else{
		if (this->dim0 < packets_size)
		{
			// print error
			Err->Print(ErrorPrinter::TOO_MANY_INITIALIZERS);
			return 0;
		}
		l = (this->dim0)*segment;
		this->init_list = new ChainExpression*[l];
	}
	this->init_list_size = l;

	for (int a = 0; a < l; a++)
		*(this->init_list + a) = 0;

	// copy to init list
	list<InitializerPacket *>::iterator it = this->packets.begin();
	for (int j = 0; j < packets_size; j++, it++){
		ChainExpression ** c = (ChainExpression **)(*it)->GetPacket();
		for (int k = 0; k < segment; k++){
			*(this->init_list + k + j*segment) = *(c + k);
		}
	}
	array_node->size = array_counter;
	array_node->num_children = num_children;
	return array_node;
}
struct initializer_node * SundInitializer::ScanArray(list<int> * dim)
{
	int array_counter = 0, num_children = 0, segment,sub_segment = 0,offset = 0;
	struct initializer_node * array_node = new struct initializer_node();
	array_node->array_list = new list<initializer_node *>();
	list<int>  new_dimension = *(dim);
	new_dimension.pop_front();
	segment = GetValue(&new_dimension);
	if (segment > 1){
		sub_segment = new_dimension.front();
	}
	
	//now, scan array
	do{
		// get first token
		scanner->GetToken();
		struct initializer_node * node;
		int node_size;
		if (*scanner->token == '{')
		{
			if (new_dimension.size() == 0){
				// print error
				Err->Print(ErrorPrinter::CANNOT_CONVERT_INIT_LIST_TO_TYPE);
				return 0;
			}
			// recursion
			node = ScanArray(&new_dimension);
			node_size = node->num_children;
		}
		else{
			scanner->Putback();
			// assigment-expresion
			ScannerState * state = new ScannerState(scanner);
			node = new struct initializer_node();
			node->chain = parser->PerformExp(state);
			node_size = 1;
		}
		
		if (node->num_children > 1 || offset == sub_segment)
		{
			// reset offset
			offset = 0;
		}
		if (offset+node_size > sub_segment && sub_segment)
		{
			// print error
			Err->Print(ErrorPrinter::TOO_MANY_INITIALIZERS);
			return 0;
		}
		offset += node_size;
		num_children += node_size;
		// now, get ','
		scanner->GetToken();
		array_node->array_list->push_back(node);
		array_counter++;
	} while (*scanner->token == ',');

	if (*scanner->token != '}'){
		// print error
		Err->Print(ErrorPrinter::SYNTAX);
		return 0;
	}

	array_node->size = array_counter;
	array_node->num_children = num_children;
	return array_node;
}
void SundInitializer::Build()
{
	//Only an initializer-list with zero or one elements can be converted to this type
	//int z1[][10][5][8] = { { 1 }, { 2 } };
	//int z2[1][1][1][2] = {  1 ,  2  };
	//int arr1[][3][4] = { { { 4, 11, 5 }, { 9, 2 }, 2, 5, 5 }, { 3, 4 }, 1, 1, 1, 1, { 5, 6, 7 }, 4, 4, 5, 6, { 5, 7 }, { 3, 3, 41 }, 7 };
	//int arr1[][4] = { 1, 1, 1, 1, { 5,  7, 8 , 9 }, { 9, 16 }, 2, 4, 5, 8 };
	//int arr2[][4] = { 1, 1, 1, 1, { 5, { 6, 7 }, 5, 6 }, { 9, 16 }, 2, 4, 5, 8 };
	//int arr2[] = { 5, 7, { 9, 16 }, 8, 9 };
	//int value = arr1[2][1][2];
	//int arr1[][3][4] = { { { 4, 11, 5 }, { 9, 2 }, 2, 5, 5 }, { 3, 4 }, 4, 4, 4, 4, 4, 4, 4, 4 };// , { 8, 9 }, 1, 1, 1, 1, { 5, 6 }, 2, 4, 5, 8, { 5, 7 }, 6, 6, 6, 6, { 3, 3, 41 }, 7};
	//int arr1[][3][4] = { { { 4, 11, 5 }, { 9, 2 }, 2, 5, 5 },  { 3, 4, 4, 4 }, { 4, 4 }, 4, 4, 4, 4};
	//int arr1[][3][4] = { { { 4, 11, 5 }, { 9, 2 }, 2, 5, 5 }, { 3, 4 }, 1, 1, 1, 1, { 5, 6 }, 2, 4, 5, 8, { 5, 7 }, 6, 6, 6, 6, { 3, 3, 41 }, 7 };
	int arr1[][3][4] = { { { 4, 11, 5 }, { 9, 2 }, 2, 5, 5 }, { 3, 4 }, 1, 1, 1, 1, { 5, 6 }, 2, 4, 5, 8, { 5, 7 }, 6, 6, 6, 6, { 3, 3, 41 }, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	//char w[] = { '1', '2', 'd' };
	ScannerState * temp = new ScannerState(scanner);
	scanner->GetToken();
	if (*scanner->token == '{')
	{ 
		this->root_node = ScanRootArray();
	}
	else
	{
		//scanner->Putback();
		scanner->Revert(*temp);
		// assigment-expresion
		this->root_node = new struct initializer_node();
		ScannerState * state = new ScannerState(scanner);
		root_node->chain = parser->PerformExp(state);
		this->init_list = new ChainExpression*[1];
		this->init_list[0] = root_node->chain;
		this->init_list_size = 1;
	}
	scanner->GetToken();
}
int SundInitializer::GetValue(list<int> * dim)
{
	int result = 1;
	list<int>::iterator it;
	for (it = dim->begin(); it != dim->end(); it++){
		result = result*(*it);
	}
	return result;
}
SundInitializer::~SundInitializer()
{
}
