/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

/* User Tracking Module (Was mod_cookies.c)
 *
 * *** IMPORTANT NOTE: This module is not designed to generate 
 * *** cryptographically secure cookies.  This means you should not 
 * *** use cookies generated by this module for authentication purposes
 *
 * This Apache module is designed to track users paths through a site.
 * It uses the client-side state ("Cookie") protocol developed by Netscape.
 * It is known to work on most browsers.
 *
 * Each time a page is requested we look to see if the browser is sending
 * us a Cookie: header that we previously generated.
 *
 * If we don't find one then the user hasn't been to this site since
 * starting their browser or their browser doesn't support cookies.  So
 * we generate a unique Cookie for the transaction and send it back to
 * the browser (via a "Set-Cookie" header)
 * Future requests from the same browser should keep the same Cookie line.
 *
 * By matching up all the requests with the same cookie you can
 * work out exactly what path a user took through your site.  To log
 * the cookie use the " %{Cookie}n " directive in a custom access log;
 *
 * Example 1 : If you currently use the standard Log file format (CLF)
 * and use the command "TransferLog somefilename", add the line
 *       LogFormat "%h %l %u %t \"%r\" %s %b %{Cookie}n"
 * to your config file.
 *
 * Example 2 : If you used to use the old "CookieLog" directive, you
 * can emulate it by adding the following command to your config file
 *       CustomLog filename "%{Cookie}n \"%r\" %t"
 *
 * Mark Cox, mjc@apache.org, 6 July 95
 *
 * This file replaces mod_cookies.c
 */

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include <sys/time.h>

module MODULE_VAR_EXPORT usertrack_module;

typedef struct {
    int always;
    time_t expires;
} cookie_log_state;

typedef enum {
    CT_UNSET,
    CT_NETSCAPE,
    CT_COOKIE,
    CT_COOKIE2
} cookie_type_e;

typedef enum {
    CF_NORMAL,
    CF_COMPACT
} cookie_format_e;

typedef struct {
    int enabled;
    cookie_type_e style;
    cookie_format_e format;
    char *cookie_name;
    char *cookie_domain;
    char *prefix_string;
    char *regexp_string;  /* used to compile regexp; save for debugging */
    regex_t *regexp;  /* used to find usertrack cookie in cookie header */
} cookie_dir_rec;

/* Define this to allow post-2000 cookies. Cookies use two-digit dates,
 * so it might be dicey. (Netscape does it correctly, but others may not)
 */
#define MILLENIAL_COOKIES

/* Default name of the cookie
 */
#define COOKIE_NAME "Apache"


/* Make cookie id: Try to make something unique based on 
 * pid, time, and hostid, plus the user-configurable prefix.
 *
 */
static char * make_cookie_id(char * buffer, int bufsize, request_rec *r,
                             cookie_format_e cformat)
{
    struct timeval tv;
    struct timezone tz = {0, 0};
    char hbuf[NI_MAXHOST];

    cookie_dir_rec *dcfg;

    long reqtime = (long) r->request_time;
    long clocktime;

    getnameinfo((struct sockaddr *)&r->connection->remote_addr,
        r->connection->remote_addr.ss_len,
        hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST);

    const char *rname = ap_get_remote_host(r->connection, r->per_dir_config,
					   REMOTE_NAME);
    dcfg = ap_get_module_config(r->per_dir_config, &usertrack_module);

    gettimeofday(&tv, &tz);

    reqtime = (long) tv.tv_sec;
    if (cformat == CF_COMPACT)
	clocktime = (long) (tv.tv_usec % 65535);
    else
	clocktime = (long) (tv.tv_usec / 1000);

    if (cformat == CF_COMPACT)
	ap_snprintf(buffer, bufsize, "%s%s%x%lx%lx", 
		    dcfg->prefix_string, hbuf, (int) getpid(),
                    reqtime, clocktime);
    else
	ap_snprintf(buffer, bufsize, "%s%s.%d%ld%ld", 
		    dcfg->prefix_string, rname, (int) getpid(),
                    reqtime, clocktime);

    return buffer;
}



