#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include "mylist.h"
//定义app_info链表结构
typedef struct application_info
{
	uint32_t app_id;
	uint32_t up_flow;
	uint32_t down_flow;
	struct list_head app_info_node;
}app_info;

app_info* get_app_info(uint32_t app_id, uint32_t up_flow, uint32_t down_flow)
{
	app_info *app = (app_info *)malloc(sizeof(app_info));
	if(app == NULL){
		fprintf(stderr,"Fail to malloc memory, error:%u, reason:%s\r\n",errno, strerror(errno));
		return NULL;
	}

	app->app_id = app_id;
	app->up_flow = up_flow;
	app->down_flow = down_flow;

	return app;
}

static void for_each_app(const struct list_head *head)
{
	struct list_head *pos;
	app_info *app;

	//遍历链表
	list_for_each(pos, head){
		app = list_entry(pos, app_info, app_info_node);
		printf("app_id:%u\tup_flow:%u\tdown_flow :%u\r\n",app->app_id, app->up_flow, app->down_flow);
	}
}


void destory_app_list(struct list_head *head)
{
	struct list_head *pos = head->next;
	struct list_head *tmp = NULL;
	while(pos != head)
	{
		tmp = pos->next;
		list_del(pos);
		pos = tmp;
	}
}

int main(void)
{
	//创建一个app_info
	app_info *app_info_list = (app_info *)malloc(sizeof(app_info));
	app_info *app;

	if(app_info_list == NULL){
		fprintf(stderr, "Failed to malloc memory, errno: %u, reason: %s\r\n",errno, strerror(errno));
		return -1;
	}

	//初始化链表头部
	struct list_head *head  = &app_info_list->app_info_node;
	init_list_head(head);

	//插入三个app_info
	app = get_app_info(1001, 100, 200);
	list_add_tail(&app->app_info_node, head);
	app = get_app_info(1002, 80, 100);
	list_add_tail(&app->app_info_node, head);
	app = get_app_info(1003, 90, 102);
	list_add_tail(&app->app_info_node, head);

	app = get_app_info(1004, 101, 300);
	list_add(&app->app_info_node, head->next);
	printf("After insert three app_info: \r\n");
	for_each_app(head);


	//将第一个节点移到末尾
	printf("Move first node to tail:\r\n");
	list_move_tail(head->next,head);
	for_each_app(head);

	//将第一个节点下移一个位置
	printf("Mover first node to one node:\r\n");
	list_move(head->next, head->next->next);
	for_each_app(head);

	//删除最后一个节点
	printf("Del the last noe:\r\n");
	list_del(head->prev);
	for_each_app(head);

	//删除第一个节点
	printf("Del the first node:\r\n");
	list_del(head->next);
	for_each_app(head);
	destory_app_list(head);
	free(app_info_list);
	return 0;
}
