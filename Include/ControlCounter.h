#pragma once

#define ProgramControlCounter ControlCounter::GetInstance() 

class ControlCounter
{
private:
	int block_counter,if_counter,while_counter,do_counter,switch_counter,for_counter,string_counter;
	static ControlCounter * instance;
public:
	ControlCounter(void);
	static ControlCounter * GetInstance();
	/*void IncIfCounter(){++if_counter;}
	void IncWhileCounter(){++while_counter;}
	void IncDoCounter(){++do_counter;}
	void IncSwitchCounter(){++switch_counter;}
	void IncForCounter(){++for_counter;}*/
	void ResetAll();
	~ControlCounter(void);

	friend class ProgramControlBase;
	/*friend class IfControl;
	friend class WhileControl;
	friend class DoControl;
	friend class SwitchControl;
	friend class ForControl;*/
};