static void make_cookie(request_rec *r)
{
    cookie_log_state *cls = ap_get_module_config(r->server->module_config,
						 &usertrack_module);

    /* 1024 == hardcoded constant */
    char cookiebuf[1024];
    char *new_cookie;
    cookie_dir_rec *dcfg;

    dcfg = ap_get_module_config(r->per_dir_config, &usertrack_module);

    make_cookie_id(cookiebuf, sizeof(cookiebuf), r, dcfg->format);

    if (cls->expires) {
        struct tm *tms;
        time_t when;

        when = cls->expires;
        if ((dcfg->style == CT_UNSET) || (dcfg->style == CT_NETSCAPE)) {
            when += r->request_time;

#ifndef MILLENIAL_COOKIES
        /*
         * Only two-digit date string, so we can't trust "00" or more.
         * Therefore, we knock it all back to just before midnight on
         * 1/1/2000 (which is 946684799)
         */

        if (when > 946684799)
            when = 946684799;
#endif
        }
        tms = gmtime(&when);

        /* Cookie with date; as strftime '%a, %d-%h-%y %H:%M:%S GMT' */
        new_cookie = ap_psprintf(r->pool, "%s=%s; path=/",
                                 dcfg->cookie_name, cookiebuf);
        if ((dcfg->style == CT_UNSET) || (dcfg->style == CT_NETSCAPE)) {
            new_cookie = ap_psprintf(r->pool, "%s; "
                                     "expires=%s, %.2d-%s-%.2d "
                                     "%.2d:%.2d:%.2d GMT",
                                     new_cookie,
                                     ap_day_snames[tms->tm_wday],
                                     tms->tm_mday,
                                     ap_month_snames[tms->tm_mon],
                                     tms->tm_year % 100,
                                     tms->tm_hour, tms->tm_min, tms->tm_sec);
        }
        else {
            new_cookie = ap_psprintf(r->pool, "%s; max-age=%d",
                                     new_cookie, (int) when);
        }
    }
    else {
	new_cookie = ap_psprintf(r->pool, "%s=%s; path=/",
				 dcfg->cookie_name, cookiebuf);
    }
    if (dcfg->cookie_domain != NULL) {
        new_cookie = ap_psprintf(r->pool, "%s; domain=%s",
                                 new_cookie, dcfg->cookie_domain);
    }
    if (dcfg->style == CT_COOKIE2) {
        new_cookie = ap_pstrcat(r->pool, new_cookie, "; version=1", NULL);
    }

    ap_table_setn(r->headers_out,
                  (dcfg->style == CT_COOKIE2 ? "Set-Cookie2" : "Set-Cookie"),
                  new_cookie);
    ap_table_setn(r->notes, "cookie", ap_pstrdup(r->pool, cookiebuf));   /* log first time */
    return;
}

/*
 * dcfg->regexp is "^cookie_name=([^;]+)|;[ \t]+cookie_name=([^;]+)",
 * which has three subexpressions, $0..$2
 */
#define NUM_SUBS 3

static void set_and_comp_regexp(cookie_dir_rec *dcfg, 
                                pool *p,
                                const char *cookie_name) 
{
    /*
     * The goal is to end up with this regexp, 
     * ^cookie_name=([^;]+)|;[\t]+cookie_name=([^;]+) 
     * with cookie_name obviously substituted either
     * with the real cookie name set by the user in httpd.conf,
     * or with the default COOKIE_NAME.
     */
    dcfg->regexp_string = ap_pstrcat(p, "^", cookie_name,
                                     "=([^;]+)|;[ \t]+", cookie_name,
                                     "=([^;]+)", NULL);
    dcfg->regexp = ap_pregcomp(p, dcfg->regexp_string, REG_EXTENDED);
}

static int spot_cookie(request_rec *r)
{
    cookie_dir_rec *dcfg = ap_get_module_config(r->per_dir_config,
						&usertrack_module);
    const char *cookie_header;
    regmatch_t regm[NUM_SUBS];

    if (!dcfg->enabled) {
        return DECLINED;
    }

    if ((cookie_header = ap_table_get(r->headers_in,
                                      (dcfg->style == CT_COOKIE2
                                       ? "Cookie2"
                                       : "Cookie")))) {
	if (!ap_regexec(dcfg->regexp, cookie_header, NUM_SUBS, regm, 0)) {
	    char *cookieval = NULL;
	    /* Our regexp,
	     * ^cookie_name=([^;]+)|;[ \t]+cookie_name=([^;]+)
	     * only allows for $1 or $2 to be available. ($0 is always
	     * filled with the entire matched expression, not just
	     * the part in parentheses.) So just check for either one
	     * and assign to cookieval if present. */
	    if (regm[1].rm_so != -1) {
		cookieval = ap_pregsub(r->pool, "$1", cookie_header, 
                                       NUM_SUBS, regm);
	    }
	    if (regm[2].rm_so != -1) {
		cookieval = ap_pregsub(r->pool, "$2", cookie_header, 
                                       NUM_SUBS, regm);
	    }
	    /* Set the cookie in a note, for logging */
	    ap_table_setn(r->notes, "cookie", cookieval);

	    return DECLINED;    /* There's already a cookie, no new one */
	}
    }
    make_cookie(r);
    return OK;                  /* We set our cookie */
}

