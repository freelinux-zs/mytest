#include <iostream>
#include <cstddef>
struct node{
	int payload;
	node *next;
};
int main(){
	node* head = nullptr;
	
	for(int i = 0; i < 10; i++){
		node* new_node = new node;
		new_node->payload = i * 10;
		new_node->next = head;
		head = new_node;
	}

	node* iterator = head;
	while(iterator){
		std::cout << iterator->payload << std::endl;
		iterator = iterator->next;
	}
	return 0;
}
