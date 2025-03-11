#include "msstd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <paths.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <assert.h>
#include <sys/time.h>

int ms_task_set_name(const char* name)
{
	char thread_name[128] = {0};
	if (!name) 
		return -1;
	snprintf(thread_name, sizeof(thread_name), "%s(%ld)", name, syscall(SYS_gettid));
	prctl(PR_SET_NAME, thread_name, 0, 0, 0);
	return 0;
}

int ms_task_create(TASK_HANDLE *handle, int size, void *(*func)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (size >= THREAD_STACK_MIN_SIZE)
        pthread_attr_setstacksize(&attr, size);
	else
		pthread_attr_setstacksize(&attr, THREAD_STACK_MIN_SIZE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
		pthread_attr_destroy(&attr);
        return TASK_ERROR;
    }
	pthread_detach(*handle);
    pthread_attr_destroy(&attr);
    return TASK_OK;	
}

int ms_task_create_join(TASK_HANDLE *handle, void *(*func)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr, THREAD_STACK_1M_SIZE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
		pthread_attr_destroy(&attr);
        return TASK_ERROR;
    }

    pthread_attr_destroy(&attr);
    return TASK_OK;	
}

int ms_task_create_join_stack_size(TASK_HANDLE *handle, int size, void *(*func)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (size >= THREAD_STACK_MIN_SIZE)
        pthread_attr_setstacksize(&attr, size);
	else
		pthread_attr_setstacksize(&attr, THREAD_STACK_MIN_SIZE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
		pthread_attr_destroy(&attr);
        return TASK_ERROR;
    }

    pthread_attr_destroy(&attr);
    return TASK_OK;	
}

int ms_task_join(TASK_HANDLE *handle)
{
    int ret;
	if(!handle)
		return TASK_ERROR;
    ret = pthread_join(*handle, NULL);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        return TASK_ERROR;
    }

    return TASK_OK;	
}

int ms_task_detach(TASK_HANDLE *handle)
{
	return pthread_detach(*handle);
}

int ms_task_cancel(TASK_HANDLE *handle)
{
	return pthread_cancel(*handle);
}

//void inline ms_task_quit()
//{
//	pthread_exit(NULL);
//}

//mutext
int ms_mutex_init(MUTEX_OBJECT *mutex)
{
    int ret = -1;
#ifdef THREAD_MUTEX_RECURSIVE
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr); 
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP); 
    ret = pthread_mutex_init(mutex, &attr);
	pthread_mutexattr_destroy(&attr);
#else
	ret = pthread_mutex_init(mutex, NULL);
#endif
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        return TASK_ERROR;
    }

    return TASK_OK;
}
int ms_mutex_trylock(MUTEX_OBJECT *mutex)
{
    return pthread_mutex_trylock(mutex);
}

int ms_mutex_timelock(MUTEX_OBJECT *mutex, int us)
{
    struct timespec tout;
    struct timeval now;
	
    gettimeofday(&now, NULL);
    now.tv_usec += us;
    if(now.tv_usec >= 1000000)
    {
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
    }
    tout.tv_nsec = now.tv_usec * 1000;
    tout.tv_sec = now.tv_sec;
    return pthread_mutex_timedlock(mutex, &tout);
}

void ms_mutex_lock(MUTEX_OBJECT *mutex)
{
    pthread_mutex_lock(mutex);
}

void ms_mutex_unlock(MUTEX_OBJECT *mutex)
{
    pthread_mutex_unlock(mutex);
}

void ms_mutex_uninit(MUTEX_OBJECT *mutex)
{
    pthread_mutex_destroy(mutex);
}

void ms_rwlock_init(RWLOCK_BOJECT *rwlock)
{
	int ret;
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	// do not share lock(PTHREAD_PROCESS_SHARED) with child process. solin
	pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
	ret = pthread_rwlock_init(rwlock, &attr);
    if (ret)
		printf("%s Failed: %d\n", __FUNCTION__, ret);
    pthread_rwlockattr_destroy(&attr);
}

void ms_rwlock_uninit(RWLOCK_BOJECT *rwlock)
{
    pthread_rwlock_destroy(rwlock);
}

int ms_rwlock_rdlock(RWLOCK_BOJECT *rwlock)
{
    return pthread_rwlock_rdlock(rwlock);
}

int ms_rwlock_wrlock(RWLOCK_BOJECT *rwlock)
{
    return pthread_rwlock_wrlock(rwlock);
}

