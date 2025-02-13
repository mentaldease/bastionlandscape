///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define BUILDING_LUAPLUS
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#define LUA_CORE
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaState.h"
#include "LuaCall.h"
#include "LuaPlusCD.h"

#if defined(WIN32) && !defined(_XBOX)
#include <windows.h>
#undef GetObject
#undef LoadString
#elif defined(_XBOX)
#include <xtl.h>
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(disable: 4702)
#endif // _MSC_VER

#include <ctype.h>

LUA_EXTERN_C_BEGIN
#include "src/lstate.h"
#include "src/lvm.h"
#include "src/lgc.h"
LUA_EXTERN_C_END


#ifndef LUAPLUS_ENABLE_INLINES
#include "LuaState.inl"
#endif // LUAPLUS_ENABLE_INLINES

extern "C" int str_format_helper (luaL_Buffer* b, lua_State *L, int arg);

namespace LuaPlus {

LuaStackObject LuaState::Stack(int index)
{
    return LuaStackObject(this, index);
}

LuaStackObject LuaState::StackTop()
{
    return LuaStackObject(this, GetTop());
}

LuaStackObject LuaState::PushVFString(const char *fmt, va_list argp)
{
	lua_State* L = m_state;
	lua_lock(L);
	luaC_checkGC(L);
	luaO_pushvfstring(L, fmt, argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(m_state));
}


LuaStackObject LuaState::PushFString(const char *fmt, ...)
{
	lua_State* L = m_state;
	va_list argp;
	lua_lock(L);
	luaC_checkGC(L);
	va_start(argp, fmt);
	luaO_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_unlock(L);
	return LuaStackObject(this, lua_gettop(m_state));
}


class LUAPLUS_CLASS LuaStateOutputDebugStringFile : public LuaStateOutFile
{
public:
	LuaStateOutputDebugStringFile() {}
	virtual ~LuaStateOutputDebugStringFile() {}

	virtual bool Open(const char* fileName)
	{
		(void)fileName;
		return true;
	}

	virtual void Close()
	{
	}

