#ifndef SAVEQUEUE_H_
#define SAVEQUEUE_H_


#include <deque>
#define DEFAULT_MAX_QUEUE_SIZE 1240
template <typename T>
class safequeue
{   
public:
	safequeue()
    {
    	pthread_mutex_init (&mutex_read,  NULL);
		pthread_mutex_init (&mutex_write, NULL);
		canreadnums = 0;
		canwritenums = DEFAULT_MAX_QUEUE_SIZE;
    }

	~safequeue()
	{
    	pthread_mutex_destroy (&mutex_read);
		pthread_mutex_destroy (&mutex_write);
		c.clear();
    }
	void init(int max_queue_size)
	{
		canwritenums = max_queue_size;
	}

    bool empty()
    {
    	pthread_mutex_lock(&mutex_read);
		pthread_mutex_lock(&mutex_write);
		bool result = c.empty();
    	pthread_mutex_unlock(&mutex_read);
		pthread_mutex_unlock(&mutex_write);
		
		return result;
	}
	
	bool full() const 
	{
		return (canwritenums == 0);
	}

	void front(T& elem)
	{
		pthread_mutex_lock(&mutex_read);
		pthread_mutex_lock(&mutex_write);
		elem = c.front();
    	pthread_mutex_unlock(&mutex_read);
		pthread_mutex_unlock(&mutex_write);
	}

	void clear()
	{    	
		pthread_mutex_lock(&mutex_read);
		pthread_mutex_lock(&mutex_write);
		bool result = c.empty();
    	pthread_mutex_unlock(&mutex_read);
		pthread_mutex_unlock(&mutex_write);
	}
    
    //输入数据
    int safepush(T& elem)
    {
		pthread_mutex_lock(&mutex_write);
		if( canwritenums <= 0 ) {
			pthread_mutex_unlock(&mutex_write);
			return 1;
		}
		canwritenums--;
		c.push_back(elem);
		pthread_mutex_unlock(&mutex_write);

		pthread_mutex_lock(&mutex_read);
		canreadnums++;
		pthread_mutex_unlock(&mutex_read);

	/*
		  LONG lPrevCount;
		  WaitForSingleObject(mutex_write, INFINITE);
		  if( canwritenums <= 0 )
		  {
              ReleaseMutex(mutex_write);
			  return -1;
		  }
		  canwritenums--;
	      c.push_back(elem);
		  ReleaseMutex(mutex_write);
    
	      WaitForSingleObject(mutex_read, INFINITE);
		  canreadnums++;
		  ReleaseSemaphore(read_lock, 1, &lPrevCount);
	      ReleaseMutex(mutex_read);	
*/
		  return 0;
	}
    
    //得到数据
	int safepop(T& elem)
	{
		pthread_mutex_lock(&mutex_read);
		if (c.empty()) {
			pthread_mutex_unlock(&mutex_read);
			return -1;
		}
		/*while(canreadnums <= 0)
	      {
			  ReleaseMutex(mutex_read);	
              WaitForSingleObject(read_lock, INFINITE);
			  WaitForSingleObject(mutex_read, INFINITE);
	      }*/
		
		canreadnums--;
		elem = c.front();
		c.pop_front();
		pthread_mutex_unlock(&mutex_read);

		pthread_mutex_lock(&mutex_write);
		canwritenums++;
		pthread_mutex_unlock(&mutex_write);
		return 0;
	}

	int safeerase(const T& elem) 
	{
		int ret = -1;
		pthread_mutex_lock(&mutex_read);
		pthread_mutex_lock(&mutex_write);

		typename std::deque<T>::iterator it = c.begin();
		for (; it != c.end(); ++it) {
			if (*it == elem) {
				ret = 0;
				break;
			}
		}
		c.erase(it);
	
    	pthread_mutex_unlock(&mutex_read);
		pthread_mutex_unlock(&mutex_write);

		return ret;
	}

	bool find(const T& elem)
	{
		bool ret = false;
		pthread_mutex_lock(&mutex_read);
		pthread_mutex_lock(&mutex_write);

		typename std::deque<T>::iterator it = c.begin();
		for (; it != c.end(); ++it) {
			if (*it == elem) {
				ret = true;
				break;
			}
		}

    	pthread_mutex_unlock(&mutex_read);
		pthread_mutex_unlock(&mutex_write);

		return ret;
	}

protected:
    std::deque<T> c;

private:
	pthread_mutex_t mutex_read, mutex_write;
	pthread_mutex_t read_lock;

 //   HANDLE	read_lock;
	int canreadnums;
    int canwritenums;
};

#endif //SAVEQUEUE_H_
