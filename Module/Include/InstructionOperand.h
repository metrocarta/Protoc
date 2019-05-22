#pragma once
#define OPERAND_NAME_LEN 80
class ChainExpression;
class InstructionOperand
{
private:
	wchar_t var_name[OPERAND_NAME_LEN];
public:
	//InstructionOperand();
	InstructionOperand(ChainExpression *chain);
	~InstructionOperand();
};

