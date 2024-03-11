#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread* current_thread = NULL;
static int id = 1;
// static jmp_buf env_st;
// static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){//得到新的thread
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    printf("address: 0x%x\n",&f);
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;
    return t;
}
void thread_add_runqueue(struct thread *t){//加進到ranqueue裏面，就可以執行thread_start
    if(current_thread == NULL){//current_thread設置為t，加一些相對應的設定
        current_thread=t;
        t->next=t;
        t->previous=t;
    }
    else{//t加到current_thread前面
        t->next=current_thread;
        t->previous=current_thread->previous;
        current_thread->previous->next=t;
        current_thread->previous=t;
    }
    // printf("add_succes!\n");
}
void thread_yield(void){//當前thread正式中斷
    //setjump 把thread狀態儲存起來
    //schedule() to determine which thread to run next and dispatch() to excute the next thread.
    //if the thread is resumed later, it should return to the calling place in the function
    printf("before set: ra %d, sp %d \n",(unsigned long) current_thread->env[0].ra,current_thread->env[0].sp);
    // memcpy(env_tmp,current_thread->env,sizeof(jmp_buf));
    // printf("tmp.ra %d,tmp.sp %d \n",(unsigned long) env_tmp[0].ra,env_tmp[0].sp);
    if(setjmp(current_thread->env)==0){ // (setjmp(current_thread->env) == 0) {
        // Save current context and switch to next thread
        printf("after set: ra %d,sp %d \n",(unsigned long) current_thread->env[0].ra,current_thread->env[0].sp);

        printf("save thread %d\n",current_thread->ID);
        // current_thread->buf_set = 1;
        schedule();
        dispatch();
        return;
    }
    // printf("stack_p %d, sp %d \n",(unsigned long) current_thread->stack_p,current_thread->env[0].sp);
    else{
        // memcpy(current_thread->env,env_tmp,sizeof(jmp_buf));
        printf("after longjmp\n");
        printf("sp %lu,stack_p %lu,ID %lu\n",current_thread->env[0].sp,(unsigned long)current_thread->stack_p,current_thread->ID);
        printf("ra %lu,fp %lu,ID %lu\n",current_thread->env[0].ra,(unsigned long)current_thread->fp,current_thread->ID);

    
    // current_thread->fp(current_thread->arg)
    // printf("%d,%d\n",current_thread->ID,current_thread->next->ID);
        return;
    }
    
}
void dispatch(void){//讓下一個thread去執行
    // TODO
    //如果thread沒有跑過，要初始化（move the stack pointer）
    //如果thread有跑過，代表用過setjump，所以只要用longjump恢復他的狀態就可以了
    //如果thread function如果沒有呼叫thread_exit()而是直接return，那需要從ranqueue被移除，讓下一個準備執行（呼叫thread_exit()?）

    if (!current_thread->buf_set) {
        // // If the thread has never run before
        current_thread->env[0].ra=(unsigned long) current_thread->fp;
        current_thread->env[0].sp=(unsigned long) current_thread->stack_p;
        // memcpy(env_st,current_thread->env,sizeof(jmp_buf));
        // memcpy(env_tmp,current_thread->env,sizeof(jmp_buf));
        // current_thread->env[0]=env_st[0];
        // current_thread->env[0].sp = (unsigned long) current_thread->stack_p;
        // current_thread->env[0].ra=(unsigned long) current_thread->fp;
        printf("initializa: stack_p %lu, sp %lu , ra %lu\n",(unsigned long) current_thread->stack_p,current_thread->env[0].sp,(unsigned long)current_thread->fp);
        current_thread->buf_set = 1;
        current_thread->fp(current_thread->arg);
        // If the thread function returns, it should exit
        thread_exit();
    } else {
        printf("before longjmp: sp %lu,stack_p %lu,ID %lu\n",current_thread->env[0].sp,(unsigned long)current_thread->stack_p,current_thread->ID);
        printf("before longjmp: ra %lu,fp %lu,ID %lu\n",current_thread->env[0].ra,(unsigned long)current_thread->fp,current_thread->ID);
        // Restore the context
        longjmp(current_thread->env,1);//longjmp(current_thread->env,1);
    }
   
    // Update current_thread to the next thread
    schedule();
}
void schedule(void){//決定下一個thread
    // TODO
    current_thread=current_thread->next;
    printf("now thread: %d\n",current_thread->ID);

}
void thread_exit(void){
    //完成工作呼叫，移除ranqueue當前thread，free its stack and the entire thread structure
    //update current_thread(just call schedule())
    //讓下一個thread準備執行（dispatch()

    if(current_thread->next != current_thread){
        current_thread->previous->next = current_thread->next;
        current_thread->next->previous = current_thread->previous;
        schedule();

        // Free the stack
        free(current_thread->stack);
        // Free the thread structure
        free(current_thread);

        dispatch();
    }
    else{
        free(current_thread->stack);
        free(current_thread);
        exit(0);
        // Hint: No more thread to execute
    }
}
void thread_start_threading(void){//從main function被呼叫
    //it should return only if all threads have exited
    //(thread_exit()->thread_starting_threading()->mainfunction)
    // while (current_thread != current_thread->next) {
    //     dispatch();
    // }
    while (current_thread != NULL) {
        dispatch();
    }
}

// part 2
void thread_assign_task(struct thread *t, void (*f)(void *), void *arg){
    // TODO
}