int ms_rwlock_tryrdlock(RWLOCK_BOJECT *rwlock)
{
    return pthread_rwlock_tryrdlock(rwlock);
}

int ms_rwlock_trywrlock(RWLOCK_BOJECT *rwlock)
{
    return pthread_rwlock_trywrlock(rwlock);
}

int ms_rwlock_unlock(RWLOCK_BOJECT *rwlock)
{
    return pthread_rwlock_unlock(rwlock);
}


void ms_barrier_init(BARRIER_OBJECT *barrier, unsigned int count)
{
	pthread_barrier_init(barrier, NULL, count);
}

void ms_barrier_uninit(BARRIER_OBJECT *barrier)
{
	pthread_barrier_destroy(barrier);
}

int ms_barrier_wait(BARRIER_OBJECT * barrier)
{
	return pthread_barrier_wait(barrier);
}

//system() function
int ms_system(const char* cmd)
{
	if (!cmd)
		return -1;
   	int res = 0;
	int status = 0;
	pid_t pid = vfork();
	struct rusage info;

	memset(&info, 0, sizeof(struct rusage));
	if (pid == 0)
	{
		//printf("child run.\n");
		execl("/bin/sh", "/bin/sh", "-c", cmd, (char*) NULL);
		//printf("child finished.\n");
		_exit(1);
	} 
	else if (pid > 0) 
	{
		for(;;)
		{
			res = wait4(pid, &status, 0, &info);//wait4 base waitpid,wait4 more advanced
			if (res > -1)
			{
				res = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
				break;
			} else if (errno != EINTR) 
				break;
		}
		//printf("parent finished.\n");
	}
	else
	{
		res = -1;
	}
	return res;
}

int ms_vsystem(const char *cmd)
{
	if (!cmd)
		return -1;
    int res = 0;
	int status = 0;
	pid_t pid = vfork();
	struct rusage info;

	memset(&info, 0, sizeof(struct rusage));
	if (pid == 0) {
		execl(_PATH_BSHELL, "sh", "-c", cmd, (char *) NULL);
		_exit(1);
	} 
	else if (pid > 0)
	{
		for(;;) 
		{
			res = wait4(pid, &status, 0, &info);
			if (res > -1)
			{
				res = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
				break;
			} 
			else if (errno != EINTR)
				break;
		}
	}
	else
	{
		res = -1;
	}
	return res;
}

static int close_all_fd()
{
    struct rlimit rl;
    unsigned int i;
    
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("getrlimit err, errno = %d, strerr = %s\n", errno, strerror(errno));
        return -1;
    }
    
    if(rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 4096;
    }
    
    for(i = 0; i < rl.rlim_max; ++i) {
        if (i == STDIN_FILENO || i == STDOUT_FILENO || i == STDERR_FILENO) {
            continue;
        }
        close(i);
    }

    return 0;
}

// different from @ms_system(): this func will close all fd's inherited from the parent process
int ms_system_closefd(const char* cmd)
{
    if (!cmd) {
        return -1;
    }
    
    int res = 0;
    int status = 0;
    pid_t pid = vfork();
    struct rusage info;

    memset(&info, 0, sizeof(struct rusage));
    if (pid == 0) {
        close_all_fd();
        //printf("child run.\n");
        execl("/bin/sh", "/bin/sh", "-c", cmd, (char*) NULL);
        //printf("child finished.\n");
        _exit(1);
    } else if (pid > 0) {
        for(;;) {
            res = wait4(pid, &status, 0, &info);//wait4 base waitpid,wait4 more advanced
            if (res > -1) {
            	res = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            	break;
            } else if (errno != EINTR) {
                break;
            }
        }
        //printf("parent finished.\n");
        } else {
            res = -1;
        }

        return res;
}

//###################################################
struct pid 
{
	struct pid *next;
	FILE *fp;
	pid_t pid;
};

