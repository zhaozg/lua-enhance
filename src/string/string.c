#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

static int endswith(lua_State *L) {
    const char *string = luaL_checkstring(L, 1);
    int string_len = lua_objlen(L, 1);

    const char *token = luaL_checkstring(L, 2);
    int token_len = lua_objlen(L, 2);

    int ti = token_len, si = string_len, end = 1;
    if(token_len <= string_len){
        while(ti > 0) {
            if(string[--si] != token[--ti]){ 
                end = 0;
                break;

            }
        }
    }
    else {
        end = 0;
    }
    lua_pushboolean(L, end);
    return 1;
}

static int startswith(lua_State *L) {
    const char *string = luaL_checkstring(L, 1);
    int string_len = lua_objlen(L, 1);

    const char *token = luaL_checkstring(L, 2);
    int token_len = lua_objlen(L, 2);
    int i=0, start = 1;
    // please make this less ugly... 
    if(token_len <= string_len){
        while(i < token_len) {
            if(string[i] != token[i]){
                start = 0;
                break;
            }
            i++;
        }
    }
    else {
        start = 0;
    }
    lua_pushboolean(L, start);
    return 1;
}

static int split(lua_State *L) {
    const char *string = luaL_checkstring(L, 1);
    const char *sep = luaL_checkstring(L, 2);
    const char *token;
    int i = 1;
    lua_newtable(L);
    while ((token = strchr(string, *sep)) != NULL) {
        lua_pushlstring(L, string, token - string);
        lua_rawseti(L, -2, i++);
        string = token + 1;
    }
    lua_pushstring(L, string);
    lua_rawseti(L, -2, i);
    return 1;
}

// http://lua-users.org/wiki/StringTrim
static int strip(lua_State *L) {
    const char *front;
    const char *end;
    size_t      size;

    front = luaL_checklstring(L, 1, &size);
    end   = &front[size - 1];

    for ( ; size && isspace(*front) ; size-- , front++)
    ;
    for ( ; size && isspace(*end) ; size-- , end--)
    ;

    lua_pushlstring(L, front, (size_t)(end - front) + 1);
    return 1;
}
unsigned int SDBMHash(char *str)
{
    unsigned int hash = 0;

    while (*str)
    {
        // equivalent to: hash = 65599*hash + (*str++);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }

    return (hash & 0x7FFFFFFF);
}

// RS Hash Function
unsigned int RSHash(char *str)
{
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * a + (*str++);
        a *= b;
    }

    return (hash & 0x7FFFFFFF);
}

// JS Hash Function
unsigned int JSHash(char *str)
{
    unsigned int hash = 1315423911;

    while (*str)
    {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }

    return (hash & 0x7FFFFFFF);
}

// P. J. Weinberger Hash Function
unsigned int PJWHash(char *str)
{
    unsigned int BitsInUnignedInt = (unsigned int)(sizeof(unsigned int) * 8);
    unsigned int ThreeQuarters	= (unsigned int)((BitsInUnignedInt  * 3) / 4);
    unsigned int OneEighth		= (unsigned int)(BitsInUnignedInt / 8);
    unsigned int HighBits		 = (unsigned int)(0xFFFFFFFF) << (BitsInUnignedInt - OneEighth);
    unsigned int hash			 = 0;
    unsigned int test			 = 0;

    while (*str)
    {
        hash = (hash << OneEighth) + (*str++);
        if ((test = hash & HighBits) != 0)
        {
            hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return (hash & 0x7FFFFFFF);
}

// ELF Hash Function
unsigned int ELFHash(char *str)
{
    unsigned int hash = 0;
    unsigned int x	= 0;

    while (*str)
    {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash & 0x7FFFFFFF);
}

// BKDR Hash Function
unsigned int BKDRHash(const char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

// DJB Hash Function
unsigned int DJBHash(char *str)
{
    unsigned int hash = 5381;

    while (*str)
    {
        hash += (hash << 5) + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

// AP Hash Function
unsigned int APHash(char *str)
{
    unsigned int hash = 0;
    int i;

    for (i=0; *str; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }

    return (hash & 0x7FFFFFFF);
}

static int hash(lua_State *L) {
    const char* str = luaL_checkstring(L, 1);
    unsigned int h = BKDRHash(str);
    lua_pushinteger(L, h);
    return 1;
}

static const char hex[] = {'0','1','2','3', '4','5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static const char bin[256] =
{
    /*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
    /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    /* 40 */ 0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 60 */ 0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 90 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* a0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* b0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* c0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* d0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* e0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* f0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


static int hex2bin(lua_State *L) {
    size_t l;
    size_t i;
    const char* s = luaL_checklstring(L, 1, &l);
    luaL_Buffer B;
    luaL_buffinit(L, &B);
    for(i=0; i<l; i+=2)
    {
        unsigned char h = s[i];
        unsigned char l = s[i+1];
        luaL_addchar(&B,bin[h]<<4 | bin[l]);
    }
    luaL_pushresult(&B);
    return 1;
}
static int bin2hex(lua_State *L) {
    int l;
    int i;
    const char* s = luaL_checklstring(L, 1, &l);
	int b = luaL_optint(L, 2, 1);
	int e = luaL_optint(L, 3, l-b);

	luaL_Buffer B;
	if(l==0){
		lua_pushstring(L,"");
		return 1;
	}
	
	if(e>l-b)
		e = l-b;
	
    luaL_buffinit(L, &B);
    for(i=0; i<=e; i++)
    {
        unsigned char c = s[i+b-1];
        luaL_addchar(&B, hex[c>>4]);
        luaL_addchar(&B, hex[c&0xf]);
    }
    luaL_pushresult(&B);
    return 1;
}

int string_distance(lua_State *L);
const luaL_reg extstr[] = {
    {"split", split},
    {"strip", strip},
    {"startswith", startswith},
    {"endswith", endswith},
    {"hash", hash},
    {"bin2hex", bin2hex},
    {"hex2bin", hex2bin},
    {"distance",string_distance},
    {NULL, NULL}
};
