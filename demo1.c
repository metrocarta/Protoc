#define B 23
#define SIZE 10
#define A 3
#define YES 1
#define NO  0

char d=9;
char arr[SIZE];
inline int test1(int a1,int a2);
int test2(int a1,int a2);
void main()
{
	int a = 8 , b = B,  c;
	a = b;
	for(b=0;b <= test1(5,6);++b){
		++a;
	}
}
inline int test1(int a1,int a2)
{
	int value = (a1-a2);
	return (a1*a2);
}
int test2(int a1,int a2)
{
	return (a1/a2);
}


