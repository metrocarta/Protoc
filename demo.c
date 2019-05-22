word c = 0x1245;
int sub(int a, int b);
int add(int a, int b);
int f(void);

void main(){
	char d = '\t';
	int  s = (4 - 2 + 4 + 20);
	int r = (5 + 6);
	byte arr[] = { 1, 2, 0 };
	s = (s + 1)*(s - 1)/(s-6);
	if (s <= r){
		s = add(r, s);
	}
	if (s > 30)
		s = 1;
}
int add(int a, int b)
{
	int tmp1;
	tmp1 = b;
	return a + b;
}

int sub(int a, int b)
{
	int tmp2;
	tmp2 = b;
	return a - b;
}
int f(void) {
}

