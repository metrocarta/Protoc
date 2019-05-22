#include "../Include/ControlCounter.h"

ControlCounter * ControlCounter::instance = 0;
ControlCounter::ControlCounter(void)
{
	this->ResetAll();

	instance = this;
}
void ControlCounter::ResetAll()
{
	if_counter = 0;
	while_counter = 0;
	do_counter = 0;
	switch_counter = 0;
	for_counter = 0;
	block_counter = 0;
	string_counter = 0;
}
ControlCounter * ControlCounter::GetInstance()
{
	return instance;
}
ControlCounter::~ControlCounter(void)
{
}
