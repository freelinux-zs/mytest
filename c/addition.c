#include <stdio.h>

int add(int a,int b){
	if(a!=0&&b!=0)
		return printf("%*c%*c",a,'r',b,'r');
	else
		return a!=0?a:b;
}

int Add(int x, int y)
{
	if (y == 0)
		return x;
	else
		return Add( x ^ y, (x & y) << 1);
}

int main(){
	int A = 0, B = 0, C = 0;
	printf("Enter the two numbers to addn ");
	scanf("%d %d",&A,&B);
	printf("Required sum is %d\n",Add(A,B));
	return 0;
}
