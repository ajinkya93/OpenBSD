/*	$OpenBSD: tables.c,v 1.10 2005/07/16 14:09:51 krw Exp $	*/

/* Tables of information... */

/*
 * Copyright (c) 1995, 1996 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include "dhcpd.h"

/*
 * DHCP Option names, formats and codes, from RFC1533.
 *
 * Format codes:
 *
 * e - end of data
 * I - IP address
 * l - 32-bit signed integer
 * L - 32-bit unsigned integer
 * s - 16-bit signed integer
 * S - 16-bit unsigned integer
 * b - 8-bit signed integer
 * B - 8-bit unsigned integer
 * t - ASCII text
 * f - flag (true or false)
 * A - array of whatever precedes (e.g., IA means array of IP addresses)
 */

const struct option dhcp_options[256] = {
	/*   0 */ { "pad", "" },
	/*   1 */ { "subnet-mask", "I" },
	/*   2 */ { "time-offset", "l" },
	/*   3 */ { "routers", "IA" },
	/*   4 */ { "time-servers", "IA" },
	/*   5 */ { "ien116-name-servers", "IA" },
	/*   6 */ { "domain-name-servers", "IA" },
	/*   7 */ { "log-servers", "IA" },
	/*   8 */ { "cookie-servers", "IA" },
	/*   9 */ { "lpr-servers", "IA" },
	/*  10 */ { "impress-servers", "IA" },
	/*  11 */ { "resource-location-servers", "IA" },
	/*  12 */ { "host-name", "X" },
	/*  13 */ { "boot-size", "S" },
	/*  14 */ { "merit-dump", "t" },
	/*  15 */ { "domain-name", "t" },
	/*  16 */ { "swap-server", "I" },
	/*  17 */ { "root-path", "t" },
	/*  18 */ { "extensions-path", "t" },
	/*  19 */ { "ip-forwarding", "f" },
	/*  20 */ { "non-local-source-routing", "f" },
	/*  21 */ { "policy-filter", "IIA" },
	/*  22 */ { "max-dgram-reassembly", "S" },
	/*  23 */ { "default-ip-ttl", "B" },
	/*  24 */ { "path-mtu-aging-timeout", "L" },
	/*  25 */ { "path-mtu-plateau-table", "SA" },
	/*  26 */ { "interface-mtu", "S" },
	/*  27 */ { "all-subnets-local", "f" },
	/*  28 */ { "broadcast-address", "I" },
	/*  29 */ { "perform-mask-discovery", "f" },
	/*  30 */ { "mask-supplier", "f" },
	/*  31 */ { "router-discovery", "f" },
	/*  32 */ { "router-solicitation-address", "I" },
	/*  33 */ { "static-routes", "IIA" },
	/*  34 */ { "trailer-encapsulation", "f" },
	/*  35 */ { "arp-cache-timeout", "L" },
	/*  36 */ { "ieee802-3-encapsulation", "f" },
	/*  37 */ { "default-tcp-ttl", "B" },
	/*  38 */ { "tcp-keepalive-interval", "L" },
	/*  39 */ { "tcp-keepalive-garbage", "f" },
	/*  40 */ { "nis-domain", "t" },
	/*  41 */ { "nis-servers", "IA" },
	/*  42 */ { "ntp-servers", "IA" },
	/*  43 */ { "vendor-encapsulated-options", "X" },
	/*  44 */ { "netbios-name-servers", "IA" },
	/*  45 */ { "netbios-dd-server", "IA" },
	/*  46 */ { "netbios-node-type", "B" },
	/*  47 */ { "netbios-scope", "t" },
	/*  48 */ { "font-servers", "IA" },
	/*  49 */ { "x-display-manager", "IA" },
	/*  50 */ { "dhcp-requested-address", "I" },
	/*  51 */ { "dhcp-lease-time", "L" },
	/*  52 */ { "dhcp-option-overload", "B" },
	/*  53 */ { "dhcp-message-type", "B" },
	/*  54 */ { "dhcp-server-identifier", "I" },
	/*  55 */ { "dhcp-parameter-request-list", "BA" },
	/*  56 */ { "dhcp-message", "t" },
	/*  57 */ { "dhcp-max-message-size", "S" },
	/*  58 */ { "dhcp-renewal-time", "L" },
	/*  59 */ { "dhcp-rebinding-time", "L" },
	/*  60 */ { "dhcp-class-identifier", "t" },
	/*  61 */ { "dhcp-client-identifier", "X" },
	/*  62 */ { "option-62", "X" },
	/*  63 */ { "option-63", "X" },
	/*  64 */ { "nisplus-domain", "t" },
	/*  65 */ { "nisplus-servers", "IA" },
	/*  66 */ { "tftp-server-name", "t" },
	/*  67 */ { "bootfile-name", "t" },
	/*  68 */ { "mobile-ip-home-agent", "IA" },
	/*  69 */ { "smtp-server", "IA" },
	/*  70 */ { "pop-server", "IA" },
	/*  71 */ { "nntp-server", "IA" },
	/*  72 */ { "www-server", "IA" },
	/*  73 */ { "finger-server", "IA" },
	/*  74 */ { "irc-server", "IA" },
	/*  75 */ { "streettalk-server", "IA" },
	/*  76 */ { "streettalk-directory-assistance-server", "IA" },
	/*  77 */ { "user-class", "t" },
	/*  78 */ { "option-78", "X" },
	/*  79 */ { "option-79", "X" },
	/*  80 */ { "option-80", "X" },
	/*  81 */ { "option-81", "X" },
	/*  82 */ { "option-82", "X" },
	/*  83 */ { "option-83", "X" },
	/*  84 */ { "option-84", "X" },
	/*  85 */ { "nds-servers", "IA" },
	/*  86 */ { "nds-tree-name", "X" },
	/*  87 */ { "nds-context", "X" },
	/*  88 */ { "option-88", "X" },
	/*  89 */ { "option-89", "X" },
	/*  90 */ { "option-90", "X" },
	/*  91 */ { "option-91", "X" },
	/*  92 */ { "option-92", "X" },
	/*  93 */ { "option-93", "X" },
	/*  94 */ { "option-94", "X" },
	/*  95 */ { "option-95", "X" },
	/*  96 */ { "option-96", "X" },
	/*  97 */ { "option-97", "X" },
	/*  98 */ { "option-98", "X" },
	/*  99 */ { "option-99", "X" },
	/* 100 */ { "option-100", "X" },
	/* 101 */ { "option-101", "X" },
	/* 102 */ { "option-102", "X" },
	/* 103 */ { "option-103", "X" },
	/* 104 */ { "option-104", "X" },
	/* 105 */ { "option-105", "X" },
	/* 106 */ { "option-106", "X" },
	/* 107 */ { "option-107", "X" },
	/* 108 */ { "option-108", "X" },
	/* 109 */ { "option-109", "X" },
	/* 110 */ { "option-110", "X" },
	/* 111 */ { "option-111", "X" },
	/* 112 */ { "option-112", "X" },
	/* 113 */ { "option-113", "X" },
	/* 114 */ { "option-114", "X" },
	/* 115 */ { "option-115", "X" },
	/* 116 */ { "option-116", "X" },
	/* 117 */ { "option-117", "X" },
	/* 118 */ { "option-118", "X" },
	/* 119 */ { "option-119", "X" },
	/* 120 */ { "option-120", "X" },
	/* 121 */ { "option-121", "X" },
	/* 122 */ { "option-122", "X" },
	/* 123 */ { "option-123", "X" },
	/* 124 */ { "option-124", "X" },
	/* 125 */ { "option-125", "X" },
	/* 126 */ { "option-126", "X" },
	/* 127 */ { "option-127", "X" },
	/* 128 */ { "option-128", "X" },
	/* 129 */ { "option-129", "X" },
	/* 130 */ { "option-130", "X" },
	/* 131 */ { "option-131", "X" },
	/* 132 */ { "option-132", "X" },
	/* 133 */ { "option-133", "X" },
	/* 134 */ { "option-134", "X" },
	/* 135 */ { "option-135", "X" },
	/* 136 */ { "option-136", "X" },
	/* 137 */ { "option-137", "X" },
	/* 138 */ { "option-138", "X" },
	/* 139 */ { "option-139", "X" },
	/* 140 */ { "option-140", "X" },
	/* 141 */ { "option-141", "X" },
	/* 142 */ { "option-142", "X" },
	/* 143 */ { "option-143", "X" },
	/* 144 */ { "option-144", "X" },
	/* 145 */ { "option-145", "X" },
	/* 146 */ { "option-146", "X" },
	/* 147 */ { "option-147", "X" },
	/* 148 */ { "option-148", "X" },
	/* 149 */ { "option-149", "X" },
	/* 150 */ { "option-150", "X" },
	/* 151 */ { "option-151", "X" },
	/* 152 */ { "option-152", "X" },
	/* 153 */ { "option-153", "X" },
	/* 154 */ { "option-154", "X" },
	/* 155 */ { "option-155", "X" },
	/* 156 */ { "option-156", "X" },
	/* 157 */ { "option-157", "X" },
	/* 158 */ { "option-158", "X" },
	/* 159 */ { "option-159", "X" },
	/* 160 */ { "option-160", "X" },
	/* 161 */ { "option-161", "X" },
	/* 162 */ { "option-162", "X" },
	/* 163 */ { "option-163", "X" },
	/* 164 */ { "option-164", "X" },
	/* 165 */ { "option-165", "X" },
	/* 166 */ { "option-166", "X" },
	/* 167 */ { "option-167", "X" },
	/* 168 */ { "option-168", "X" },
	/* 169 */ { "option-169", "X" },
	/* 170 */ { "option-170", "X" },
	/* 171 */ { "option-171", "X" },
	/* 172 */ { "option-172", "X" },
	/* 173 */ { "option-173", "X" },
	/* 174 */ { "option-174", "X" },
	/* 175 */ { "option-175", "X" },
	/* 176 */ { "option-176", "X" },
	/* 177 */ { "option-177", "X" },
	/* 178 */ { "option-178", "X" },
	/* 179 */ { "option-179", "X" },
	/* 180 */ { "option-180", "X" },
	/* 181 */ { "option-181", "X" },
	/* 182 */ { "option-182", "X" },
	/* 183 */ { "option-183", "X" },
	/* 184 */ { "option-184", "X" },
	/* 185 */ { "option-185", "X" },
	/* 186 */ { "option-186", "X" },
	/* 187 */ { "option-187", "X" },
	/* 188 */ { "option-188", "X" },
	/* 189 */ { "option-189", "X" },
	/* 190 */ { "option-190", "X" },
	/* 191 */ { "option-191", "X" },
	/* 192 */ { "option-192", "X" },
	/* 193 */ { "option-193", "X" },
	/* 194 */ { "option-194", "X" },
	/* 195 */ { "option-195", "X" },
	/* 196 */ { "option-196", "X" },
	/* 197 */ { "option-197", "X" },
	/* 198 */ { "option-198", "X" },
	/* 199 */ { "option-199", "X" },
	/* 200 */ { "option-200", "X" },
	/* 201 */ { "option-201", "X" },
	/* 202 */ { "option-202", "X" },
	/* 203 */ { "option-203", "X" },
	/* 204 */ { "option-204", "X" },
	/* 205 */ { "option-205", "X" },
	/* 206 */ { "option-206", "X" },
	/* 207 */ { "option-207", "X" },
	/* 208 */ { "option-208", "X" },
	/* 209 */ { "option-209", "X" },
	/* 210 */ { "option-210", "X" },
	/* 211 */ { "option-211", "X" },
	/* 212 */ { "option-212", "X" },
	/* 213 */ { "option-213", "X" },
	/* 214 */ { "option-214", "X" },
	/* 215 */ { "option-215", "X" },
	/* 216 */ { "option-216", "X" },
	/* 217 */ { "option-217", "X" },
	/* 218 */ { "option-218", "X" },
	/* 219 */ { "option-219", "X" },
	/* 220 */ { "option-220", "X" },
	/* 221 */ { "option-221", "X" },
	/* 222 */ { "option-222", "X" },
	/* 223 */ { "option-223", "X" },
	/* 224 */ { "option-224", "X" },
	/* 225 */ { "option-225", "X" },
	/* 226 */ { "option-226", "X" },
	/* 227 */ { "option-227", "X" },
	/* 228 */ { "option-228", "X" },
	/* 229 */ { "option-229", "X" },
	/* 230 */ { "option-230", "X" },
	/* 231 */ { "option-231", "X" },
	/* 232 */ { "option-232", "X" },
	/* 233 */ { "option-233", "X" },
	/* 234 */ { "option-234", "X" },
	/* 235 */ { "option-235", "X" },
	/* 236 */ { "option-236", "X" },
	/* 237 */ { "option-237", "X" },
	/* 238 */ { "option-238", "X" },
	/* 239 */ { "option-239", "X" },
	/* 240 */ { "option-240", "X" },
	/* 241 */ { "option-241", "X" },
	/* 242 */ { "option-242", "X" },
	/* 243 */ { "option-243", "X" },
	/* 244 */ { "option-244", "X" },
	/* 245 */ { "option-245", "X" },
	/* 246 */ { "option-246", "X" },
	/* 247 */ { "option-247", "X" },
	/* 248 */ { "option-248", "X" },
	/* 249 */ { "option-249", "X" },
	/* 250 */ { "option-250", "X" },
	/* 251 */ { "option-251", "X" },
	/* 252 */ { "option-252", "X" },
	/* 253 */ { "option-253", "X" },
	/* 254 */ { "option-254", "X" },
	/* 255 */ { "option-end", "e" },
};
