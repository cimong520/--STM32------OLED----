#include "task.h"
#include <string.h>

// ��������������������������
#define MAX_TASKS 10

// �������飨�洢����������Ϣ��
Task_t tasks[MAX_TASKS];

// ȫ��ϵͳʱ�䣨ÿ���������
volatile uint32_t system_time = 0;

// ����λͼ����Ч��������״̬��
static uint16_t task_ready_bitmap = 0;   // ����λͼ��1=��Ҫִ�У�
static uint16_t task_active_bitmap = 0;  // ����λͼ��1=������ڣ�

/**
 * @brief ��ʼ������ϵͳ
 * 
 * ���ܣ�
 * 1. ������������
 * 2. ����ϵͳʱ��
 * 3. �������λͼ
 * 
 * ע�⣺�������������ǰ����
 */
void Task_Init(void) {
    memset(tasks, 0, sizeof(tasks));
    system_time = 0;
    task_ready_bitmap = 0;
    task_active_bitmap = 0;
}

/**
 * @brief ���������
 * 
 * @param task_func ������ָ��
 * @param interval ִ�м�������룩
 * @param priority �������ȼ�
 * @param name ��������
 * @return TaskID ���������ID��0-9����INVALID_TASK_ID��ʧ�ܣ�
 * 
 * �������̣�
 * 1. ���ҵ�һ����������ۣ�δ���������
 * 2. ��ʼ��������ƿ�
 * 3. �������񼤻�λ
 * 4. ��������ID
 */
TaskID Task_Add(void (*task_func)(void), 
                uint32_t interval, 
                TaskPriority priority,
                const char *name) 
{
    // ������������ۣ�0��MAX_TASKS-1��
    for (TaskID i = 0; i < MAX_TASKS; i++) {
        // ���������Ƿ���У�δ���
        if (!(task_active_bitmap & (1 << i))) {
            // ��ʼ��������ƿ�
            tasks[i] = (Task_t){
                .task_func = task_func,
                .interval = interval,
                .last_run = system_time,  // ��¼��ǰʱ��
                .max_exec_time = 0,       // ��ʼ���ִ��ʱ��
                .state = TASK_READY,      // ��ʼ״̬Ϊ����
                .priority = priority,
                .name = name
            };
            
            // �������񼤻�λ
            task_active_bitmap |= (1 << i);
            
            // ��������ID
            return i;
        }
    }
    return INVALID_TASK_ID; // �޿��ò�λ
}

/**
 * @brief ɾ������
 * @param id Ҫɾ��������ID
 * 
 * ���ܣ�
 * 1. ������񼤻�λ
 * 2. ����������λ
 * 3. ��������״̬Ϊ����
 */
void Task_Remove(TaskID id) {
    if (id >= MAX_TASKS) return; // ���ID��Ч��
    
    // �������λ�������ٲ�����ȣ�
    task_active_bitmap &= ~(1 << id);
    
    // �������λ����������ھ������У�
    task_ready_bitmap &= ~(1 << id);
    
    // ��������״̬Ϊ����
    tasks[id].state = TASK_SUSPENDED;
}

/**
 * @brief ��������
 * @param id Ҫ���������ID
 * 
 * ���ܣ�
 * 1. ��������״̬Ϊ����
 * 2. ����������λ
 */
void Task_Suspend(TaskID id) {
    if (id >= MAX_TASKS) return;
    
    tasks[id].state = TASK_SUSPENDED;
    task_ready_bitmap &= ~(1 << id);
}

/**
 * @brief �ָ����������
 * @param id Ҫ�ָ�������ID
 * 
 * ���ܣ�
 * 1. ��������״̬Ϊ����
 * 2. �����ϴ�ִ��ʱ�䣨��������ִ�У�
 */
void Task_Resume(TaskID id) {
    if (id >= MAX_TASKS) return;
    
    tasks[id].state = TASK_READY;
    tasks[id].last_run = system_time; // ����ʱ�䣬��������ִ��
}

