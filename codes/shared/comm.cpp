#include "comm.h"


shared_ptr<CommClass> CommClass::m_cc;

CommClass *CommClass::geti()
{
    if (m_cc.get() == NULL)
    {
	m_cc = shared_ptr<CommClass>(new CommClass); 
	printf("create a new instance %p\n", m_cc.get());
    }

    return m_cc.get();
}
