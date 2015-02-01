#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

static const char *MEM = "ENHANCE_MEM";

typedef struct {
  uint32_t  size;
  uint8_t*  pointer;
}mem;

static int m_malloc(lua_State*L) {
  int size = luaL_checkint(L, 1);
  mem *m = lua_newuserdata(L, sizeof(mem));
  m->pointer = (uint8_t*)malloc(size);
  m->size = size;
  luaL_getmetatable(L, MEM);
  lua_setmetatable(L, -2);
  return 1;
}

static int m_realloc(lua_State*L) {
  mem* m = (mem*)luaL_checkudata(L, 1, MEM);
  uint32_t size = luaL_checkint(L, 2);
  m->pointer = realloc(m->pointer,size);
  m->size = size;
  return 1;
}

static int m_free(lua_State*L) {
  mem* m = (mem*)luaL_checkudata(L, 1, MEM);
  if (m->pointer) {
    free(m->pointer);
    m->size = 0;
  }
  lua_pushnil(L);
  lua_setmetatable(L,1);
  return 0;
}

static int m_tostring(lua_State*L) {
  mem* m = (mem*)luaL_checkudata(L, 1, MEM);
  size_t l = luaL_optint(L, 2, m->size);
  lua_pushlstring(L, m->pointer, l);
  return 1;
};

static int m_index(lua_State*L) {
  mem* m = (mem*)luaL_checkudata(L, 1, MEM);
  if (lua_isnumber(L, 2)) {
    uint32_t loc = lua_tointeger(L, 2);
    if (loc < m->size && loc >= 0) {
      lua_pushinteger(L, m->pointer[loc]);
      return 1;
    }
  }
  else if (lua_isstring(L, 2)) {
    size_t l;
    uint8_t* p;
    const char* s = luaL_checklstring(L, 2, &l);
    l = luaL_optint(L, 3, l);

    for(p = m->pointer; p < m->pointer + m->size - l; p++)
    {
      if (memcmp(p, s, l) == 0) {
        lua_pushinteger(L, p - m->pointer);
        return 1;
      }
    }
  }
  return 0;
}

static int m_newindex(lua_State*L) {
  mem* m = (mem*)luaL_checkudata(L, 1, MEM);
  if (lua_isnumber(L, 2)) {
    uint32_t loc = lua_tointeger(L, 2);
    uint8_t b = luaL_checkint(L, 3);
    if (loc < m->size && loc >= 0) {
      m->pointer[loc] = b;
      return 0;
    }
  }
  else if (lua_isstring(L, 2)) {
    size_t l1,l2;
    uint8_t* p;
    const char* s = luaL_checklstring(L, 2, &l1);
    const char* t = luaL_checklstring(L, 3, &l2);
    if (l1 == l2)
    {
      for (p = m->pointer; p < m->pointer + m->size - l1; p++)
      {
        if (memcmp(p, s, l1) == 0) {
          memcpy(p, t, l2);
          p += l2;
        }
      }
    }
  }
  return 0;
}
static const luaL_Reg mem_fun[] = {
  { "__newindex", m_newindex },
  { "__index", m_index },
  { "__gc", m_free },
  { "__tostring", m_tostring },
  { NULL, NULL }
};

int luaopen_mem(lua_State*L) {
  luaL_newmetatable(L, MEM);
  luaL_register(L, NULL, mem_fun);

  lua_newtable(L);
  lua_pushcfunction(L, m_malloc);
  lua_setfield(L, -2, "malloc");
  lua_pushcfunction(L, m_realloc);
  lua_setfield(L, -2, "realloc");
  lua_pushcfunction(L, m_free);
  lua_setfield(L, -2, "free");
  lua_pushcfunction(L, m_tostring);
  lua_setfield(L, -2, "tostring");

  return 1;
};