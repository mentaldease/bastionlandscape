#include "LuaPlus/LuaPlus.h"

using namespace LuaPlus;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <windows.h>
#include <sqlext.h>

#define LUASQL_API
#define LUASQL_PREFIX "LuaSQL: "
#define LUASQL_TABLENAME "luasql"
#define LUASQL_ENVIRONMENT "Each driver must have an environment metatable"
#define LUASQL_CONNECTION "Each driver must have a connection metatable"
#define LUASQL_CURSOR "Each driver must have a cursor metatable"

LUASQL_API int luasql_faildirect(lua_State *L, const char *err);
LUASQL_API int luasql_createmeta(lua_State *L, const char *name, const luaL_reg *methods);
LUASQL_API void luasql_setmeta(lua_State *L, const char *name);
LUASQL_API void luasql_getlibtable(lua_State *L);

#define LUASQL_ENVIRONMENT_ODBC "ODBC environment"
#define LUASQL_CONNECTION_ODBC "ODBC connection"
#define LUASQL_CURSOR_ODBC "ODBC cursor"


/*
** Typical database error situation
*/
LUASQL_API int luasql_faildirect(lua_State *L, const char *err) {
    lua_pushnil(L);
    lua_pushstring(L, err);
    return 2;
}

/*
** Create a metatable
*/
LUASQL_API int luasql_createmeta (lua_State *L, const char *name, const luaL_reg *methods) {
	if (!luaL_newmetatable (L, name))
		return 0;

	/* define methods */
	luaL_openlib (L, NULL, methods, 0);

	/* define metamethods */
	lua_pushliteral (L, "__gc");
	lua_pushcfunction (L, methods->func);
	lua_settable (L, -3);

	lua_pushliteral (L, "__index");
	lua_pushvalue (L, -2);
	lua_settable (L, -3);

	lua_pushliteral (L, "__metatable");
	lua_pushliteral (L, LUASQL_PREFIX"you're not allowed to get this metatable");
	lua_settable (L, -3);

	return 1;
}


/*
** Define the metatable for the object on top of the stack
*/
LUASQL_API void luasql_setmeta (lua_State *L, const char *name) {
	luaL_getmetatable (L, name);
	lua_setmetatable (L, -2);
}


/*
** Push the library table onto the stack.
** If it does not exist, create one.
*/
LUASQL_API void luasql_getlibtable (lua_State *L) {
	lua_getglobal(L, LUASQL_TABLENAME);
	if (lua_isnil (L, -1)) {
		lua_newtable (L);
		lua_pushvalue (L, -1);
		lua_setglobal (L, LUASQL_TABLENAME);
	}
}


typedef struct {
	short      closed;
	SQLHENV    henv;               /* environment handle */
} env_data;


typedef struct {
	short      closed;
	int        env;                /* reference to environment */
	SQLHDBC    hdbc;               /* database connection handle */
} conn_data;


typedef struct {
	short      closed;
	int        conn;               /* reference to connection */
	int        numcols;            /* number of columns */
	int        coltypes, colnames; /* reference to column information tables */
	SQLHSTMT   hstmt;              /* statement handle */
} cur_data;


/* we are lazy */
#define hENV SQL_HANDLE_ENV
#define hSTMT SQL_HANDLE_STMT
#define hDBC SQL_HANDLE_DBC
#define error(a) ((a) != SQL_SUCCESS && (a) != SQL_SUCCESS_WITH_INFO)


/*
** Check for valid environment.
*/
static env_data *getenvironment (lua_State *L) {
	env_data *env = (env_data *)luaL_checkudata (L, 1, LUASQL_ENVIRONMENT_ODBC);
	luaL_argcheck (L, env != NULL, 1, "environment expected");
	luaL_argcheck (L, !env->closed, 1, "environment is closed");
	return env;
}


/*
** Check for valid connection.
*/
static conn_data *getconnection (lua_State *L) {
	conn_data *conn = (conn_data *)luaL_checkudata (L, 1, LUASQL_CONNECTION_ODBC);
	luaL_argcheck (L, conn != NULL, 1, "connection expected");
	luaL_argcheck (L, !conn->closed, 1, "connection is closed");
	return conn;
}


