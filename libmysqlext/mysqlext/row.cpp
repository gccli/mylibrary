#include "row.h"
using namespace MySQL;

Row::Row(MYSQL_RES *& _res)
	:m_result(_res)
{
	EMPTY = NULL;
}

Row::~Row(void)
{
	deinit();
}

Row* Row::do_fetch(MYSQL_ROW& r)
{
	// retrieve each field's actual length in the current row
	unsigned long* len = mysql_fetch_lengths (m_result);
	for (unsigned i = 0; i < width(); i++)
	{
		unsigned long datalen = len[i];
		if (r[i]==NULL && datalen==0){
			_columns[i]->is_null = true;
		}
		else{
			// copy the actual length of data
			memcpy (_columns[i]->value, r[i], datalen);
			_columns[i]->length = datalen;
		}
	}

	return this;
}

bool Row::init(void)
{
	static const char *UNKNOWN = "unknown field";
	if (!EMPTY) {
		EMPTY = new Field(UNKNOWN, (unsigned int) strlen(UNKNOWN), 1);
		EMPTY->length = 0;
		EMPTY->type   = MYSQL_TYPE_NULL;
	}

	int count = mysql_num_fields (m_result);
	Field *f;
	MYSQL_FIELD* pf;
	for (int i = 0; i < count; i++)
	{
		pf = mysql_fetch_field (m_result);
		if (pf == NULL)
		{
			_columns.push_back (EMPTY);
			continue;
		}

		// allocate enough memory space
		f = new Field(pf->name, pf->name_length, pf->max_length+1);
		f->type = pf->type;
		_columns.push_back(f);
	}

	return true;
}

bool Row::deinit(void) 
{
	fieldlist_t::iterator i;
	for (i = _columns.begin(); i != _columns.end(); ++i)
		if (*i != EMPTY)
			delete *i;
	_columns.clear();

	delete EMPTY;

	return true;
}

Field *Row::field(const char *name)
{
	fieldlist_t::iterator i;
	for (i = _columns.begin(); i != _columns.end(); ++i)
		if (!strcasecmp((*i)->name.c_str(), name))
			return *i;

	return NULL;
}

unsigned Row::width(void)
{
	return (unsigned )_columns.size();
}

int Row::position(const char *name)
{
	unsigned n = 0;
	fieldlist_t::iterator i;
	for (i = _columns.begin(); i != _columns.end(); ++i, n++)
		if (!strcasecmp((*i)->name.c_str(), name))
			return n;

	return -1;
}

const char *Row::name(unsigned position)
{
	if (position+1 > _columns.size())
		return NULL;

	return _columns[position]->name.c_str();
}

bool Row::isnull(unsigned position)
{
	if (position+1 > _columns.size())
		return true;

	return _columns[position]->is_null;
}

Field& Row::operator[](unsigned position)
{
	if (position+1 <= _columns.size())
		return *(_columns[position]);
	else
		return (Field&)*EMPTY;
}

Field& Row::operator[](signed position)
{
	return (*this)[(unsigned)position];
}

Field& Row::operator[](const char *name)
{
	Field *f = field(name);
	return f ? *f : (Field&)*EMPTY;
}

Field& Row::operator[](std::string &name)
{
	Field *f = field(name.c_str());
	return f ? *f : (Field&)*EMPTY;
}

bool Row::reset(void)
{
	fieldlist_t::iterator i;
	for (i = _columns.begin(); i != _columns.end(); ++i){
		memset((*i)->value, 0, (*i)->width);
	}

	return true;
}