static pid_t *childpid = NULL;
FILE * ms_vpopen(const char *program, const char *type)
{
	FILE *iop;
	int pdes[2];
	pid_t pid;

	if ((*type != 'r' && *type != 'w') || type[1] != '\0') {
		errno = EINVAL;
		return (NULL);
	}

	if (!childpid) {
		childpid = calloc(1, sizeof(pid)*getdtablesize());
		if (!childpid)
			return NULL;
	}
	if (pipe(pdes) < 0) {
		return (NULL);
	}

	switch (pid = vfork()) {
	case -1:			/* Error. */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		return (NULL);
	case 0:				/* Child. */
	    {
			if (*type == 'r') {
				int tpdes1 = pdes[1];

				(void) close(pdes[0]);
				if (tpdes1 != STDOUT_FILENO) {
					(void)dup2(tpdes1, STDOUT_FILENO);
					(void)close(tpdes1);
					tpdes1 = STDOUT_FILENO;
				}
			} else {
				(void)close(pdes[1]);
				if (pdes[0] != STDIN_FILENO) {
					(void)dup2(pdes[0], STDIN_FILENO);
					(void)close(pdes[0]);
				}
			}
			execl(_PATH_BSHELL, "sh", "-c", program, (char *)NULL);
			_exit(127);
	    }
	}

	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}

	childpid[fileno(iop)] = pid;

	return (iop);
}

int ms_vpclose(FILE *iop)
{
	int pstat;
	pid_t pid;
	int fd = 0;
	int ret = -1;

	if (!childpid) return -1;
	if (!iop) return -1;
	fd = fileno(iop);
	pid = childpid[fd];
	if (pid == 0) return -1;
	(void)fclose(iop);

	do 
	{
		ret = waitpid(pid, &pstat, 0);
	} while (ret == -1 && errno == EINTR);
	childpid[fd] = 0;

	return (ret == -1 ? -1 : pstat);
}

/////////////////////////////////
//list
//link table
int ms_list_init (ms_list_t * li)
{
  if (li == NULL)
    return -1;
  memset (li, 0, sizeof (ms_list_t));
  return 0;                     /* ok */
}

void ms_list_special_free (ms_list_t * li, void *(*free_func) (void *))
{
  int pos = 0;
  void *element;

  if (li == NULL)
    return;
  while (!ms_list_eol (li, pos))
    {
      element = (void *) ms_list_get (li, pos);
      ms_list_remove (li, pos);
      if (free_func != NULL)
        free_func (element);
    }
  ms_free (li);
}

void ms_list_ofchar_free (ms_list_t * li)
{
  int pos = 0;
  char *chain;

  if (li == NULL)
    return;
  while (!ms_list_eol (li, pos))
    {
      chain = (char *) ms_list_get (li, pos);
      ms_list_remove (li, pos);
      ms_free (chain);
    }
  ms_free (li);
}

int ms_list_size (const ms_list_t * li)
{
  if (li != NULL)
    return li->nb_elt;
  else
    return -1;
}

int ms_list_eol (const ms_list_t * li, int i)
{
  if (li == NULL)
    return -1;
  if (i < li->nb_elt)
    return 0;                   /* not end of list */
  return 1;                     /* end of list */
}

/* index starts from 0; */
int ms_list_add (ms_list_t * li, void *el, int pos)
{
  ms_node_t *ntmp;
  int i = 0;

  if (li == NULL)
    return -1;

  if (li->nb_elt == 0)
    {

      li->node = (ms_node_t *) ms_malloc (sizeof (ms_node_t));
      if (li->node == NULL)
        return -1;
      li->node->element = el;
      li->node->next = NULL;
      li->nb_elt++;
      return li->nb_elt;
    }

  if (pos == -1 || pos >= li->nb_elt)
    {                           /* insert at the end  */
      pos = li->nb_elt;
    }

  ntmp = li->node;              /* exist because nb_elt>0  */

  if (pos == 0)                 /* pos = 0 insert before first elt  */
    {
      li->node = (ms_node_t *) ms_malloc (sizeof (ms_node_t));
      if (li->node == NULL)
        {
          /* leave the list unchanged */
          li->node = ntmp;
          return -1;
        }
      li->node->element = el;
      li->node->next = ntmp;
      li->nb_elt++;
      return li->nb_elt;
    }


  while (pos > i + 1)
    {
      i++;
      /* when pos>i next node exist  */
      ntmp = (ms_node_t *) ntmp->next;
    }

  /* if pos==nb_elt next node does not exist  */
  if (pos == li->nb_elt)
    {
      ntmp->next = (ms_node_t *) ms_malloc (sizeof (ms_node_t));
      if (ntmp->next == NULL)
        return -1;              /* leave the list unchanged */
      ntmp = (ms_node_t *) ntmp->next;
      ntmp->element = el;
      ntmp->next = NULL;
      li->nb_elt++;
      return li->nb_elt;
    }

  /* here pos==i so next node is where we want to insert new node */
  {
    ms_node_t *nextnode = (ms_node_t *) ntmp->next;

    ntmp->next = (ms_node_t *) ms_malloc (sizeof (ms_node_t));
    if (ntmp->next == NULL)
      {
        /* leave the list unchanged */
        ntmp->next = nextnode;
        return -1;
      }
    ntmp = (ms_node_t *) ntmp->next;
    ntmp->element = el;
    ntmp->next = nextnode;
    li->nb_elt++;
  }
  return li->nb_elt;
}

