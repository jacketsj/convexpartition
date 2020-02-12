#pragma once
#include <bits/stdc++.h>
using namespace std;

typedef string val;

//template <typename... Args>
//replace val with Args?
val param(val... args)
{
	val ret = "("; // start with a parenthese
	for (auto v : {args...})
		ret += v + " ";
	ret[ret.size()-1] = ")"; // replace the last space with a parenthese
	return ret;
}

val sub(const val &a, const val &b)
{
	return param("bvsub", a, b);
}

val mult(const val &a, const val &b)
{
	return param("bvmul", a, b);
}

//TODO name
val signed_le(const val &a, const val &b)
{
	return param("bvsle", a, b);
}

val loc_le(const val &a, const val &b)
{
	//TODO not bvslt
	return param("bvsle", a, b);
}

// we use cross products both for intersections and for angle constraints in the adjacency list
val cross(const val &x, const val &y, const val &ox, const val &oy)
{
	return val("(bvsub (bvmul " + x + " " + oy + ") (bvmul " + ox + " " + y + "))");
}

// cross product indicates <=180 degrees
auto cross_max(const val &x, const val &y, const val &ox, const val &oy)
{
	// signed <=
	return val("(bvsle " + cross(x,y,ox,oy) + literal(0) + ")");
}
