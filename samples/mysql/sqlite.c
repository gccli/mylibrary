#include <stdio.h>
#include <sqlite3.h>

static sqlite3 *db = NULL;


int db_init(const char *dbname)
{
    char *errmsg;
    const char *sql;
    int rc;
    rc = sqlite3_open(dbname, &db);
    if (rc) {
	printf("Can't open database: %s\n", sqlite3_errmsg(db));
	sqlite3_close(db);
    }

  sql="PRAGMA foreign_keys = ON;"
    "CREATE TABLE IF NOT EXISTS env("
    "name TEXT PRIMARY KEY,"
    "version INTEGER);"
    "CREATE TABLE IF NOT EXISTS config("
    "cfg_key TEXT PRIMARY KEY,"
    "cfg_value TEXT DEFAULT '',"
    "cfg_value_type INTEGER DEFAULT 0,"
    "env_name TEXT NOT NULL,"
    "version INTEGER,"
    "FOREIGN KEY (env_name)"
    "  REFERENCES env (name) ON UPDATE CASCADE ON DELETE CASCADE"
      ");";

  rc = sqlite3_exec(db, sql, NULL, 0, &errmsg);
  if (rc) {
      printf("SQL '%s' error: %s\n", sql, errmsg);
      sqlite3_free(errmsg);
      sqlite3_close(db);
      return rc;
  }

    return rc;
}

int db_exec(const char *sql)
{
    int rc;
    char *errmsg = NULL;

    rc = sqlite3_exec(db, sql, NULL, 0, &errmsg);
    if (rc) {
	printf("SQL '%s' error: %s\n", sql, errmsg);
	sqlite3_free(errmsg);
	return rc;
    }

    return 0;
}

static int callback(void *pArg, int nArg, char **azArg, char **azCol)
{
    int head = *(int *)pArg;
    int i, w=16;
    for(i=0; head == 0 && i<nArg; ++i) {
	printf("%*s ", w, azCol[i]);
    }

    if (head == 0) printf("\n----------------------------------\n");
    if (head == 0) *(int *)pArg = 1;

    for(i=0; i<nArg; ++i) {
	printf("%*s ", w, azArg[i]);
    }
    printf("\n");

    return 0;
}

int db_env_get_version()
{
    int rc, head;
    char sql[1024] = {0};
    char *errmsg = NULL;
    sprintf(sql, "SELECT * FROM env;");

    head = 0;
    rc = sqlite3_exec(db, sql, callback, &head, &errmsg);
    if (rc) {
	printf("SQL '%s' error: %s", sql, errmsg);
	sqlite3_free(errmsg);
    }

    return rc;
}

int insert_data(sqlite3* db)
{
  sqlite3_stmt *pInsert = 0;
  const char *env = "env-xx-1";
  char sql[4096];
  char key[64];
  int rc;
  int i, j;

  sprintf(sql, "INSERT OR REPLACE INTO env VALUES('%s', 1)", env);
  if (db_exec(sql))
      return -1;

  snprintf(sql, sizeof(sql), "INSERT OR REPLACE INTO config VALUES(?,?,?,?,?);");

  rc = sqlite3_prepare_v2(db, sql, -1, &pInsert, 0);
  if(rc){
      fprintf(stderr, "Error %d: %s on [%s]\n",
	      sqlite3_extended_errcode(db), sqlite3_errmsg(db),
	      sql);
      goto end_trans;
  }

  for(i=0; i<10; ++i) {
      sprintf(key, "key_%d", i);
      j = 1;
      sqlite3_bind_text(pInsert, j, key, strlen(key), SQLITE_STATIC);
      j++;
      sqlite3_bind_text(pInsert, j, key, strlen(key), SQLITE_STATIC);
      j++;
      sqlite3_bind_int64(pInsert, j, 1);
      j++;
      sqlite3_bind_text(pInsert, j, env, strlen(env), SQLITE_STATIC);
      j++;
      sqlite3_bind_int64(pInsert, j, i+10);
      j++;

      rc = sqlite3_step(pInsert);
      if((rc != SQLITE_OK) && (rc != SQLITE_DONE) && (rc != SQLITE_ROW)){
	  fprintf(stderr, "Step error %d: %s\n", sqlite3_extended_errcode(db),
		  sqlite3_errmsg(db));
	  goto end_trans;
      }
      sqlite3_reset(pInsert);
  }

end_trans:
  sqlite3_finalize(pInsert);

  return rc;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
	printf("Usage: %s <db>\n", argv[0]);
	return 1;
    }
    if (db_init(argv[1]))
	return 1;
    if (insert_data(db))
	return 1;

    db_env_get_version();

    return 0;
}