/* index starts from 0 */
void *ms_list_get (const ms_list_t * li, int pos)
{
  ms_node_t *ntmp;
  int i = 0;

  if (li == NULL)
    return NULL;

  if (pos < 0 || pos >= li->nb_elt)
    /* element does not exist */
    return NULL;


  ntmp = li->node;              /* exist because nb_elt>0 */

  while (pos > i)
    {
      i++;
      ntmp = (ms_node_t *) ntmp->next;
    }
  return ntmp->element;
}

int ms_list_remove (ms_list_t * li, int pos)
{
	
	ms_node_t *ntmp;
	int i = 0;
	
	if (li == NULL)
		return -1;
	
	if (pos < 0 || pos >= li->nb_elt)
		/* element does not exist */
		return -1;
	
	ntmp = li->node;              /* exist because nb_elt>0 */
	
	if ((pos == 0))
    {                           /* special case  */
		li->node = (ms_node_t *) ntmp->next;
		li->nb_elt--;
		ms_free (ntmp);
		return li->nb_elt;
    }
	
	while (pos > i + 1)
    {
		i++;
		ntmp = (ms_node_t *) ntmp->next;
    }
	
	/* insert new node */
	{
		ms_node_t *remnode;
		
		remnode = (ms_node_t *) ntmp->next;
		ntmp->next = ((ms_node_t *) ntmp->next)->next;
		ms_free (remnode);
		li->nb_elt--;
	}
	return li->nb_elt;
}

void *ms_list_get_first (ms_list_t * li, ms_list_iterator_t * iterator)
{
	if (0 >= li->nb_elt)
	{
		iterator->actual = 0;
		return 0;
	}

	iterator->actual = li->node;
	iterator->prev = &li->node;
	iterator->li = li;
	iterator->pos = 0;

	return li->node->element;
}

void *ms_list_get_next (ms_list_iterator_t * iterator)
{
  iterator->prev = (ms_node_t **) & (iterator->actual->next);
  iterator->actual = iterator->actual->next;
  ++(iterator->pos);

  if (ms_list_iterator_has_elem (*iterator))
    {
      return iterator->actual->element;
    }

  iterator->actual = 0;
  return 0;
}

void *ms_list_iterator_remove (ms_list_iterator_t * iterator)
{
  if (ms_list_iterator_has_elem (*iterator))
    {
      --(iterator->li->nb_elt);

      *(iterator->prev) = iterator->actual->next;

      ms_free (iterator->actual);
      iterator->actual = *(iterator->prev);
    }

  if (ms_list_iterator_has_elem (*iterator))
    {
      return iterator->actual->element;
    }

  return 0;
}

int ms_list_iterator_add(ms_list_iterator_t * it, void* element, int pos)
{
	if (pos == -1 || pos >= it->li->nb_elt) {                           
		pos = it->li->nb_elt;
    }
	if (ms_list_add(it->li, element, pos) < 0)
		return -1;
	if (pos <= it->pos)
		it->pos ++;
	return it->li->nb_elt;
}

void ms_pool_thread_init(ms_thread_pool *pool_task, int thread_num, void *(*pthread_main)(void *arg))
{
	if(!pool_task && thread_num <= 0){
		printf("pool thread init failed!\n");
		return ;
	}
	int i = 0;
	
	pthread_mutex_init(&(pool_task->queue_lock), NULL);
	pthread_cond_init(&(pool_task->queue_ready), NULL);

	pool_task->queue_head = NULL;
	pool_task->max_thread_num = thread_num;
	pool_task->cur_queue_size = 0;
	pool_task->shutdown = 0;

	pool_task->pthreadId = (pthread_t *)ms_malloc(thread_num * sizeof(pthread_t));

	for(i = 0; i < thread_num; i++)
	{
		pthread_create(&(pool_task->pthreadId[i]), NULL, pthread_main, (void *)&i);
	}

	return ;
}