	virtual void Print(const char* str, ...)
	{
		char message[800];
		va_list arglist;

		va_start(arglist, str);
		vsprintf(message, str, arglist);
		va_end(arglist);

#if defined(WIN32) || defined(_XBOX)
		OutputDebugString(message);
#else // !WIN32
		puts(message);
#endif // WIN32
	}

protected:
};


static void luaI_addquotedbinary (LuaStateOutFile& file, const char* s, int l)
{
	file.Print("%c", '"');
	while (l--)
	{
		switch (*s)
		{
			case '"':  case '\\':
				file.Print("\\%c", *s);
				break;
			case '\a':		file.Print("\\a");		break;
			case '\b':		file.Print("\\b");		break;
			case '\f':		file.Print("\\f");		break;
			case '\n':		file.Print("\\n");		break;
			case '\r':		file.Print("\\r");		break;
			case '\t':		file.Print("\\t");		break;
			case '\v':		file.Print("\\v");		break;
			default:
				if (isprint((unsigned char)*s))
					file.Print("%c", *s);
				else
				{
					file.Print("\\x%02x", (unsigned int)(unsigned char)*s);
				}
		}
		s++;
	}
	file.Print("%c", '"');
}


static void luaI_addquotedwidebinary (LuaStateOutFile& file, const lua_WChar* s, int l)
{
	file.Print("L\"");
	while (l--)
	{
		switch (*s)
		{
			case '"':
			case '\\':
				file.Print("\\%c", *s);
				break;
			case '\a':		file.Print("\\a");		break;
			case '\b':		file.Print("\\b");		break;
			case '\f':		file.Print("\\f");		break;
			case '\n':		file.Print("\\n");		break;
			case '\r':		file.Print("\\r");		break;
			case '\t':		file.Print("\\t");		break;
			case '\v':		file.Print("\\v");		break;
			default:
				if (*s < 256  &&  isprint((unsigned char)*s))
				{
					file.Print("%c", *s);
				}
				else
				{
					file.Print("\\x%04x", (unsigned int)*s);
				}
		}
		s++;
	}
	file.Print("\"");
}


#define bufflen(B)	((B)->p - (B)->buffer)

static int LS_LuaFilePrint(LuaState* state)
{
	LuaStateOutFile* file = (LuaStateOutFile*)state->Stack(1).GetUserData();

	luaL_Buffer b;
	str_format_helper(&b, *state, 2);

	size_t l = bufflen(&b);
	if (l != 0)
	{
		if (b.isWide)
		{
			luaplus_assert(0);
		}
		else
		{
			luaL_addchar(&b, 0);
			file->Print(b.buffer);
		}
	}

	return 0;
}


static int LS_LuaFileIndent(LuaState* state)
{
	LuaStateOutFile* file = (LuaStateOutFile*)state->Stack(1).GetUserData();
	int indentLevel = (int)state->Stack(2).GetInteger();
	file->Indent((unsigned int)indentLevel);

	return 0;
}


bool LuaState::CallFormatting(LuaObject& tableObj, LuaStateOutFile& file, int indentLevel,
		bool writeAll, bool alphabetical, bool writeTablePointers, unsigned int maxIndentLevel)
{
	LuaObject metaTableObj = tableObj.GetMetaTable();
	if (metaTableObj.IsNil())
		return false;

	LuaObject formattedWriteObj = metaTableObj["FormattedWrite"];
	if (!formattedWriteObj.IsFunction())
		return false;

	LuaState* state = tableObj.GetState();

	{
		LuaObject funcObj = state->GetGlobals()["LuaFilePrint"];
		if (funcObj.IsNil())
		{
			state->GetGlobals().Register("LuaFilePrint", LS_LuaFilePrint);
		}

		funcObj = state->GetGlobals()["LuaFileIndent"];
		if (funcObj.IsNil())
		{
			state->GetGlobals().Register("LuaFileIndent", LS_LuaFileIndent);
		}
	}

	LuaCall call = formattedWriteObj;
	call << &file << tableObj << alphabetical << indentLevel << maxIndentLevel << writeAll << writeTablePointers << LuaRun();

	return true;
}


struct KeyValue
{
	LuaObject key;
	LuaObject value;

	inline bool operator<(const KeyValue& right) const
	{
		if (key.Type() == right.key.Type())
			return key < right.key;
		if (key.IsNumber())
			return true;
		if (key.IsString()  &&  !right.key.IsNumber())
			return true;
		return false;
	}
};


static void WriteKey(LuaStateOutFile& file, LuaObject& key)
{
	if (key.IsNumber())
	{
		char keyName[255];
		sprintf(keyName, "[%.16g]", key.GetNumber());
		file.Print("%s", keyName);
	}
	else if (key.IsString())
	{
		const char* ptr = key.GetString();
		bool isAlphaNumeric = true;
		if (isdigit(*ptr))
			isAlphaNumeric = false;
		while (*ptr)
		{
			if (!isalnum(*ptr)  &&  *ptr != '_')
			{
				isAlphaNumeric = false;
				break;
			}
			ptr++;
		}

		if (isAlphaNumeric)
			file.Print("%s", key.GetString());
		else
		{
			file.Print("[");
			luaI_addquotedbinary(file, key.GetString(), key.StrLen());
			file.Print("]");
		}
	}
}


template<typename E>
class SimpleList
{
public:
	SimpleList() :
		m_pHead(NULL),
		m_pTail(NULL)
	{
	}

	~SimpleList() throw()
	{
		while (m_pHead)
		{
			CNode* pKill = m_pHead;
			m_pHead = m_pHead->m_pNext;
			delete pKill;
		}

		m_pHead = NULL;
		m_pTail = NULL;
	}

