/*
**  OpenSSL.xs
*/

#include "openssl.h"

SV *
new_ref(type, obj, mort)
  char *type;
  char *obj;
{
    SV *ret;

    if (mort)
        ret = sv_newmortal();
    else
        ret = newSViv(0);
#ifdef DEBUG
    printf(">new_ref %d\n",type);
#endif
    sv_setref_pv(ret, type, (void *)obj);
    return(ret);
}

int 
ex_new(obj, data, ad, idx, argl, argp)
  char *obj;
  SV *data;
  CRYPTO_EX_DATA *ad;
  int idx;
  long argl;
  char *argp;
{
    SV *sv;

#ifdef DEBUG
    printf("ex_new %08X %s\n",obj,argp); 
#endif
    sv = sv_newmortal();
    sv_setref_pv(sv, argp, (void *)obj);
#ifdef DEBUG
    printf("%d>new_ref '%s'\n", sv, argp);
#endif
    CRYPTO_set_ex_data(ad, idx, (char *)sv);
    return(1);
}

void 
ex_cleanup(obj, data, ad, idx, argl, argp)
  char *obj;
  SV *data;
  CRYPTO_EX_DATA *ad;
  int idx;
  long argl;
  char *argp;
{
    pr_name("ex_cleanup");
#ifdef DEBUG
    printf("ex_cleanup %08X %s\n", obj, argp);
#endif
    if (data != NULL)
        SvREFCNT_dec((SV *)data);
}

MODULE = OpenSSL  PACKAGE = OpenSSL

PROTOTYPES: ENABLE

BOOT:
    boot_bio();
    boot_cipher();
    boot_digest();
    boot_err();
    boot_ssl();

	/*								*/
	/* The next macro is the completely correct way to call a C	*/
	/* function that uses perl calling conventions but is not	*/
	/* registered with perl.					*/
	/*								*/
	/* The second macro seems to work for this context.  (We just	*/
	/* need a mark for the called function since we don't have	*/
	/* any local variables and what-not.)				*/
	/*								*/
	/* Unfortunately, we need to do this because these boot_*	*/
	/* functions are auto-generated by xsubpp and are normally	*/
	/* called from DyncLoader, but we're pulling them in here.	*/
	/*								*/
#define FULL_callBootFunc(func) { \
	    dSP; \
	    ENTER; \
	    SAVETMPS; \
	    PUSHMARK(SP); \
    		func(); \
	    FREETMPS; \
	    LEAVE; \
	}
#define callBootFunc(func) { \
	    PUSHMARK(SP); \
    		func(); \
	}
    callBootFunc(boot_OpenSSL__BN);
    callBootFunc(boot_OpenSSL__BIO);
    callBootFunc(boot_OpenSSL__Cipher);
    callBootFunc(boot_OpenSSL__MD);
    callBootFunc(boot_OpenSSL__ERR);
    callBootFunc(boot_OpenSSL__SSL);
    callBootFunc(boot_OpenSSL__X509);