/*
** Check for valid cursor.
*/
static cur_data *getcursor (lua_State *L) {
	cur_data *cursor = (cur_data *)luaL_checkudata (L, 1, LUASQL_CURSOR_ODBC);
	luaL_argcheck (L, cursor != NULL, 1, "cursor expected");
	luaL_argcheck (L, !cursor->closed, 1, "cursor is closed");
	return cursor;
}


/*
** Pushes 1 and returns 1
*/
static int pass(lua_State *L) {
    lua_pushnumber(L, 1);
    return 1;
}


/*
** Fails with error message from ODBC
** Inputs:
**   type: type of handle used in operation
**   handle: handle used in operation
*/
static int fail(lua_State *L,  const SQLSMALLINT type, const SQLHANDLE handle) {
    SQLCHAR State[6];
    SQLINTEGER NativeError;
    SQLSMALLINT MsgSize, i;
    SQLRETURN ret;
    SQLCHAR Msg[SQL_MAX_MESSAGE_LENGTH];
    luaL_Buffer b;
    lua_pushnil(L);

    luaL_buffinit(L, &b);
    i = 1;
    while (1) {
        ret = SQLGetDiagRec(type, handle, i, State, &NativeError, Msg,
                sizeof(Msg), &MsgSize);
        if (ret == SQL_NO_DATA) break;
        luaL_addlstring(&b, (const char*)Msg, MsgSize);
        luaL_putchar(&b, '\n');
        i++;
    }
    luaL_pushresult(&b);
    return 2;
}

/*
** Returns the name of an equivalent lua type for a SQL type.
*/
static const char *sqltypetolua (const SQLSMALLINT type) {
    switch (type) {
        case SQL_UNKNOWN_TYPE: case SQL_CHAR: case SQL_VARCHAR:
        case SQL_TYPE_DATE: case SQL_TYPE_TIME: case SQL_TYPE_TIMESTAMP:
        case SQL_DATE: case SQL_INTERVAL: case SQL_TIMESTAMP:
        case SQL_LONGVARCHAR:
            return "string";
        case SQL_BIGINT: case SQL_TINYINT: case SQL_NUMERIC:
        case SQL_DECIMAL: case SQL_INTEGER: case SQL_SMALLINT:
        case SQL_FLOAT: case SQL_REAL: case SQL_DOUBLE:
            return "number";
        case SQL_BINARY: case SQL_VARBINARY: case SQL_LONGVARBINARY:
            return "binary";	/* !!!!!! nao seria string? */
        case SQL_BIT:
            return "boolean";
        default:
            assert(0);
            return NULL;
    }
}


/*
** Retrieves data from the i_th column in the current row
** Input
**   types: index in stack of column types table
**   hstmt: statement handle
**   i: column number
** Returns:
**   0 if successfull, non-zero otherwise;
*/
static int push_column(lua_State *L, int coltypes, const SQLHSTMT hstmt,
        SQLUSMALLINT i) {
    const char *tname;
    char type;
    /* get column type from types table */
	lua_rawgeti (L, LUA_REGISTRYINDEX, coltypes);
	lua_rawgeti (L, -1, i);	/* typename of the column */
    tname = lua_tostring(L, -1);
    if (!tname)
		return luasql_faildirect(L, LUASQL_PREFIX"Invalid type in table.");
    type = tname[1];
    lua_pop(L, 2);	/* pops type name and coltypes table */

    /* deal with data according to type */
    switch (type) {
        /* nUmber */
        case 'u': {
              double num;
              SQLINTEGER got;
              SQLRETURN rc = SQLGetData(hstmt, i, SQL_C_DOUBLE, &num, 0, &got);
              if (error(rc)) return fail(L, hSTMT, hstmt);
              if (got == SQL_NULL_DATA) lua_pushnil(L);
              else lua_pushnumber(L, (lua_Number)num);
              return 0;
          }
                  /* bOol */
        case 'o': {
              char b;
              SQLINTEGER got;
              SQLRETURN rc = SQLGetData(hstmt, i, SQL_C_BIT, &b, 0, &got);
              if (error(rc)) return fail(L, hSTMT, hstmt);
              if (got == SQL_NULL_DATA) lua_pushnil(L);
              else lua_pushstring(L, b ? "true" : "false");
              return 0;
          }
        /* sTring */
        case 't':
        /* bInary */
        case 'i': {
              SQLSMALLINT stype = (type == 't') ? SQL_C_CHAR : SQL_C_BINARY;
              SQLINTEGER got;
              char *buffer;
              luaL_Buffer b;
              SQLRETURN rc;
              luaL_buffinit(L, &b);
              buffer = luaL_prepbuffer(&b);
              rc = SQLGetData(hstmt, i, stype, buffer, LUAL_BUFFERSIZE, &got);
              if (got == SQL_NULL_DATA) {
                  lua_pushnil(L);
                  return 0;
              }
              /* concat intermediary chunks */
              while (rc == SQL_SUCCESS_WITH_INFO) {
                  if (got >= LUAL_BUFFERSIZE || got == SQL_NO_TOTAL) {
                      got = LUAL_BUFFERSIZE;
                      /* get rid of null termination in string block */
                      if (stype == SQL_C_CHAR) got--;
                  }
                  luaL_addsize(&b, got);
                  buffer = luaL_prepbuffer(&b);
                  rc = SQLGetData(hstmt, i, stype, buffer,
                          LUAL_BUFFERSIZE, &got);
              }
              /* concat last chunk */
              if (rc == SQL_SUCCESS) {
                  if (got >= LUAL_BUFFERSIZE || got == SQL_NO_TOTAL) {
                      got = LUAL_BUFFERSIZE;
                      /* get rid of null termination in string block */
                      if (stype == SQL_C_CHAR) got--;
                  }
                  luaL_addsize(&b, got);
              }
              if (rc == SQL_ERROR) return fail(L, hSTMT, hstmt);
              /* return everything we got */
              luaL_pushresult(&b);
              return 0;
          }
    }
    return 0;
}