	void AddTail(E& element)
	{
		CNode* pNewNode = new CNode(element);
		pNewNode->m_pPrev = m_pTail;
		pNewNode->m_pNext = NULL;

		if (m_pTail)
			m_pTail->m_pNext = pNewNode;
		else
			m_pHead = pNewNode;

		m_pTail = pNewNode;
	}

	void* GetHeadPosition() const throw()
	{
		return m_pHead;
	}

	E& GetNext( void*& pos ) throw()
	{
		CNode* pNode = (CNode*)pos;
		pos = (void*)pNode->m_pNext;
		return pNode->m_element;
	}

	void Sort()
	{
		// Exit if not enough items.
		if (!m_pHead  ||  m_pHead->m_pNext == NULL)
			return;

		CNode* curNode = m_pHead->m_pNext;
		while (curNode)
		{
			CNode* curCheckNode = curNode;
			while (curCheckNode->m_pPrev  &&  curNode->m_element < curCheckNode->m_pPrev->m_element)
			{
				curCheckNode = curCheckNode->m_pPrev;
			}

			CNode* nextNode = curNode->m_pNext;

			if (curNode != curCheckNode)
			{
				// Remove curNode from the list.
				if (curNode == m_pTail)
				{
					m_pTail = curNode->m_pPrev;
				}
				else
				{
					curNode->m_pNext->m_pPrev = curNode->m_pPrev;
				}

				curNode->m_pPrev->m_pNext = curNode->m_pNext;

				// Insert curNode to the new location.
				if (curCheckNode->m_pPrev)
				{
					curCheckNode->m_pPrev->m_pNext = curNode;
				}
				else
				{
					m_pHead = curNode;
				}

				curNode->m_pPrev = curCheckNode->m_pPrev;
				curNode->m_pNext = curCheckNode;
				curCheckNode->m_pPrev = curNode;
			}

			curNode = nextNode;
		}
	}

private:
	class CNode
	{
	public:
		CNode( E& element ) : m_element( element )  { }

		CNode* m_pNext;
		CNode* m_pPrev;
		E m_element;

	private:
		CNode( const CNode& ) throw();
	};

	CNode* m_pHead;
	CNode* m_pTail;

	SimpleList( const SimpleList& ) throw();
	SimpleList& operator=( const SimpleList& ) throw();
};


/**
	Writes a Lua object to a text file.
**/
bool LuaState::DumpObject(LuaStateOutFile& file, LuaObject& key, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	bool alreadyDumpedKey = (flags & 0xF0000000) != 0;
	flags &= ~0xF0000000;

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	// If the variable is user data or a function...
	if (!alreadyDumpedKey  &&  value.IsUserData()  ||  value.IsFunction()  ||  value.IsCFunction())
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if ((flags & DUMP_WRITEALL)  &&  !key.IsNil())
		{
			if (value.IsUserData())
			{
				file.Print("-- ");
				WriteKey(file, key);
				file.Print(" = '!!!USERDATA!!!'\r\n");
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push();
				lua_getinfo(m_state, ">S", &ar);
//				printf("%d\r\n", ar.linedefined);
				file.Print("-- ");
				WriteKey(file, key);
				file.Print(" = '!!!FUNCTION!!! %s %d'\r\n", ar.source, ar.linedefined);
			}
			else
			{
				file.Print("-- ");
				WriteKey(file, key);
				file.Print(" = '!!!CFUNCTION!!!'\r\n");
			}

			return true;
		}

		return false;
	}

	// Indent the line the number of spaces for the current indentation level.
	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;

	if (!alreadyDumpedKey)
	{
		if ((unsigned int)indentLevel < maxIndentLevel)
			file.Indent(indentSpaces);
		else
			file.Print(" ");

		// If the object has a name, write it out.
		if (!key.IsNil())
		{
			WriteKey(file, key);

			file.Print(" = ");
		}
	}

	// If the object's value is a number, write it as a number.
	if (value.IsBoolean())
		file.Print("%s", value.GetBoolean() ? "true" : "false");