int ms_pool_add_task(ms_thread_pool *pool_task, ms_thread_pool_task *new_task)
{
	if(!new_task)
	{
		printf("%s pool add task failed!\n", __FUNCTION__);
		return -1;
	}
	if(pool_task->cur_queue_size > MAX_THREAD_POOL_TASK)
	{
		printf("%s pool task(%d > %d) too much!\n", __FUNCTION__, pool_task->cur_queue_size, MAX_THREAD_POOL_TASK);
		return -1;
	}
	pthread_mutex_lock(&(pool_task->queue_lock));
	ms_thread_pool_task *member = pool_task->queue_head;
	
	if(member != NULL)
	{
		while(member->next != NULL)
			member = member->next;
		member->next = new_task;
	}
	else
	{
		pool_task->queue_head = new_task;
	}

	assert (pool_task->queue_head != NULL);
	
    pool_task->cur_queue_size++;
	
	pthread_cond_signal (&(pool_task->queue_ready));
    pthread_mutex_unlock (&(pool_task->queue_lock));
    
	
    return 0;
}

int ms_pool_thread_uninit(ms_thread_pool *pool_task)
{
	if(pool_task->shutdown)
		return -1;
	pool_task->shutdown = 1;
	
	
	pthread_cond_broadcast(&(pool_task->queue_ready));

	int i = 0;
	for(i = 0; i < pool_task->max_thread_num; i++)
	{
		pthread_join(pool_task->pthreadId[i], NULL);
	}
	if(pool_task->pthreadId)
	{
		ms_free(pool_task->pthreadId);
	}
	
	ms_thread_pool_task *header = NULL;
	while(pool_task->queue_head != NULL)
	{
		header = pool_task->queue_head;
		pool_task->queue_head = pool_task->queue_head->next;
		ms_free(header);
	}

	pthread_mutex_destroy(&(pool_task->queue_lock));
	pthread_cond_destroy(&(pool_task->queue_ready));
	ms_free(pool_task);
	return 0;
}

int ms_cond_init(COND_OBJECT *cond, MUTEX_OBJECT *mutex)
{
	pthread_mutex_init(mutex, NULL);
	pthread_cond_init(cond , NULL);
	return 0;
}
int ms_cond_uninit(COND_OBJECT *cond, MUTEX_OBJECT *mutex)
{
	pthread_cond_destroy(cond);
	pthread_mutex_destroy(mutex);
	return 0;
}
int ms_cond_signal(COND_OBJECT *cond, MUTEX_OBJECT *mutex)
{
	pthread_mutex_lock(mutex);
	pthread_cond_signal(cond);
	pthread_mutex_unlock(mutex);
	return 0;
}
int ms_cond_wait(COND_OBJECT *cond, MUTEX_OBJECT *mutex)
{
	pthread_mutex_lock(mutex);
	pthread_cond_wait(cond, mutex);
	pthread_mutex_unlock(mutex);
	return 0;
}

int ms_cond_wait_time(COND_OBJECT *cond, MUTEX_OBJECT *mutex, int secs)
{
	pthread_mutex_lock(mutex);
	struct timeval tv;
    struct timespec ts;
	
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + secs;
    ts.tv_nsec = tv.tv_usec * 1000;
	
	pthread_cond_timedwait(cond, mutex, &ts);
	pthread_mutex_unlock(mutex);
	return 0;
}
int ms_set_task_priority(pthread_t thread_id, int priority)
{
    int policy = SCHED_RR;
	struct sched_param param;
	if (priority <= 0) {
        msprintf("[david debug] set task priority is failed.");
        return -1;	
    }
	if (!thread_id) {
        msprintf("[david debug] set task priority is failed.");
        return -1;
    }
	memset(&param, 0, sizeof(param));
	param.sched_priority = priority;
	if (pthread_setschedparam(thread_id, policy, &param) < 0) {
        perror("pthread_setschedparam");
    }
	pthread_getschedparam(thread_id, &policy, &param);
	if (param.sched_priority != priority) {
        msprintf("[david debug] set task priority is failed.");
        return -1;
    }
	msprintf("set_realtime_cam thread:%d priority:%d", (int)thread_id, priority);
	return 0;
}