static void *make_cookie_log_state(pool *p, server_rec *s)
{
    cookie_log_state *cls =
    (cookie_log_state *) ap_palloc(p, sizeof(cookie_log_state));

    cls->expires = 0;

    return (void *) cls;
}

static void *make_cookie_dir(pool *p, char *d)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) ap_pcalloc(p, sizeof(cookie_dir_rec));
    dcfg->cookie_name = COOKIE_NAME;
    dcfg->cookie_domain = NULL;
    dcfg->prefix_string = "";
    dcfg->style = CT_UNSET;
    dcfg->format = CF_NORMAL;
    dcfg->enabled = 0;
    /*
     * In case the user does not use the CookieName directive,
     * we need to compile the regexp for the default cookie name.
     */
    set_and_comp_regexp(dcfg, p, COOKIE_NAME);
    return dcfg;
}

static const char *set_cookie_enable(cmd_parms *cmd, void *mconfig, int arg)
{
    cookie_dir_rec *dcfg = mconfig;

    dcfg->enabled = arg;
    return NULL;
}

static const char *set_cookie_exp(cmd_parms *parms, void *dummy,
                                  const char *arg)
{
    cookie_log_state *cls;
    time_t factor, modifier = 0;
    time_t num = 0;
    char *word;

    cls  = ap_get_module_config(parms->server->module_config,
                                &usertrack_module);
    /* The simple case first - all numbers (we assume) */
    if (ap_isdigit(arg[0]) && ap_isdigit(arg[strlen(arg) - 1])) {
        cls->expires = atol(arg);
        return NULL;
    }

    /*
     * The harder case - stolen from mod_expires 
     *
     * CookieExpires "[plus] {<num> <type>}*"
     */

    word = ap_getword_conf(parms->pool, &arg);
    if (!strncasecmp(word, "plus", 1)) {
        word = ap_getword_conf(parms->pool, &arg);
    };

    /* {<num> <type>}* */
    while (word[0]) {
        /* <num> */
	if (ap_isdigit(word[0]))
            num = atoi(word);
        else
            return "bad expires code, numeric value expected.";

        /* <type> */
        word = ap_getword_conf(parms->pool, &arg);
        if (!word[0])
            return "bad expires code, missing <type>";

        factor = 0;
        if (!strncasecmp(word, "years", 1))
            factor = 60 * 60 * 24 * 365;
        else if (!strncasecmp(word, "months", 2))
            factor = 60 * 60 * 24 * 30;
        else if (!strncasecmp(word, "weeks", 1))
            factor = 60 * 60 * 24 * 7;
        else if (!strncasecmp(word, "days", 1))
            factor = 60 * 60 * 24;
        else if (!strncasecmp(word, "hours", 1))
            factor = 60 * 60;
        else if (!strncasecmp(word, "minutes", 2))
            factor = 60;
        else if (!strncasecmp(word, "seconds", 1))
            factor = 1;
        else
            return "bad expires code, unrecognized type";

        modifier = modifier + factor * num;

        /* next <num> */
        word = ap_getword_conf(parms->pool, &arg);
    }

    cls->expires = modifier;

    return NULL;
}

static const char *set_cookie_name(cmd_parms *cmd, void *mconfig, char *name)
{
    cookie_dir_rec *dcfg = (cookie_dir_rec *) mconfig;

    dcfg->cookie_name = ap_pstrdup(cmd->pool, name);

    set_and_comp_regexp(dcfg, cmd->pool, name);

    if (dcfg->regexp == NULL) {
	return "Regular expression could not be compiled.";
    }
    if (dcfg->regexp->re_nsub + 1 != NUM_SUBS) {
        return ap_pstrcat(cmd->pool, "Invalid cookie name \"",
                           name, "\"", NULL);
    }

    return NULL;
}

