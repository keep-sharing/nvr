/* ported from KAME: common.h,v 1.29 2002/06/11 08:24:34 jinmei Exp */

/*
 * Copyright (C) 1998 and 1999 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define IN6_IFF_INVALID -1

/* ANSI __func__ can not be concatantated (C99 std) */
#if defined (HAVE_GCC_FUNCTION)
#define FNAME __FUNCTION__ ":"
#else
#define FNAME ""
#endif

extern int foreground;
extern int debug_thresh;

/* common.c */
extern int dhcp6_copy_list(struct dhcp6_list *, const struct dhcp6_list *);
extern void dhcp6_clear_list(struct dhcp6_list *);
extern int dhcp6_count_list(struct dhcp6_list *);
extern struct dhcp6_listval *dhcp6_find_listval(struct dhcp6_list *, void *,
                                                dhcp6_listval_type_t);
extern struct dhcp6_listval *dhcp6_add_listval(struct dhcp6_list *, void *,
                                               dhcp6_listval_type_t);
extern struct ia_listval *ia_create_listval();
extern void ia_clear_list(struct ia_list *);
extern int ia_copy_list(struct ia_list *, struct ia_list *);
extern struct ia_listval *ia_find_listval(struct ia_list *,
                                          iatype_t, u_int32_t);
extern struct dhcp6_event *dhcp6_create_event(struct dhcp6_if *, int);
extern void dhcp6_remove_event(struct dhcp6_event *);
extern int dhcp6_has_option(struct dhcp6_list * optlist, int option);
extern int getifaddr(struct in6_addr *, char *, struct in6_addr *,
                     int, int, int);
extern int transmit_sa(int, struct sockaddr_in6 *, char *, size_t);
extern long random_between(long, long);
extern int prefix6_mask(struct in6_addr *, int);
extern int sa6_plen2mask(struct sockaddr_in6 *, int);
extern char *addr2str(struct sockaddr *, socklen_t);
extern char *in6addr2str(struct in6_addr *, int);
extern const char *getdev(struct sockaddr_in6 *);
extern int in6_addrscopebyif(struct in6_addr *, char *);
extern int in6_scope(struct in6_addr *);
extern void setloglevel(int);
extern void dhcpv6_dprintf(int, const char *, ...);
extern int duid_match_llt(struct duid *, struct duid *);
extern int get_duid(const char *, const char *, struct duid *);
extern int save_duid(const char *, const char *, struct duid *);
extern u_int16_t calculate_duid_len(const char *, u_int16_t *);
extern void dhcp6_init_options(struct dhcp6_optinfo *);
extern void dhcp6_clear_options(struct dhcp6_optinfo *);
extern int dhcp6_copy_options(struct dhcp6_optinfo *, struct dhcp6_optinfo *);
extern int dhcp6_get_options(struct dhcp6opt *, struct dhcp6opt *,
                             struct dhcp6_optinfo *);
extern int dhcp6_set_options(struct dhcp6opt *, struct dhcp6opt *,
                             struct dhcp6_optinfo *);
extern void dhcp6_set_timeoparam(struct dhcp6_event *);
extern void dhcp6_reset_timer(struct dhcp6_event *);
extern char *dhcp6optstr(int);
extern char *dhcp6msgstr(int);
extern char *dhcp6_stcodestr(int);
extern char *duidstr(const struct duid *);
extern int duidcpy(struct duid *, const struct duid *);
extern int duidcmp(const struct duid *, const struct duid *);
extern void duidfree(struct duid *);
extern void relayfree(struct relay_list *);
extern void ifinit(const char *);
extern int configure_duid(const char *, struct duid *);
extern struct dhcp6_if *find_ifconfbyname(const char *);
extern struct dhcp6_if *find_ifconfbyid(unsigned int);
extern struct prefix_ifconf *find_prefixifconf(const char *);
extern struct host_conf *find_hostconf(const struct duid *);
