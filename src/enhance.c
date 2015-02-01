#include "enhance.h"

LUA_API int luaopen_enhance(lua_State*L) {
  lua_newtable(L);
  luaopen_mem(L);
  lua_setfield(L, -2, "mem");

  return 1;
};