/*
** Get another row of the given cursor.
*/
static int cur_fetch (lua_State *L) {
    cur_data *cur = (cur_data *) getcursor (L);
    SQLHSTMT hstmt = cur->hstmt;
    int ret;
    SQLRETURN rc = SQLFetch(cur->hstmt);
    if (rc == SQL_NO_DATA) {
        lua_pushnil(L);
        return 1;
    } else if (error(rc)) fail(L, hSTMT, hstmt);

	if (lua_istable (L, 2)) {
		SQLUSMALLINT i;
		const char *opts = luaL_optstring (L, 3, "n");
		int num = strchr (opts, 'n') != NULL;
		int alpha = strchr (opts, 'a') != NULL;
		for (i = 1; i <= cur->numcols; i++) {
			ret = push_column (L, cur->coltypes, hstmt, i);
			if (ret)
				return ret;
			if (alpha) {
				lua_rawgeti (L, LUA_REGISTRYINDEX, cur->colnames);
				lua_rawgeti (L, -1, i);
				lua_pushvalue (L, -3);
				lua_rawset (L, 2);
				lua_pop (L, 1);	/* pops colnames table */
			}
			if (num)
				lua_rawseti (L, 2, i);
		}
		lua_pushvalue (L, 2);
		return 1;	/* return table */
	}
	else {
		SQLUSMALLINT i;
		for (i = 1; i <= cur->numcols; i++) {
			ret = push_column (L, cur->coltypes, hstmt, i);
			if (ret)
				return ret;
		}
		return cur->numcols;
	}
}

/*
** Closes a cursor.
*/
static int cur_close (lua_State *L) {
	cur_data *cur = (cur_data *) luaL_checkudata (L, 1, LUASQL_CURSOR_ODBC);
	SQLHSTMT hstmt = cur->hstmt;
	SQLRETURN ret;
	if (cur->closed)
		return 0;

	/* Nullify structure fields. */
	cur->closed = 1;
	ret = SQLCloseCursor(hstmt);
    if (error(ret))
		return fail(L, hSTMT, hstmt);
	ret = SQLFreeHandle(hSTMT, hstmt);
	if (error(ret))
		return fail(L, hSTMT, hstmt);
	luaL_unref (L, LUA_REGISTRYINDEX, cur->conn);
	luaL_unref (L, LUA_REGISTRYINDEX, cur->colnames);
	luaL_unref (L, LUA_REGISTRYINDEX, cur->coltypes);
    return pass(L);
}


/*
** Returns the table with column names.
*/
static int cur_colnames (lua_State *L) {
	cur_data *cur = (cur_data *) getcursor (L);
	lua_rawgeti (L, LUA_REGISTRYINDEX, cur->colnames);
	return 1;
}


