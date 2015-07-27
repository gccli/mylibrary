#ifndef LOGQUEUE_H_
#define LOGQUEUE_H_

#include <stdio.h>
#include <string.h>

#define MAX_LOGBUFFER_LENGTH            512
typedef struct _LogItem
{
    unsigned char category;                       // what type of program is logging the message
    unsigned char level;                          // determines the importance of the message
    char          buffer[MAX_LOGBUFFER_LENGTH];

    _LogItem()
    :category(0), level(0) {
        memset (buffer, 0, MAX_LOGBUFFER_LENGTH);
    }
    _LogItem(const _LogItem& item) {
        *this = item;
    }
    _LogItem& operator = (const _LogItem& item) {
        if (this == &item)
            return *this;

        this->category = item.category;
        this->level = item.level;
        strcpy (this->buffer, item.buffer);

        return *this;
    }
} LogItem;

enum QUEUE_STATE
{
    QUEUE_OK     = 0,
    QUEUE_EMPTY,
    QUEUE_UPLIMIT,
    QUEUE_FULL
};

class logqueue
{
    public:
        logqueue(int capacity);
        ~logqueue();

        void init(int capacity);
        bool full() const { return ((m_tail+1)%m_capacity == m_head); }
        bool empty() const { return (m_head == m_tail); }

        int push(int category, int level, const char* buffer);
        int pop(LogItem* item);
        int getcapacity() const { return m_capacity; }
        int getusage();
        int getpercent();

    private:
        int      m_capacity;
        int      m_uplimit;                       // 90% of m_capacity
        int      m_head, m_tail;
        LogItem* v;
};
#endif
