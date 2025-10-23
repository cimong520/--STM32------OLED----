#ifndef __TASK_H
#define __TASK_H

#include "../Start/stm32f10x.h"


// ����ȫ�ֱ�������task.c�ж��壩
extern volatile uint32_t system_time;   // ϵͳʱ�����������λ�����룩
extern uint16_t task_ready_bitmap;      // �������λͼ��ÿλ��Ӧһ������
extern uint16_t task_active_bitmap;     // ���񼤻�λͼ��ÿλ��Ӧһ������

// ����ID���Ͷ��壨0-9Ϊ��ЧID��0xFF��ʾ��ЧID��
typedef uint8_t TaskID;
#define INVALID_TASK_ID 0xFF

// �������ȼ����壨�Ӹߵ��ͣ�
typedef enum {
    PRIORITY_CRITICAL = 0,  // ������ȼ���ʵʱ��Ҫ��ߵ�����
    PRIORITY_HIGH,          // �����ȼ�����Ҫ����
    PRIORITY_NORMAL,        // ��ͨ���ȼ���һ������
    PRIORITY_LOW,           // �����ȼ�����̨����
    PRIORITY_COUNT          // ���ȼ��������ڲ�ʹ�ã�
} TaskPriority;

// ����״̬����
typedef enum {
    TASK_READY,     // ����������ȴ�ִ�У�
    TASK_RUNNING,   // ��������ִ����
    TASK_SUSPENDED  // ������𣨲�������ȣ�
} TaskState;

// ������ƿ� (TCB) - �洢�����������Ϣ
typedef struct {
    void (*task_func)(void);  // ������ָ�루�޲����޷���ֵ��
    uint32_t interval;        // ִ�м�������룩��0��ʾÿ��ѭ����ִ��
    uint32_t last_run;        // �ϴ�ִ��ʱ�䣨ϵͳʱ�����
    uint32_t max_exec_time;   // ���ִ��ʱ�䣨�������ܼ�أ�
    TaskState state;          // ��ǰ����״̬
    const char *name;         // �������ƣ������ã�
    TaskPriority priority;    // �������ȼ�
} Task_t;

/********************* �������ӿ� *********************/

/**
 * @brief ��ʼ������ϵͳ
 * @note ����������κ�����ǰ����
 */
void Task_Init(void);

/**
 * @brief ���������
 * @param task_func ������ָ�루void func(void)��ʽ��
 * @param interval ִ�м�������룩��0��ʾÿ�ε���ѭ����ִ��
 * @param priority �������ȼ�
 * @param name �������ƣ����ڵ��ԣ�
 * @return TaskID ���������ID��INVALID_TASK_ID��ʾ���ʧ�ܣ�
 * 
 * @example 
 * TaskID myTask = Task_Add(MyTaskFunc, 100, PRIORITY_NORMAL, "MyTask");
 */
TaskID Task_Add(void (*task_func)(void), 
               uint32_t interval, 
               TaskPriority priority,
               const char *name);

/**
 * @brief ɾ������
 * @param id Ҫɾ��������ID
 * 
 * @note ɾ����������ִ�У�ID�ɱ�����������
 */
void Task_Remove(TaskID id);

/**
 * @brief ��������
 * @param id Ҫ���������ID
 * 
 * @note �����������״̬����ͨ��Task_Resume�ָ�
 */
void Task_Suspend(TaskID id);

/**
 * @brief �ָ����������
 * @param id Ҫ�ָ�������ID
 * 
 * @note �ָ������������������ȣ�����last_runʱ�䣩
 */
void Task_Resume(TaskID id);

/**
 * @brief �޸�����ִ�м��
 * @param id ����ID
 * @param new_interval �µ�ִ�м�������룩
 */
void Task_ChangeInterval(TaskID id, uint32_t new_interval);

/**
 * @brief �޸��������ȼ�
 * @param id ����ID
 * @param new_priority �µ����ȼ�
 */
void Task_ChangePriority(TaskID id, TaskPriority new_priority);

/**
 * @brief �������������
 * @note ����ѭ���в��ϵ��ô˺���
 */
void Task_RunScheduler(void);

/**
 * @brief ��ȡ��ǰϵͳʱ��
 * @return uint32_t ϵͳʱ�䣨���룩
 */
static inline uint32_t Task_GetSystemTime(void) {
    return system_time;
}

/**
 * @brief ��������Ƿ����
 * @param task_id ����ID
 * @return uint8_t 1=������0=δ����
 * 
 * @note �ڲ�ʹ�ã�ͨ������Ҫֱ�ӵ���
 */
static inline uint8_t Task_IsReady(uint8_t task_id) {
    return (task_ready_bitmap & (1 << task_id)) != 0;
}

/**
 * @brief ������������־
 * @param task_id ����ID
 * 
 * @note �ڲ�ʹ�ã�����ִ�к��Զ�����
 */
static inline void Task_ClearReadyFlag(uint8_t task_id) {
    task_ready_bitmap &= ~(1 << task_id);
}

/**
 * @brief ϵͳ�������£���SysTick�жϵ��ã�
 * @note ÿ�������һ�Σ�����ϵͳʱ�������״̬
 */
void Task_UpdateTick(void);

#endif
