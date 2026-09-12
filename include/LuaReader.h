#pragma once

#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>

#include <string>
#include <unordered_map>

namespace term {
    class Lua
    {
        public:
            Lua();
            ~Lua();

            enum Type {
                L_ERROR,
                NIL,
                NUMBER,
                BOOL,
                STRING,
            }; // if err, store type in string

            struct Variable {
                union Value {
                    lua_Number number;
                    bool boolean;
                    const char* string;
                } value;
                Type type{NIL};
            };

            void AccessTableFromFile(const std::string& file, const std::string& tableName);

            Variable GetFieldFromTable(const std::string& tableName, const std::string& fieldName);

            bool GetError() { return m_error; }

        private:
            lua_State* m_L{nullptr};

            std::unordered_map<std::string, uint32_t> m_luaStack;

            bool m_error{false};
    }; 
}