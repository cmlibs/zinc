#ifdef UIL_CODE
#include "UxXt.h"
#else
#include "UxLib.h"
#endif
#include "XvgGlobals.h"

#ifdef SGI
#define e04uef_defaults    e04uef_defaults_
#define e04uef_func_prec   e04uef_func_prec_
#define e04uef_verify      e04uef_verify_
#endif

void e04uef_func_prec(void)
{
	e04uef("Function Precision 1.0E-6");
}

void e04uef_defaults(void)
{
	e04uef("Defaults");
}

void e04uef_verify(void)
{
	e04uef("Verify Level 3");
}
