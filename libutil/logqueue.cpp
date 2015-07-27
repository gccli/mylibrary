#include <assert.h>
#include "logqueue.h"

logqueue::logqueue(int capacity)
:m_head(0)
,m_tail(0)
,v(NULL)
{
  init (capacity);;
}


logqueue::~logqueue()
{
}


void logqueue::init(int capacity)
{
  assert (capacity > 0);

  m_capacity = capacity/10;
  m_capacity = 10*m_capacity;
  assert (m_capacity > 0);

  m_uplimit  = m_capacity - m_capacity/10;
  v = new LogItem[capacity];
}


int logqueue::getusage()
{
  int n;
  if (m_head < m_tail)
    n = m_tail - m_head;
  else if (m_head == m_tail)
    n = 0;
  else
    n = m_capacity+m_tail-m_head;

  return n;
}


int logqueue::getpercent()
{
  int percent = getusage();
  percent = 100*percent/m_capacity;

  return percent;
}


int logqueue::push(int category,int level,const char * buffer)
{
  int ret = QUEUE_OK;
  if (!full()) {
    LogItem* item = v + m_tail;
    item->category = category;
    item->level    = level;
    int length = strlen (buffer);

    if (length < MAX_LOGBUFFER_LENGTH) {
      strcpy (item->buffer, buffer);
    }
    else {
      printf ("warning: message is too long(%d), truncate it.\n", length);
      strncpy (item->buffer, buffer, MAX_LOGBUFFER_LENGTH-6);
      strcat  (item->buffer, " ...\n");
      item->buffer[MAX_LOGBUFFER_LENGTH-1] = 0;
    }
    if (getusage() > m_uplimit) {
      ret = QUEUE_UPLIMIT;
    }

    m_tail = (m_tail+1)%m_capacity;
  }
  else {
    return QUEUE_FULL;
  }

  return ret;
}


int logqueue::pop(LogItem * item)
{
  if (empty())
    return QUEUE_EMPTY;

  *item = v[m_head];
  m_head = (m_head+1)%m_capacity;

  return 0;
}