/*
** Returns the table with column types.
*/
static int cur_coltypes (lua_State *L) {
	cur_data *cur = (cur_data *) getcursor (L);
	lua_rawgeti (L, LUA_REGISTRYINDEX, cur->coltypes);
	return 1;
}


/*
** Creates two tables with the names and the types of the columns.
*/
static void create_colinfo (lua_State *L, cur_data *cur) {
	SQLCHAR buffer[256];
	SQLSMALLINT namelen, datatype, i;
	SQLRETURN ret;
	int types, names;

	lua_newtable(L);
	types = lua_gettop (L);
	lua_newtable(L);
	names = lua_gettop (L);
	for (i = 1; i <= cur->numcols; i++) {
		ret = SQLDescribeCol(cur->hstmt, i, buffer, sizeof(buffer),
                &namelen, &datatype, NULL, NULL, NULL);
		/*if (ret == SQL_ERROR) return fail(L, hSTMT, cur->hstmt);*/
		lua_pushstring (L, (const char*)buffer);
		lua_rawseti (L, names, i);
		lua_pushstring(L, sqltypetolua(datatype));
		lua_rawseti (L, types, i);
	}
	cur->colnames = luaL_ref (L, LUA_REGISTRYINDEX);
	cur->coltypes = luaL_ref (L, LUA_REGISTRYINDEX);
}


/*
** Creates a cursor table and leave it on the top of the stack.
*/
static int create_cursor (lua_State *L, conn_data *conn,
        const SQLHSTMT hstmt, const SQLSMALLINT numcols) {
    cur_data *cur = (cur_data *) lua_newuserdata(L, sizeof(cur_data));
	luasql_setmeta (L, LUASQL_CURSOR_ODBC);

    /* fill in structure */
	cur->closed = 0;
	cur->conn = LUA_NOREF;
    cur->numcols = numcols;
	cur->colnames = LUA_NOREF;
	cur->coltypes = LUA_NOREF;
    cur->hstmt = hstmt;
	lua_pushvalue (L, 1);
    cur->conn = luaL_ref (L, LUA_REGISTRYINDEX);

	/* make and store column information table */
	create_colinfo (L, cur);

    return 1;
}


/*
** Closes a connection.
*/
static int conn_close (lua_State *L) {
	SQLRETURN ret;
    conn_data *conn = (conn_data *)luaL_checkudata(L,1,LUASQL_CONNECTION_ODBC);
	if (conn->closed)
		return 0;

	/* Nullify structure fields. */
	conn->closed = 1;
	luaL_unref (L, LUA_REGISTRYINDEX, conn->env);
	ret = SQLDisconnect(conn->hdbc);
	if (error(ret))
		return fail(L, hDBC, conn->hdbc);
	ret = SQLFreeHandle(hDBC, conn->hdbc);
	if (error(ret))
		return fail(L, hDBC, conn->hdbc);
    return pass(L);
}


/*
** Executes a SQL statement.
** Returns
**   cursor object: if there are results or
**   row count: number of rows affected by statement if no results
*/
static int conn_execute (lua_State *L) {
	conn_data *conn = (conn_data *) getconnection (L);
	const char *statement = luaL_checkstring(L, 2);
	SQLHDBC hdbc = conn->hdbc;
	SQLHSTMT hstmt;
	SQLSMALLINT numcols;
	SQLRETURN ret;
	ret = SQLAllocHandle(hSTMT, hdbc, &hstmt);
	if (error(ret))
		return fail(L, hDBC, hdbc);
	ret = SQLPrepare(hstmt, (SQLCHAR*) statement, SQL_NTS);
	if (error(ret))
		return fail(L, hSTMT, hstmt);
	/* execute the statement */
	ret = SQLExecute (hstmt);
	if (error(ret))
		return fail(L, hSTMT, hstmt);
	/* determine the number of results */
	ret = SQLNumResultCols (hstmt, &numcols);
	if (error(ret)) {
		ret = fail(L, hSTMT, hstmt);
		SQLFreeHandle(hSTMT, hstmt);
		return ret;
	}
	if (numcols > 0)
    	/* if there is a results table (e.g., SELECT) */
		return create_cursor (L, conn, hstmt, numcols);
	else {
		/* if action has no results (e.g., UPDATE) */
		SQLINTEGER numrows;
		ret = SQLRowCount(hstmt, &numrows);
		if (error(ret)) {
			ret = fail(L, hSTMT, hstmt);
			SQLFreeHandle(hSTMT, hstmt);
			return ret;
		}
		lua_pushnumber(L, numrows);
		return 1;
	}
}

