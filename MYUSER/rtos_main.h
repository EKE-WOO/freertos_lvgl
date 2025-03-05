#ifndef _RTOS_MAIN_H_
#define _RTOS_MAIN_H_





//硬件初始化
void Hart_init(void);
//软件初始化，所有的新任务都在这里
void Soft_init(void);
//队列初始化
void Queue_init(void);

#endif
