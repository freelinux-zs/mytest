#include <iostream>

void reverse(int[], int);
void output(int[], int);

int main(){
	int array[] = {5, 4, 10, 1, 7, 9, 2};
	reverse(array, 7);
	output(array, 7);
	return 0;
}

void reverse(int array[], int length){
	int left = 0;
	int right = length - 1;
	std::cout <<"left = " << left << "\nlength = " << length <<std::endl;
	while(left < right){
		int temp = array[left];
		array[left] = array[right];
		array[right] = temp;
		left ++;
		right --;
	}
}

void output(int array[], int length){
	for(int i = 0; i < length; i++)
		std::cout << array[i] << std::endl;
}


