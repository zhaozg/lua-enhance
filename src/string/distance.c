/* linha de compilacao:
 * gcc -I/usr/local/include -shared -fPIC -Wall -O2 -o stringdistance.so stringdistance.c
 * $Id: stringdistance.c,v 1.7 2011-02-08 18:30:41 mmota Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"

#define SD_NAME "String Distance is a Lua implementation of the Damerau-Levenshtein and Levenshtein algorithms"
#define SD_VERSION "1.0.0"
#define SD_RELEASE "$Id: stringdistance.c,v 1.7 2011-02-08 18:30:41 mmota Exp $"


/* Auxiliar functions to Get the minimum between values */
static int min3(int a,int b,int c)
{
	int min = a;
	if(b < min)
		min = b;
	if(c < min)
		min = c;
	return min;
}

static int min4(int a,int b,int c,int d)
{
	int min = a;
	if(b < min)
		min = b;
	if(c < min)
		min = c;
	if(d < min)
		min = d;
	return min;
}

/*
 * Compute levenshtein distance between s and t 
 * if case_ins = 1 comparison is case insensitive else comp is case sensitive
 * @return distance
 */
 
static int levenshtein(const char *s, const char *t)
{
	// Step 1 
	int k, i, j, n, m, cost, *d, distance;
	n = strlen(s); 
	m = strlen(t);
	if (n == 0)
		return m;
	else if (m == 0)
			return n;
		else
		{
			m++;
			n++;
			d = malloc((sizeof(int))*m*n);
			// Step 2 
			for(k = 0; k < n; k++)
				d[k] = k;
			for(k = 0; k < m; k++)
				d[k*n] = k;
			// Step 3 and 4 
			for(i = 1; i < n; i++)
				for(j = 1; j < m; j++){
					// Step 5 
					int diff = s[i-1] - t[j-1];
					// letras iguais 
					if (diff == 0)
						cost = 0;
					else
						cost = 1;
					// Step 6 
					d[j*n+i] = min3(
					                d[(j-1)*n+i] + 1,
					                d[j*n+i-1] + 1,
					                d[(j-1)*n+i-1] + cost);
				}
			distance = d[n*m-1];
			free(d);
			return distance;
		}
} 

static int damerauLevenshtein(const char *s, const char *t)
{
    int DS[256];

	int i, j, distance, cost;
	int n = strlen(s); 
	int m = strlen(t); 
	int INF = n + m; 
	int *d = malloc((sizeof(int))*(n+2)*(m+2));	 
	d[0] = INF;

    for(i = 0; i <= n; i++) {
        d[((n+2)*1) + i+1] = i;
        d[((n+2)*0) + i+1] = INF;
    } 
    for(j = 0; j <= m; j++) {
        d[((n+2)*(j+1)) + 1] = j;
        d[((n+2)*(j+1)) + 0] = INF;
    } 

	for(i = 0; i < sizeof(char)*256; i++) {
		DS[i] = 0;
    }
    
	for(i = 1; i <= n; i++)
    {
        int DT = 0;
        int si;
        for(j = 1; j <= m; j++)
        {
            int tj = (unsigned char)t[j-1];
            int i1 = DS[tj];
            int j1 = DT;
            int diff = s[i-1] - t[j-1]; 
            if (diff == 0){
				cost = 0;
				DT = j;
            }
            else
                cost = 1;
            {
                int substitution = d[((n+2)*j) + i] + cost;
                int insertion = d[((n+2)*j) + i+1] + 1;
                int deletion = d[((n+2)*(j+1)) + i] + 1;
                int transposition = d[((n+2)*j1) + i1] + (i-i1-1) + 1 + (j-j1-1);
                d[((n+2)*(j+1)) + i+1] = min4(
                    substitution,
                    insertion,
                    deletion,
                    transposition);
            }
        }
        si = (unsigned char)s[i-1];
        DS[si] = i;
    }
	distance = d[((n+2)*(m+1)) + (n+1)]; 
	free(d);
	return distance;
}

static int _lev(lua_State *L) {
	int dist;
	const char *s1 = luaL_checkstring(L, 1);
	const char *s2 = luaL_checkstring(L, 2);
	lua_settop(L, 2);  /* erase extra arguments */
	dist = levenshtein(s1, s2);
	lua_pushinteger(L, (int) dist);
	return 1;
}

static int _damerauLevenshtein(lua_State *L) {
	int dist;
	const char *s1 = luaL_checkstring(L, 1);
	const char *s2 = luaL_checkstring(L, 2);
	lua_settop(L, 2);  /* erase extra arguments */
	dist = damerauLevenshtein(s1, s2);
	lua_pushinteger(L, (int) dist);
	return 1;
}

static const struct luaL_Reg stringdistance [] = {
	{"lev", _lev},
	{"dam", _damerauLevenshtein},
	{NULL, NULL},
};

int string_distance(lua_State *L) {
    const char *s1 = luaL_checkstring(L, 1);
    const char *s2 = luaL_checkstring(L, 2);
    const char *alg = luaL_optstring(L, 3, "dam");
    int dist = 0;
    if(strcmp(alg,"lev")==0)
    {
        dist = levenshtein(s1, s2);
    }else if(strcmp(alg,"dam")==0)
    {
        dist = damerauLevenshtein(s1, s2);
    }else
    {
        luaL_error(L, "#3 arg must be lev or dam\n"
            "\tlev for Levenshtein algorithms\n"
            "\tdam for Damerau-Levenshtein algorithms\n");
    }

	return 1;
}
