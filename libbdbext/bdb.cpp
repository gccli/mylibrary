#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <db.h>


#include <uuid/uuid.h>

//void uuid_generate(uuid_t out);
//void uuid_generate_random(uuid_t out);
//void uuid_generate_time(uuid_t out);


struct inode 
{
  unsigned int i_ino;
  unsigned int i_count;
  uid_t        i_uid;
  gid_t        i_gid;
  mode_t       i_mode;
  off_t        i_size;
  char         name[16];
  void print () {
    printf ("ino:   %d\n", i_ino);
    printf ("count: %d\n", i_count);
    printf ("uid:   %d\n", i_uid);
    printf ("gid:   %d\n", i_gid);
    printf ("mode:  %d\n", i_mode);
    printf ("size:  %d\n", i_size);
    printf ("name:  %s\n", name);
  }
};

struct ns__Req
{ 
  int                  __size;// size of data
  unsigned char*       __ptr; // points to data
  unsigned char        type;
  unsigned short       op;

  void print () {
    printf ("__ptr:   %x\n", __ptr);
    printf ("__size:  %d\n", __size);
    printf ("type:    %d\n", type);
    printf ("op:      %d\n", op);
  }
};

int main1(int argc, char* argv[])
{
  DB *dbp;
  DB_ENV *dbenv = NULL;
  DB_TXN *txnid = NULL;
  int ret;
  const char* dbfile = "s.db";
  const char* dbname = NULL;
  unsigned int flags, env_flags;

  // environment
  if ((ret = db_env_create (&dbenv, 0)) != 0) {
    printf ("db_env_create: %s\n", db_strerror(ret));
    return 1;
  }
  env_flags = DB_CREATE | DB_INIT_MPOOL; // in-memory cache
  if ((ret = dbenv->open (dbenv, "dbenv", env_flags, 0)) != 0) {
    printf ("dbenv->open: %s\n", db_strerror(ret));
    return 1;
  }
  
  if ((ret = db_create (&dbp, dbenv, 0)) != 0){
    printf ("db_create: %s\n", db_strerror(ret));
    return 1;
  }

  flags = DB_CREATE;
  if ((ret = dbp->open (dbp, 
			txnid,
			dbfile,
			dbname,
			DB_BTREE,
			flags,
			0600)) != 0) {
    printf ("dbp->open: %s\n", db_strerror(ret));
    return 1;
  }
  // Database Records
  DBT key, data;
  memset (&key,  0, sizeof(DBT));
  memset (&data, 0, sizeof(DBT));

  //  uuid_t uuid;
  //  uuid_generate_time (uuid);
  struct inode i;
  memset (&i, 0, sizeof (inode));
  
  i.i_ino = 1;
  i.i_count = 1;
  i.i_uid = getuid();
  i.i_gid = getgid();
  i.i_mode = 0600;
  i.i_size = 1024;
  strcpy (i.name, "lijing");

  key.data = &i.i_ino;
  key.size = sizeof(unsigned int);

  //  int ikey = 5555;
  //  key.data = &ikey;
  //  key.size = sizeof(int);

  //  char* desc = "hello, world.";
  data.data = &i;
  data.size = sizeof(inode);

  // put data
  //  if ((ret = dbp->exists(dbp, txnid, &key, 0)) == DB_NOTFOUND) {
  //  printf ("key not exists.\n");  
  dbp->del (dbp, txnid, &key, 0);
  if ((ret = dbp->put(dbp, txnid, &key, &data, DB_NOOVERWRITE)) != 0) {
    dbp->err (dbp, ret, "dbp->put");
    if (ret != DB_KEYEXIST) {
      return 1;
    }
  }
  //  }

  // administration
  /*  
  unsigned int openflag;
  dbp->get_open_flags (dbp, &openflag);
  printf ("flag %d\n", openflag);

  unsigned int gbytes, bytes;
  int ncache;
  dbp->get_cachesize(dbp, &gbytes, &bytes, &ncache);
  printf ("gigabytes:%d, bytes:%d, number of cache:%d\n",
	  gbytes, bytes, ncache);

  if ((ret = dbp->stat_print(dbp, DB_STAT_ALL)) != 0) {
    printf ("dbp->stat_print: %s\n", db_strerror(ret));
    return 1;
  }
  */

  if ((ret = dbp->sync(dbp, 0)) != 0) {
    printf ("dbp->sync: %s\n", db_strerror(ret));
    return 1;
  } 

  // get data
  memset (&i, 0, sizeof (inode));
  memset (&key,  0, sizeof (DBT));
  memset (&data, 0, sizeof (DBT));
  i.i_ino = 1;
  key.data = &i.i_ino;
  key.size = sizeof(unsigned int);

  data.data = &i;
  data.ulen = sizeof (i);
  data.flags = DB_DBT_USERMEM;

  if ((ret = dbp->get (dbp, txnid, &key, &data, 0)) != 0) {
    printf ("dbp->get: %s\n", db_strerror(ret));
    return 1;
  }
  else  {
    printf ("\ndata: 0x%x\n", data.data);
    i.print();
  }

  if (dbp) 
    dbp->close(dbp, 0);

  return 0;
}