/*
 * Set the value for the 'Domain=' attribute.
 */
static const char *set_cookie_domain(cmd_parms *cmd, void *mconfig, char *name)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) mconfig;

    /*
     * Apply the restrictions on cookie domain attributes.
     */
    if (strlen(name) == 0) {
        return "CookieDomain values may not be null";
    }
    if (name[0] != '.') {
        return "CookieDomain values must begin with a dot";
    }
    if (strchr(&name[1], '.') == NULL) {
        return "CookieDomain values must contain at least one embedded dot";
    }

    dcfg->cookie_domain = ap_pstrdup(cmd->pool, name);
    return NULL;
}

/*
 * Make a note of the cookie style we should use.
 */
static const char *set_cookie_style(cmd_parms *cmd, void *mconfig, char *name)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) mconfig;

    if (strcasecmp(name, "Netscape") == 0) {
        dcfg->style = CT_NETSCAPE;
    }
    else if ((strcasecmp(name, "Cookie") == 0)
             || (strcasecmp(name, "RFC2109") == 0)) {
        dcfg->style = CT_COOKIE;
    }
    else if ((strcasecmp(name, "Cookie2") == 0)
             || (strcasecmp(name, "RFC2965") == 0)) {
        dcfg->style = CT_COOKIE2;
    }
    else {
        return ap_psprintf(cmd->pool, "Invalid %s keyword: '%s'",
                           cmd->cmd->name, name);
    }

    return NULL;
}

/*
 * Make a note of the cookie format we should use.
 */
static const char *set_cookie_format(cmd_parms *cmd, void *mconfig, char *name)
{
    cookie_dir_rec *dcfg;

    dcfg = (cookie_dir_rec *) mconfig;

    if (strcasecmp(name, "Normal") == 0) {
        dcfg->format = CF_NORMAL;
    }
    else if (strcasecmp(name, "Compact") == 0) {
        dcfg->format = CF_COMPACT;
    }
    else {
        return ap_psprintf(cmd->pool, "Invalid %s keyword: '%s'",
                           cmd->cmd->name, name);
    }

    return NULL;
}

static const char *set_cookie_prefix(cmd_parms *cmd, void *mconfig, char *name)
{
    cookie_dir_rec *dcfg = (cookie_dir_rec *) mconfig;

    dcfg->prefix_string = ap_pstrdup(cmd->pool, name);

    return NULL;
}


static const command_rec cookie_log_cmds[] = {
    {"CookieExpires", set_cookie_exp, NULL, OR_FILEINFO, TAKE1,
     "an expiry date code"},
    {"CookieTracking", set_cookie_enable, NULL, OR_FILEINFO, FLAG,
     "whether or not to enable cookies"},
    {"CookieName", set_cookie_name, NULL, OR_FILEINFO, TAKE1,
     "name of the tracking cookie"},
    {"CookieDomain", set_cookie_domain, NULL, OR_FILEINFO, TAKE1,
     "domain to which this cookie applies"},
    {"CookieStyle", set_cookie_style, NULL, OR_FILEINFO, TAKE1,
     "'Netscape', 'Cookie' (RFC2109), or 'Cookie2' (RFC2965)"},
    {"CookieFormat", set_cookie_format, NULL, OR_FILEINFO, TAKE1,
     "'Normal' or 'Compact'"},
    {"CookiePrefix", set_cookie_prefix, NULL, OR_FILEINFO, TAKE1,
     "String prepended to cookie"},
    {NULL}
};

module MODULE_VAR_EXPORT usertrack_module = {
    STANDARD_MODULE_STUFF,
    NULL,                       /* initializer */
    make_cookie_dir,            /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    make_cookie_log_state,      /* server config */
    NULL,                       /* merge server configs */
    cookie_log_cmds,            /* command table */
    NULL,                       /* handlers */
    NULL,                       /* filename translation */
    NULL,                       /* check_user_id */
    NULL,                       /* check auth */
    NULL,                       /* check access */
    NULL,                       /* type_checker */
    spot_cookie,                /* fixups */
    NULL,                       /* logger */
    NULL,                       /* header parser */
    NULL,                       /* child_init */
    NULL,                       /* child_exit */
    NULL                        /* post read-request */
};



