#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "perl_interpreter.h"

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int arg)
{
    errno = 0;
    switch (*name) {
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static XS(XS_Perl_cmiss_constant);

MODULE = Cmiss::Perl_cmiss		PACKAGE = Cmiss::Perl_cmiss		


double
constant(name,arg)
	char *		name
	int		arg

