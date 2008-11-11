#include "ti_native.h"

TiNative::TiNative ()
{
	BindMethod("debug", &TiNative::debug);
}

void TiNative::debug (const CppArgumentList &args, CppVariant *result)
{
	printf("[[TiNative:debug]] %s\n", args[0].ToString().c_str());
}