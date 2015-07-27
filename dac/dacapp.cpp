#include "dacapp.h"

uint32_t DACObjCreator::AllocId(DACObject * pObj)
{
	if (pObj != NULL)
		return INVALID_OBJECT_ID;

	unsigned int i = 1;

	DACAppTable::iterator it = m_objs.find(i);
	for (; it != m_objs.end(); ++i, it = m_objs.find(i))
		;

	return i;
}

DACObject *DACObjCreator::Create(int type)
{
	DACObject *pObj = NULL;
	switch(type)
	{
	case DAC_OBJ_APP:
		pObj = new DACAppObject;
		break;
	case DAC_OBJ_CENTRE:
		pObj = new DACCentreObject;
		break;
	case DAC_OBJ_EDGE:
		pObj = new DACEdgeObject;
		break;
	default:
		break;
	}
	if (pObj) 
	{
		unsigned int id = AllocId(pObj);
		pObj->m_id = id;
		m_objs[id] = pObj;
	}

	return pObj;
}

void DACObjCreator::Delete(unsigned int id)
{
	DACAppTable::iterator it = m_objs.find(id);
	if (it == m_objs.end())
		return ;

	m_objs.erase(it);
}

DACObject *DACObjCreator::Get(unsigned int id)
{
	DACAppTable::iterator it = m_objs.find(id);
	if (it == m_objs.end())
		return NULL;

	return it->second;
}


