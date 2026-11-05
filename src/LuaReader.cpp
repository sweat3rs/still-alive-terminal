//  Copyright (C) 2026  Miradge/Sweat3rs
#include "LuaReader.h"

#include <iostream>

using namespace term;

Lua::Lua()
{
    m_L = luaL_newstate();
    luaL_openlibs(m_L);
}

Lua::~Lua()
{
    lua_close(m_L);
}

void Lua::AccessTableFromFile(const std::string& file, const std::string& tableName)
{
    int32_t result = luaL_dofile(m_L, file.c_str());
    if (result == LUA_OK) {
        lua_getglobal(m_L, tableName.c_str());
        if (lua_istable(m_L, -1)) {
            m_luaStack.emplace(tableName, lua_gettop(m_L));
        }
        else {
            std::cout << "[C++] Could not find given table name in file" << std::endl;
            //luaL_error(m_L, "Error: %s\n", lua_tostring(m_L, -1));
            lua_pop(m_L, 1);
            m_error = true;
        }
    } else {
        std::cout << "[C++] Error reading script: " << lua_tostring(m_L, -1) << std::endl;
        m_error = true;
    }
}

Lua::Variable Lua::GetFieldFromTable(const std::string& tableName, const std::string& fieldName)
{
    if (!m_luaStack.contains(tableName) || !lua_istable(m_L, m_luaStack.find(tableName)->second)) {
        auto var = Variable{{0}, L_ERROR};
        var.value.string = "Table doesnt exist";
        return var;
    }
    
    lua_getfield(m_L, m_luaStack.find(tableName)->second, fieldName.c_str());

    auto var = Variable{{0}, NIL};

    switch (lua_type(m_L, -1)) {
        case LUA_TNIL:
            break;
        case LUA_TNUMBER:
            var.type = NUMBER;
            var.value.number = lua_tonumber(m_L, -1);
            break;
        case LUA_TBOOLEAN:
            var.type = BOOL;
            var.value.boolean = lua_toboolean(m_L, -1);
            break;
        case LUA_TSTRING:
            var.type = STRING;
            var.value.string = lua_tostring(m_L, -1);
            break;
        default:
            var.type = L_ERROR;
            var.value.string = "Field not a valid type";
            break;
    }

    lua_pop(m_L, 1);
    return var;
}