/*------------------------------------------------------------------*\
* Returns a list with the names of the tables in the data source.
\*------------------------------------------------------------------*/
static int sqlConnTableList(lua_State *L) {
    conn_data *conndata = (conn_data *) lua_touserdata(L, 1);
    SQLHSTMT hstmt;
    SQLINTEGER got;
	int index, list;
    SQLUSMALLINT size;
    char *buffer;
    SQLRETURN ret = SQLAllocHandle(hSTMT, conndata->hdbc, &hstmt);
    if (error(ret))
		return fail(L, hDBC, conndata->hdbc);
    ret = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)"TABLE", SQL_NTS );
    if (error(ret))
		return fail(L, hSTMT, hstmt);
    ret = SQLGetInfo(conndata->hdbc, SQL_MAX_TABLE_NAME_LEN,
            (SQLPOINTER) &size, sizeof(size), NULL);
    if (error(ret))
		return fail(L, hSTMT, hstmt);
    size = size > 0 ? size : 256;
    buffer = (char *) malloc(size);
    if (!buffer) luasql_faildirect(L, LUASQL_PREFIX"allocation error.");
    /* create list */
    lua_newtable(L); list = lua_gettop(L);
    /* store fields */
    index = 1;
    while (1) {
        /* ask for next table name */
        ret = SQLFetch(hstmt);
        if (ret == SQL_NO_DATA) break;
        if (error(ret)) {
            SQLFreeHandle(hSTMT, hstmt);
            return fail(L, hSTMT, hstmt);
        }
        lua_pushnumber(L, index);
        ret = SQLGetData(hstmt, 3, SQL_C_CHAR, (SQLCHAR*)buffer, size, &got);
        lua_pushlstring(L, buffer, got);
        /* save result on table name list */
        lua_settable(L, list);
        index++;
    }
    free(buffer);
    SQLFreeHandle(hSTMT, hstmt);
    /* return the table, it is already on top of stack */
    return 1;
}

/*
** Rolls back a transaction.
*/
static int conn_commit (lua_State *L) {
	conn_data *conn = (conn_data *) getconnection (L);
	SQLRETURN ret = SQLEndTran(hDBC, conn->hdbc, SQL_COMMIT);
	if (error(ret))
		return fail(L, hSTMT, conn->hdbc);
	else
		return pass(L);
}

/*
** Rollback the current transaction.
*/
static int conn_rollback (lua_State *L) {
	conn_data *conn = (conn_data *) getconnection (L);
	SQLRETURN ret = SQLEndTran(hDBC, conn->hdbc, SQL_ROLLBACK);
	if (error(ret))
		return fail(L, hSTMT, conn->hdbc);
	else
		return pass(L);
}

