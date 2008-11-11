#ifndef TI_NATIVE_H_
#define TI_NATIVE_H_

#include "webkit/glue/cpp_bound_class.h"

class TiNative : public CppBoundClass
{
public:
	TiNative();

	void debug (const CppArgumentList &args, CppVariant *result);
};

#endif