	else if (value.IsNumber())
		file.Print(LUA_NUMBER_FMT, value.GetNumber());

	// Or if the object's value is a string, write it as a quoted string.
	else if (value.IsString())
	{
		luaI_addquotedbinary(file, value.GetString(), value.StrLen());
	}

	// Or if the object's value is a string, write it as a quoted string.
	else if (value.IsWString())
	{
		luaI_addquotedwidebinary(file, value.GetWString(), value.StrLen());
	}

	// Otherwise, see if the object's value is a table.
	else if (value.IsTable())
	{
		bool calledFormatting =
				CallFormatting(value, file, indentLevel, (flags & DUMP_WRITEALL) != 0,
					(flags & DUMP_ALPHABETICAL) != 0, (flags & DUMP_WRITETABLEPOINTERS) != 0, maxIndentLevel);
		if (!calledFormatting)
		{
			// Write the table header.
			if (indentLevel != -1)
			{
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\r\n");
					file.Indent(indentSpaces);
				}
				if (flags & DUMP_WRITETABLEPOINTERS)
					file.Print("{ --%8x\r\n", value.GetLuaPointer());
				else
					file.Print("{");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\r\n");
				}
			}

			// Rename, just for ease of reading.
			LuaObject& table = value;

			// upperIndex is the upper index value of a sequential numerical array
			// items.
			int upperIndex = 1;
			bool wroteSemi = false;
			bool hasSequential = false;

			// Block to search for array items.
			{
				// Grab index 1 and index 2 of the table.
				LuaObject value1 = table[1];
				LuaObject value2 = table[2];

				// If they both exist, then there is a sequential list.
				if (!value1.IsNil()  &&  !value2.IsNil())
				{
					// Cycle through the list.
					bool headSequential = true;
					for (; ; ++upperIndex)
					{
						// Try retrieving the table entry at upperIndex.
						LuaObject value = table[upperIndex];

						// If it doesn't exist, then exit the loop.
						if (value.IsNil())
							break;

						// Only add the comma and return if not on the head item.
						if (!headSequential  &&  indentLevel != -1)
						{
							file.Print(",");
							if ((unsigned int)indentLevel + 1 < maxIndentLevel)
							{
								file.Print("\r\n");
							}
						}

						// Write the object as an unnamed entry.
						LuaObject nilObj(this);
						DumpObject(file, nilObj, value, flags, indentLevel + 1, maxIndentLevel);

						// We've definitely passed the head item now.
						headSequential = false;
					}
				}
			}

			// Did we find any sequential table values?
			if (upperIndex > 1)
			{
				hasSequential = true;
			}

			if (flags & DUMP_ALPHABETICAL)
			{
				SimpleList<KeyValue> keys;

				// Cycle through the table.
				for (LuaTableIterator it(table); it; ++it)
				{
					// Retrieve the table entry's key and value.
					LuaObject& key = it.GetKey();

					// Is the key a number?
					if (key.IsNumber())
					{
						// Yes, were there sequential array items in this table?
						if (hasSequential)
						{
							// Is the array item's key an integer?
							lua_Number realNum = key.GetNumber();
							int intNum = (int)realNum;
							if (realNum == (lua_Number)intNum)
							{
								// Yes.  Is it between 1 and upperIndex?
								if (intNum >= 1  &&  intNum < upperIndex)
								{
									// We already wrote it as part of the sequential
									// list.
									continue;
								}
							}
						}
					}

					KeyValue info;
					info.key = key;
					info.value = it.GetValue();
					keys.AddTail(info);
				}

				keys.Sort();

				if (keys.GetHeadPosition() != NULL)
				{
					// If we wrote a sequential list, the value we're about to write
					// is not nil, and we haven't written the semicolon to separate
					// the sequential table entries from the keyed table entries...
					if (hasSequential  &&  indentLevel != -1)
					{
						// Then add a comma (for good measure).
						file.Print(", ");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\r\n");
						}
						wroteSemi = true;
					}
				}