/*
** Sets the auto commit mode
*/
static int conn_setautocommit (lua_State *L) {
	conn_data *conn = (conn_data *) lua_touserdata(L, 1);
	SQLRETURN ret;
	if (lua_toboolean (L, 2)) {
		ret = SQLSetConnectAttr(conn->hdbc, SQL_ATTR_AUTOCOMMIT,
			(SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);
	} else {
		ret = SQLSetConnectAttr(conn->hdbc, SQL_ATTR_AUTOCOMMIT,
			(SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0);
	}
	if (error(ret))
		return fail(L, hSTMT, conn->hdbc);
	else
		return pass(L);
}


/*
** Create a new Connection object and push it on top of the stack.
*/
static int create_connection (lua_State *L, env_data *env, SQLHDBC hdbc) {
	conn_data *conn = (conn_data *) lua_newuserdata(L, sizeof(conn_data));
	/* set auto commit mode */
	SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT,
		(SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);
	if (error(ret))
		return fail(L, hDBC, hdbc);

	luasql_setmeta (L, LUASQL_CONNECTION_ODBC);

	/* fill in structure */
	conn->closed = 0;
	conn->env = LUA_NOREF;
	conn->hdbc = hdbc;
	lua_pushvalue (L, 1);
	conn->env = luaL_ref (L, LUA_REGISTRYINDEX);
	return 1;
}


/*
** Creates and returns a connection object
** Lua Input: source [, user [, pass]]
**   source: data source
**   user, pass: data source authentication information
** Lua Returns:
**   connection object if successfull
**   nil and error message otherwise.
*/
static int env_connect (lua_State *L) {
	env_data *env = (env_data *) getenvironment (L);
	const char *sourcename = luaL_checkstring (L, 2);
	const char *username = luaL_optstring (L, 3, NULL);
	const char *password = luaL_optstring (L, 4, NULL);
	SQLHDBC hdbc;
	SQLRETURN ret = SQLSetEnvAttr (env->henv, SQL_ATTR_ODBC_VERSION,
		(void*)SQL_OV_ODBC3, 0);
	if (error(ret))
		return luasql_faildirect (L, LUASQL_PREFIX"error setting SQL version.");
	/* tries to allocate connection handle */
	ret = SQLAllocHandle (hDBC, env->henv, &hdbc);
	if (error(ret))
		return luasql_faildirect (L, LUASQL_PREFIX"connection allocation error.");
	/* tries to connect handle */
	ret = SQLConnect (hdbc, (SQLCHAR*) sourcename, SQL_NTS,
		(SQLCHAR*) username, SQL_NTS, (SQLCHAR*) password, SQL_NTS);
	if (error(ret)) {
		ret = fail(L, hDBC, hdbc);
		SQLFreeHandle(hDBC, hdbc);
		return ret;
	}
	/* success, return connection object */
	return create_connection (L, env, hdbc);
}

/*
** Closes an environment object
*/
static int env_close (lua_State *L) {
	SQLRETURN ret;
	env_data *env = (env_data *)luaL_checkudata(L, 1, LUASQL_ENVIRONMENT_ODBC);
	if (env->closed)
		return 0;

	env->closed = 1;
	ret = SQLFreeHandle (hENV, env->henv);
	if (error(ret)) {
		int ret = fail(L, hENV, env->henv);
		env->henv = NULL;
		return ret;
	}
	return pass(L);
}

/*
** Create metatables for each class of object.
*/
static void create_metatables (lua_State *L) {
	struct luaL_reg environment_methods[] = {
		{"close", env_close},
		{"connect", env_connect},
		{NULL, NULL},
	};
	struct luaL_reg connection_methods[] = {
		{"close", conn_close},
		{"execute", conn_execute},
		{"commit", conn_commit},
		{"rollback", conn_rollback},
		{"setautocommit", conn_setautocommit},
		{NULL, NULL},
	};
	struct luaL_reg cursor_methods[] = {
		{"close", cur_close},
		{"fetch", cur_fetch},
		{"getcoltypes", cur_coltypes},
		{"getcolnames", cur_colnames},
		{NULL, NULL},
	};
	luasql_createmeta (L, LUASQL_ENVIRONMENT_ODBC, environment_methods);
	luasql_createmeta (L, LUASQL_CONNECTION_ODBC, connection_methods);
	luasql_createmeta (L, LUASQL_CURSOR_ODBC, cursor_methods);
}


/*
** Creates an Environment and returns it.
*/
static int create_environment (lua_State *L) {
	env_data *env;
	SQLHENV henv;
	SQLRETURN ret = SQLAllocHandle(hENV, SQL_NULL_HANDLE, &henv);
	if (error(ret))
		return luasql_faildirect(L,LUASQL_PREFIX"error creating environment.");

	env = (env_data *)lua_newuserdata (L, sizeof (env_data));
	luasql_setmeta (L, LUASQL_ENVIRONMENT_ODBC);
	/* fill in structure */
	env->closed = 0;
	env->henv = henv;
	return 1;
}


/*
** Creates the metatables for the objects and registers the
** driver open method.
*/
LUASQL_API int luasql_libopen_odbc(lua_State *L) {
	luasql_getlibtable (L);
	lua_pushstring(L, "odbc");
	lua_pushcfunction(L, create_environment);
	lua_settable(L, -3);

	create_metatables (L);

	return 0;
}

extern "C" LUAMODULE_API int luaopen_sql(lua_State* L)
{
	LuaState* state = LuaState::CastState(L);
//	LuaObject obj = state->GetGlobals().CreateTable("sql");
	luasql_getlibtable (*state);
	lua_pushstring(*state, "odbc");
	lua_pushcfunction(*state, create_environment);
	lua_settable(*state, -3);

	create_metatables(*state);
	return 0;
}
