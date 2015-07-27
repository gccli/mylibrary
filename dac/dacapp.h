#ifndef __DAC_APP_H__
#define __DAC_APP_H__

#include "dac.h"
#include "dacobject.h"
#include <map>

class DACAppObject : public DACObject
{
public:
	DACAppObject()
		:DACObject(DAC_OBJ_APP)
	{}
};


class DACCentreObject : public DACObject
{
public:
	DACCentreObject()
		:DACObject(DAC_OBJ_CENTRE)
	{}
};

class DACEdgeObject : public DACObject
{
public:
	DACEdgeObject()
		:DACObject(DAC_OBJ_EDGE)
	{}
};












typedef std::map<unsigned int, DACObject *> DACAppTable;
class DACObjCreator
{
public:
	void Delete(unsigned int id);
	DACObject *Create(int type);
	DACObject *Get(unsigned int id);

private:
	uint32_t AllocId(DACObject * pObj);

private:
	DACAppTable m_objs;
};


#endif