				for (void* keysIt = keys.GetHeadPosition(); keysIt; )
				{
					KeyValue& info = keys.GetNext(keysIt);

					// Write the table entry.
					bool ret = DumpObject(file, info.key, info.value, flags,
							indentLevel + 1, maxIndentLevel);

					// Add a comma after the table entry.
					if (indentLevel != -1  &&  ret)
					{
						file.Print(",");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\r\n");
						}
					}
				}
			}
			else
			{
				// Cycle through the table.
				for (LuaTableIterator it(table); it; ++it)
				{
					// Retrieve the table entry's key and value.
					LuaObject& key = it.GetKey();

					// Is the key a number?
					if (key.IsNumber())
					{
						// Yes, were there sequential array items in this table?
						if (hasSequential)
						{
							// Is the array item's key an integer?
							lua_Number realNum = key.GetNumber();
							int intNum = (int)realNum;
							if (realNum == (lua_Number)intNum)
							{
								// Yes.  Is it between 1 and upperIndex?
								if (intNum >= 1  &&  intNum < upperIndex)
								{
									// We already wrote it as part of the sequential
									// list.
									continue;
								}
							}
						}
					}

					// If we wrote a sequential list, the value we're about to write
					// is not nil, and we haven't written the semicolon to separate
					// the sequential table entries from the keyed table entries...
					if (hasSequential  &&  !value.IsNil()  &&  !wroteSemi)
					{
						// Then add a comma (for good measure).
						if (indentLevel != -1)
						{
							file.Print(", ");
							if ((unsigned int)indentLevel + 1 < maxIndentLevel)
							{
								file.Print("\r\n");
							}
						}
						wroteSemi = true;
					}

					// Write the table entry.
					bool ret = DumpObject(file, key, it.GetValue(), flags,
						indentLevel + 1, maxIndentLevel);

					// Add a comma after the table entry.
					if (ret  &&  indentLevel != -1)
					{
						file.Print(",");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\r\n");
						}
					}
				}
			}

			// If we wrote a sequential list and haven't written a semicolon, then
			// there were no keyed table entries.  Just write the final comma.
			if (hasSequential  &&  !wroteSemi  &&  indentLevel != -1)
			{
				file.Print(",");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\r\n");
				}
			}

			// Indent, with the intent of closing up the table.
			file.Indent(indentSpaces);

			// If the indentation level is 0, then we're at the root position.
			if (indentLevel == 0)
			{
				// Add a couple extra returns for readability's sake.
				file.Print("}");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\r\n\r\n");
				}
			}
			else if (indentLevel > 0)
			{
				// Close the table.  The comma is written when WriteObject()
				// returns from the recursive call.
				file.Print("}");
			}
		}
	}

	// If the indentation level is at the root, then add a return to separate
	// the lines.
	if (indentLevel == 0)
	{
		if ((unsigned int)indentLevel < maxIndentLevel)
		{
			file.Print("\r\n");
		}
	}

	return true;
}


/**
	Writes a Lua object to a text file.
**/
bool LuaState::DumpObject(LuaStateOutFile& file, const char* name, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	// Yes, this is hack-ish.

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	// If the variable is user data or a function...
	if (value.IsUserData()  ||  value.IsFunction()  ||  value.IsCFunction())
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if ((flags & DUMP_WRITEALL)  &&  name)
		{
			if (value.IsUserData())
			{
				file.Print("-- %s", name);
				file.Print(" = '!!!USERDATA!!!'\r\n");
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push();
				lua_getinfo(m_state, ">S", &ar);
//				printf("%d\r\n", ar.linedefined);
				file.Print("-- %s", name);
				file.Print(" = '!!!FUNCTION!!! %s %d'\r\n", ar.source, ar.linedefined);
			}
			else
			{
				file.Print("-- %s", name);
				file.Print(" = '!!!CFUNCTION!!!'\r\n");
			}

			return true;
		}

		return false;
	}

	// Indent the line the number of spaces for the current indentation level.
	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;
	if ((unsigned int)indentLevel < maxIndentLevel)
		file.Indent(indentSpaces);
	else
		file.Print(" ");

	// If the object has a name, write it out.
	if (name)
	{
		file.Print("%s = ", name);
	}

	LuaObject key(this);
	bool ret = DumpObject(file, key, value, flags | 0xF0000000, indentLevel, maxIndentLevel);
	file.Print("\n");
	return ret;
}