/**
 * @brief �޸�����ִ�м��
 * @param id ����ID
 * @param new_interval �µ�ִ�м�������룩
 * 
 * ���ܣ�
 * 1. ����������
 * 2. �����ϴ�ִ��ʱ�䣨ȷ���¼��������Ч��
 */
void Task_ChangeInterval(TaskID id, uint32_t new_interval) {
    if (id >= MAX_TASKS) return;
    
    tasks[id].interval = new_interval;
    tasks[id].last_run = system_time; // ����ʱ�䣬�¼��������Ч
}

/**
 * @brief �޸��������ȼ�
 * @param id ����ID
 * @param new_priority �µ����ȼ�
 */
void Task_ChangePriority(TaskID id, TaskPriority new_priority) {
    if (id >= MAX_TASKS) return;
    tasks[id].priority = new_priority;
}

/**
 * @brief �����������������ѭ���е��ã�
 * 
 * �������̣�
 * 1. ���û�о�������ֱ�ӷ���
 * 2. �����ȼ��Ӹߵ��ͱ���
 * 3. ��ÿ�����ȼ����ڣ�������������
 * 4. ִ�з�������������
 *    - �����Ѽ���
 *    - �����Ѿ���
 *    - �������ȼ�ƥ�䵱ǰ��
 *    - ����״̬Ϊ����
 * 5. ִ�����񲢼�¼ִ��ʱ��
 */
void Task_RunScheduler(void) {
    // ���û�о�������ֱ�ӷ���
    if (task_ready_bitmap == 0) return;
    
    // �����ȼ�����ִ�У��Ӹߵ��ͣ�
    for (TaskPriority prio = PRIORITY_CRITICAL; prio < PRIORITY_COUNT; prio++) {
        // �������������
        for (TaskID i = 0; i < MAX_TASKS; i++) {
            // ��������Ƿ񣺼�����������ȼ�ƥ�䡢״̬����
            if ((task_active_bitmap & (1 << i)) && 
                (task_ready_bitmap & (1 << i)) &&
                tasks[i].priority == prio &&
                tasks[i].state == TASK_READY) 
            {
                // ���������־����ֹ�ظ�ִ�У�
                Task_ClearReadyFlag(i);
                
                // ��������״̬Ϊ������
                tasks[i].state = TASK_RUNNING;
                
                // ��¼����ʼʱ��
                uint32_t start_time = system_time;
                
                // ִ��������
                tasks[i].task_func();
                
                // ���㲢�������ִ��ʱ��
                uint32_t exec_time = system_time - start_time;
                if (exec_time > tasks[i].max_exec_time) {
                    tasks[i].max_exec_time = exec_time;
                }
                
                // ��������״̬��ʱ��
                tasks[i].last_run = system_time;
                tasks[i].state = TASK_READY;
            }
        }
    }
}

/**
 * @brief ϵͳ�������£�ÿ�������һ�Σ�
 * 
 * �������̣�
 * 1. ����ϵͳʱ�䣨����32λ�����
 * 2. �������м��������
 * 3. ��������ϴ�ִ�е�ʱ��
 * 4. ���ʱ�����ѵ���������������þ���λ
 */
void Task_UpdateTick(void) {
    // ����ϵͳʱ�䣨��ȫ����32λ�����
    if (system_time < 0xFFFFFFFF) {
        system_time++;
    } else {
        system_time = 0; // �������������
    }

    // �������������
    for (TaskID i = 0; i < MAX_TASKS; i++) {
        // ֻ�����������
        if (task_active_bitmap & (1 << i)) {
            // ��������ϴ�ִ�е�ʱ�䣨��ȫ����ʱ����ƣ�
            uint32_t elapsed;
            if (system_time >= tasks[i].last_run) {
                elapsed = system_time - tasks[i].last_run;
            } else {
                // ������������Ƶ����
                elapsed = 0xFFFFFFFF - tasks[i].last_run + system_time + 1;
            }
            
            // ����Ƿ񵽴�ִ�м��������״̬Ϊ����
            if (elapsed >= tasks[i].interval && tasks[i].state == TASK_READY) {
                // �����������λ
                task_ready_bitmap |= (1 << i);
            }
        }
    }
}