int main(int argc, char* argv[])
{
  DB *dbp;
  DB_ENV *dbenv = NULL;
  DB_TXN *txnid = NULL;
  int ret;
  const char* dbfile = "s.db";
  const char* dbname = NULL;
  unsigned int flags, env_flags;

  // environment
  if ((ret = db_env_create (&dbenv, 0)) != 0) {
    printf ("db_env_create: %s\n", db_strerror(ret));
    return 1;
  }
  env_flags = DB_CREATE | DB_INIT_MPOOL; // in-memory cache
  if ((ret = dbenv->open (dbenv, "dbenv", env_flags, 0)) != 0) {
    printf ("dbenv->open: %s\n", db_strerror(ret));
    return 1;
  }
  
  if ((ret = db_create (&dbp, dbenv, 0)) != 0){
    printf ("db_create: %s\n", db_strerror(ret));
    return 1;
  }

  flags = DB_CREATE;
  if ((ret = dbp->open (dbp, 
			txnid,
			dbfile,
			dbname,
			DB_BTREE,
			flags,
			0600)) != 0) {
    printf ("dbp->open: %s\n", db_strerror(ret));
    return 1;
  }
  // Database Records
  DBT key, data;
  memset (&key,  0, sizeof(DBT));
  memset (&data, 0, sizeof(DBT));

  struct ns__Req r;
  memset (&r, 0, sizeof (ns__Req));

  char strbuf[] = "I'm pointer";
  int strl = strlen (strbuf);

  r.__ptr = (unsigned char*) malloc (strl+1);
  strcpy ((char*)r.__ptr, strbuf);
  r.__size = strl+1;
  r.type = 1;
  r.op = 2;
  
  unsigned int id = (r.type<<16) | r.op;
  key.data = &id;
  key.size = sizeof(unsigned int);

  // marshall data
  int buffersize = 4+strl+1+2;
  char* buffer = (char*)malloc (buffersize);
  memset (buffer, 0, buffersize);

  int buf_off = 0;
  memcpy (buffer + buf_off, &r.__size, 4);
  buf_off += 4;

  memcpy (buffer + buf_off, r.__ptr, r.__size);
  buf_off += r.__size;
  
  *(buffer + buf_off) = r.type;
  buf_off += 1;

  memcpy (buffer + buf_off, &r.op, 2);
  buf_off += 2;

  data.data = buffer;
  data.size = buf_off;

  if ((ret = dbp->put(dbp, txnid, &key, &data, DB_NOOVERWRITE)) != 0) {
    dbp->err (dbp, ret, "dbp->put");
    if (ret != DB_KEYEXIST) {
      return 1;
    }
  }

  if ((ret = dbp->sync(dbp, 0)) != 0) {
    printf ("dbp->sync: %s\n", db_strerror(ret));
    return 1;
  } 

  // get data
  //  memset (&key,  0, sizeof (DBT));
  memset (&data, 0, sizeof (DBT));

  if ((ret = dbp->get (dbp, txnid, &key, &data, 0)) != 0) {
    printf ("dbp->get: %s\n", db_strerror(ret));
    return 1;
  }
  else  {
    printf ("\ndata: 0x%x\n", data.data);
    buffer = (char* )data.data;
    int i = *(int *) buffer;
    printf ("%d\n", i);

    char* s = (char*) (buffer+4);
    printf ("%s\n", s);
  }

  if (dbp) 
    dbp->close(dbp, 0);

  return 0;
}