/**
	Save the complete script state.
**/
bool LuaState::DumpObject(const char* filename, LuaObject& key, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	if (!key.IsString())
	{
		// Open the text file to write the script state to.
		LuaStateOutFile regFile;
		LuaStateOutputDebugStringFile odsFile;

		LuaStateOutFile* file;

		if (strcmp(filename, "@") == 0)
		{
			file = &odsFile;
		}
		else
		{
			file = &regFile;
			if (!file->Open(filename))
				return false;
		}

		return DumpObject(*file, key, value, flags, indentLevel, maxIndentLevel);
	}
	else
	{
		return DumpObject(filename, key.GetString(), value, flags, indentLevel, maxIndentLevel);
	}
}


/**
	Save the complete script state.
**/
bool LuaState::DumpObject(const char* filename, const char* name, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	// Open the text file to write the script state to.
	LuaStateOutFile regFile;
	LuaStateOutputDebugStringFile odsFile;

	LuaStateOutFile* file;

	if (strcmp(filename, "@") == 0)
	{
		file = &odsFile;
	}
	else
	{
		file = &regFile;
		if (!file->Open(filename))
			return false;
	}

	// Yes, this is hack-ish.

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	// If the variable is user data or a function...
	if (value.IsUserData()  ||  value.IsFunction()  ||  value.IsCFunction())
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if ((flags & DUMP_WRITEALL)  &&  name)
		{
			if (value.IsUserData())
			{
				file->Print("-- %s", name);
				file->Print(" = '!!!USERDATA!!!'\r\n");
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push();
				lua_getinfo(m_state, ">S", &ar);
//				printf("%d\r\n", ar.linedefined);
				file->Print("-- %s", name);
				file->Print(" = '!!!FUNCTION!!! %s %d'\r\n", ar.source, ar.linedefined);
			}
			else
			{
				file->Print("-- %s", name);
				file->Print(" = '!!!CFUNCTION!!!'\r\n");
			}

			return true;
		}

		return false;
	}

	// Indent the line the number of spaces for the current indentation level.
	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;
	if ((unsigned int)indentLevel < maxIndentLevel)
		file->Indent(indentSpaces);
	else
		file->Print(" ");

	// If the object has a name, write it out.
	if (name)
	{
		file->Print("%s = ", name);
	}

	LuaObject key(this);
	bool ret = DumpObject(*file, key, value, flags | 0xF0000000, indentLevel, maxIndentLevel);
	file->Print("\n");
	return ret;
}


/**
	Adds [indentLevel] number of spaces to the file.
**/
void LuaStateOutFile::Indent(unsigned int indentLevel)
{
	// Write out indentation.
	char spaces[500];
	unsigned int i;
	for (i = 0; i < indentLevel; ++i)
		spaces[i] = '\t';
	spaces[i] = 0;
	Print(spaces);
}


/**
	Save the complete script state.
**/
bool LuaState::DumpGlobals(const char* filename, unsigned int flags, unsigned int maxIndentLevel)
{
	// Open the text file to write the script state to.
	LuaStateOutFile file;
	if (!file.Open(filename))
		return false;

	return DumpGlobals(file, flags, maxIndentLevel);
}


