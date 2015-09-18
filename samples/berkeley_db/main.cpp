#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/syscall.h>

#include "bdbquery.h"
#include <list>

using namespace std;

#define gettid() syscall(__NR_gettid)

class Account
{
public:
	Account()
	{
		memset (vendor, 0, sizeof(vendor));
		memset (name, 0, sizeof(name));
		memset (passwd, 0, sizeof(passwd));
	}

	Account(const char* v, const char* n, const char* p)
	{
		strncpy(vendor, v, sizeof(vendor));
		strncpy(name, n, sizeof(name));
		strncpy(passwd, p, sizeof(passwd));
	}

	const char* Vendor() const { return vendor; }
	void show ()
	{
		printf ("%-16s%s/%s\n", vendor, name, passwd);
	}
	
private:
	char            vendor[16];
	char            name[32];
	char            passwd[16];
};

class AccountMgr
{
public:
	AccountMgr(const char* dbname);
	~AccountMgr() { m_db.close(); }

	int createdb(FILE* fp = stdin);
	
	int init();
	
	int  acct_add(const char* vendor, const char* acname, const char* acpasswd);
	int  acct_delete();
	int  acct_modify();
	bool acct_query(const char* vendor);
	
private:
	list<Account* > m_acctlist;
	char            m_passwd[16];
	bdbext::Query   m_db;
};

AccountMgr::AccountMgr(const char* dbname)
	:m_db(dbname)
{
	memset (m_passwd, 0, sizeof(m_passwd));
}

int AccountMgr::init()
{
	if (m_db.init(false) != 0)
		return 1;
	if (m_db.open(NULL, 0) != 0)
		return 1;

	return 0;
}

int AccountMgr::createdb(FILE* fp)
{
	if (m_db.init(false) != 0)
		return 1;
	if (m_db.open(NULL, DB_CREATE) != 0)
		return 1;

	size_t len = 0;
	ssize_t read;
	char* line = NULL;

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("Retrieved line of length %zu :\n", read);
		printf("%s", line);
	}

	if (line)
		free(line);

	return 0;
}


int AccountMgr::acct_add(const char* vendor, const char* acname, const char* acpasswd)
{
	DBT _key, _value;
	memset (&_key,  0, sizeof(DBT));
	memset (&_value, 0, sizeof(DBT));

	char tbuff[128];
	memset (tbuff, 0, sizeof(tbuff));
	strcpy (tbuff, vendor);

	_key.data = tbuff;
	_key.size = strlen(tbuff);

	Account* acct = new Account(vendor, acname, acpasswd);

	_value.data = acct;
	_value.size = sizeof(Account);

	if (m_db.write(&_key, &_value, DB_NOOVERWRITE))
		return 1;
	m_db.sync();

	m_acctlist.push_back(acct);

	return 0;
}

bool AccountMgr::acct_query(const char* vendor)
{
	bool found = false;
	list<Account* >::iterator it = m_acctlist.begin();
	for (; it != m_acctlist.end(); ++it) {
		if (strcmp ((*it)->Vendor(), vendor) == 0) {
			found = true;
			break;
		}
	}
	if (found) {
		(*it)->show();
	}

	return found;
}

int main(int argc, char* argv[])
{
	AccountMgr acctMgr("account.db");

	//	acctMgr.createdb(stdin);

	acctMgr.init();
	acctMgr.acct_add("vendor", "acname", "acpasswd");
	acctMgr.acct_query("vendor");

	return 0;
}