/**
	Save the complete script state.
**/
bool LuaState::DumpGlobals(LuaStateOutFile& file, unsigned int flags, unsigned int maxIndentLevel)
{
	LuaObject globalsObj = GetGlobals();

	for (LuaTableIterator it(globalsObj); it; ++it)
	{
		// Don't try and dump the globals table.
		if (!(it.GetValue() == globalsObj))
		{
			// Run through all the globals.
			DumpObject(file, it.GetKey(), it.GetValue(), flags, 0, maxIndentLevel);
		}
	}

	return true;
}






int LuaState::Equal(const LuaObject& o1, const LuaObject& o2)
{
	int i = equalobj(o1.GetCState(), o1.GetTObject(), o2.GetTObject());
	return i;
}


int LuaState::LessThan(const LuaObject& o1, const LuaObject& o2)
{
	int i = luaV_lessthan(o1.GetCState(), o1.GetTObject(), o2.GetTObject());
	return i;
}


LuaObject LuaState::NewUserDataBox(void* u)
{
	LuaObject obj(this);
	obj.AssignUserData(this, u);
	return obj;
}


int LuaState::TypeError(int narg, const char* tname)
{
	return luaL_typerror(m_state, narg, tname);
}


int LuaState::ArgError(int narg, const char* extramsg)
{
	return luaL_argerror(m_state, narg, extramsg);
}


const char* LuaState::CheckLString(int numArg, size_t* len)
{
	return luaL_checklstring(m_state, numArg, len);
}


const char* LuaState::OptLString(int numArg, const char *def, size_t* len)
{
	return luaL_optlstring(m_state, numArg, def, len);
}


lua_Number LuaState::CheckNumber(int numArg)
{
	return luaL_checknumber(m_state, numArg);
}


lua_Number LuaState::OptNumber(int nArg, lua_Number def)
{
	return luaL_optnumber(m_state, nArg, def);
}


void LuaState::ArgCheck(bool condition, int numarg, const char* extramsg)
{
	luaL_argcheck(m_state, condition, numarg, extramsg);
}


const char* LuaState::CheckString(int numArg)
{
	return luaL_checkstring(m_state, numArg);
}


const char* LuaState::OptString(int numArg, const char* def)
{
	return luaL_optlstring(m_state, numArg, def, NULL);
}


int LuaState::CheckInt(int numArg)
{
	return (int)luaL_checkint(m_state, numArg);
}


long LuaState::CheckLong(int numArg)
{
	return (long)luaL_checklong(m_state, numArg);
}


int LuaState::OptInt(int numArg, int def)
{
	return (int)luaL_optint(m_state, numArg, def);
}


long LuaState::OptLong(int numArg, int def)
{
	return (long)luaL_optlong(m_state, numArg, def);
}


void LuaState::CheckStack(int sz, const char* msg)
{
	luaL_checkstack(m_state, sz, msg);
}


void LuaState::CheckType(int narg, int t)
{
	luaL_checktype(m_state, narg, t);
}


void LuaState::CheckAny(int narg)
{
	luaL_checkany(m_state, narg);
}


void* LuaState::CheckUData(int ud, const char* tname)
{
	return luaL_checkudata(m_state, ud, tname);
}


static size_t lua_WChar_len(const lua_WChar* str)
{
	const lua_WChar* ptr = str;
	while (*ptr != 0)
	{
		ptr++;
	}

	return (size_t)(ptr - str);
}

LuaObject LuaState::GetLocalByName( int level, const char* name )
{
	lua_State * L = GetCState();
	lua_Debug ar;
	int i;
	const char *localName;
	if (lua_getstack(L, level, &ar) == 0)
		return LuaObject(this);  /* failure: no such level in the stack */
	i = 1;
	while ((localName = lua_getlocal(L, &ar, i++)) != NULL) {
		if (strcmp(name, localName) == 0)
		{
			LuaObject obj(this, -1);
			lua_pop(L, 1);
			return obj;
		}
		lua_pop(L, 1);  /* remove variable value */
	}
	return LuaObject(this);
}



int LuaState::LoadWString(const lua_WChar* str)
{
	return luaL_loadwbuffer(m_state, str, lua_WChar_len(str), "name");
}


LuaStackObject LuaState::PushCClosure(int (*f)(LuaState*), int n)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(m_state, sizeof(f));
	memcpy(buffer, &f, sizeof(f));
	Insert(-n-1);
	lua_pushcclosure(m_state, LPCD::LuaStateFunctionDispatcher, n + 1);
	return LuaStackObject(this, lua_gettop(m_state));
}

int LuaState::UpValueIndex(int i)
{
	return lua_upvalueindex(i);
}

int LuaState::LoadString(const char* str)
{
	return luaL_loadbuffer(m_state, str, strlen(str), str);
}

int LuaState::DoString( const char *str, LuaObject& fenvObj )
{
	int status = luaL_loadbuffer(m_state, str, strlen(str), str);
	if (status == 0)
	{
		fenvObj.Push();
		SetFEnv(-2);
	}
	return aux_do(m_state, status);
}


LuaObject LuaState::GetGlobal(const char *name)
{
	return GetGlobals()[name];
}


LuaObject LuaState::GetRegistry()
{
	return LuaObject(this, LUA_REGISTRYINDEX);  //{  lua_getregistry(m_state);
}


int LuaState::DoFile( const char *filename, LuaObject& fenvObj )
{
	int status = luaL_loadfile(m_state, filename);
	if (status == 0)
	{
		fenvObj.Push();
		SetFEnv(-2);
	}
	return aux_do(m_state, status);
}


int LuaState::DoBuffer( const char *buff, size_t size, const char *name, LuaObject& fenvObj )
{
	int status = luaL_loadbuffer(m_state, buff, size, name);
	if (status == 0)
	{
		fenvObj.Push();
		SetFEnv(-2);
	}
	return aux_do(m_state, status);
}


namespace LuaHelper {

LUAPLUS_API void MergeObjects(LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates)
{
	if (mergeTo.GetState() == mergeFrom.GetState())
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject toNodeKeyObj = mergeTo[it.GetKey()];
			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				mergeTo.SetObject(it.GetKey(), it.GetValue());
			}
		}
	}
	else
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject obj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	obj.AssignBoolean(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:	obj.AssignNumber(mergeTo.GetState(), it.GetKey().GetNumber());			break;
				case LUA_TSTRING:	obj.AssignString(mergeTo.GetState(), it.GetKey().GetString());			break;
				case LUA_TWSTRING:	obj.AssignWString(mergeTo.GetState(), it.GetKey().GetWString());		break;
			}

			LuaObject toNodeKeyObj = mergeTo[obj];

			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				LuaObject toKeyObj;
				switch (it.GetKey().Type())
				{
					case LUA_TBOOLEAN:	toKeyObj.AssignBoolean(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
					case LUA_TNUMBER:	toKeyObj.AssignNumber(mergeTo.GetState(), it.GetKey().GetNumber());			break;
					case LUA_TSTRING:	toKeyObj.AssignString(mergeTo.GetState(), it.GetKey().GetString());			break;
					case LUA_TWSTRING:	toKeyObj.AssignWString(mergeTo.GetState(), it.GetKey().GetWString());		break;
				}

				switch (it.GetValue().Type())
				{
					case LUA_TBOOLEAN:	mergeTo.SetBoolean(toKeyObj, it.GetValue().GetBoolean());	break;
					case LUA_TNUMBER:	mergeTo.SetNumber(toKeyObj, it.GetValue().GetNumber());		break;
					case LUA_TSTRING:	mergeTo.SetString(toKeyObj, it.GetValue().GetString());		break;
					case LUA_TWSTRING:	mergeTo.SetWString(toKeyObj, it.GetValue().GetWString());	break;
				}
			}
		}
	}
}

} // namespace LuaHelper

} // namespace LuaPlus


namespace LPCD
{
	void Push(lua_State* L, int (*value)(LuaState*))
	{
		LuaState* state = LuaState::CastState(L);
		state->PushCClosure(value, 0);
	}
}
