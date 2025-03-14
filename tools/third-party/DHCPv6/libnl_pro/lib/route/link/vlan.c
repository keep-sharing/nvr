/*
 * lib/route/link/vlan.c	VLAN Link Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2007 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link_info
 * @defgroup vlan VLAN
 * @brief
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link/info-api.h>
#include <netlink/route/link/vlan.h>

#include <linux/if_vlan.h>

#ifndef VLAN_FLAG_REORDER_HDR//David.milesight
#define VLAN_FLAG_REORDER_HDR 0x01
#endif//end

/** @cond SKIP */
#define VLAN_HAS_ID		(1<<0)
#define VLAN_HAS_FLAGS		(1<<1)
#define VLAN_HAS_INGRESS_QOS	(1<<2)
#define VLAN_HAS_EGRESS_QOS	(1<<3)

struct vlan_info
{
	uint16_t		vi_vlan_id;
	uint32_t		vi_flags;
	uint32_t		vi_flags_mask;
	uint32_t		vi_ingress_qos[VLAN_PRIO_MAX+1];
	uint32_t		vi_negress;
	uint32_t		vi_egress_size;
	struct vlan_map * 	vi_egress_qos;
	uint32_t		vi_mask;
};
/** @endcond */

static struct trans_tbl vlan_flags[] = {
	__ADD(VLAN_FLAG_REORDER_HDR, reorder_hdr)
};

char *rtnl_link_vlan_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, vlan_flags, ARRAY_SIZE(vlan_flags));
}

int rtnl_link_vlan_str2flags(const char *name)
{
	return __str2flags(name, vlan_flags, ARRAY_SIZE(vlan_flags));
}

static struct nla_policy vlan_policy[IFLA_VLAN_MAX+1] = {
	[IFLA_VLAN_ID]		= { .type = NLA_U16 },
	[IFLA_VLAN_FLAGS]	= { .minlen = sizeof(struct ifla_vlan_flags) },
	[IFLA_VLAN_INGRESS_QOS]	= { .type = NLA_NESTED },
	[IFLA_VLAN_EGRESS_QOS]	= { .type = NLA_NESTED },
};

static int vlan_alloc(struct rtnl_link *link)
{
	struct vlan_info *vi;

	if ((vi = calloc(1, sizeof(*vi))) == NULL)
		return nl_errno(ENOMEM);

	link->l_info = vi;

	return 0;
}

static int vlan_parse(struct rtnl_link *link, struct nlattr *data,
		      struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_VLAN_MAX+1];
	struct vlan_info *vi;
	int err;

	NL_DBG(3, "Parsing VLAN link info");

	if ((err = nla_parse_nested(tb, IFLA_VLAN_MAX, data, vlan_policy)) < 0)
		goto errout;

	if ((err = vlan_alloc(link)) < 0)
		goto errout;

	vi = link->l_info;

	if (tb[IFLA_VLAN_ID]) {
		vi->vi_vlan_id = nla_get_u16(tb[IFLA_VLAN_ID]);
		vi->vi_mask |= VLAN_HAS_ID;
	}

	if (tb[IFLA_VLAN_FLAGS]) {
		struct ifla_vlan_flags flags;
		nla_memcpy(&flags, tb[IFLA_VLAN_FLAGS], sizeof(flags));

		vi->vi_flags = flags.flags;
		vi->vi_mask |= VLAN_HAS_FLAGS;
	}

	if (tb[IFLA_VLAN_INGRESS_QOS]) {
		struct ifla_vlan_qos_mapping *map;
		struct nlattr *nla;
		int remaining;

		memset(vi->vi_ingress_qos, 0, sizeof(vi->vi_ingress_qos));

		nla_for_each_nested(nla, tb[IFLA_VLAN_INGRESS_QOS], remaining) {
			if (nla_len(nla) < sizeof(*map))
				return nl_error(EINVAL, "Malformed mapping");

			map = nla_data(nla);
			if (map->from < 0 || map->from > VLAN_PRIO_MAX) {
				return nl_error(EINVAL, "VLAN prio %d out of "
						"range", map->from);
			}

			vi->vi_ingress_qos[map->from] = map->to;
		}

		vi->vi_mask |= VLAN_HAS_INGRESS_QOS;
	}

	if (tb[IFLA_VLAN_EGRESS_QOS]) {
		struct ifla_vlan_qos_mapping *map;
		struct nlattr *nla;
		int remaining, i = 0;

		nla_for_each_nested(nla, tb[IFLA_VLAN_EGRESS_QOS], remaining) {
			if (nla_len(nla) < sizeof(*map))
				return nl_error(EINVAL, "Malformed mapping");
			i++;
		}

		/* align to have a little reserve */
		vi->vi_egress_size = (i + 32) & ~31;
		vi->vi_egress_qos = calloc(vi->vi_egress_size, sizeof(*map));
		if (vi->vi_egress_qos == NULL)
			return nl_errno(ENOMEM);

		i = 0;
		nla_for_each_nested(nla, tb[IFLA_VLAN_EGRESS_QOS], remaining) {
			map = nla_data(nla);
			NL_DBG(4, "Assigning egress qos mapping %d\n", i);
			vi->vi_egress_qos[i].vm_from = map->from;
			vi->vi_egress_qos[i++].vm_to = map->to;
		}

		vi->vi_negress = i;
		vi->vi_mask |= VLAN_HAS_EGRESS_QOS;
	}

	err = 0;
errout:
	return err;
}

static void vlan_free(struct rtnl_link *link)
{
	struct vlan_info *vi = link->l_info;

	if (vi) {
		free(vi->vi_egress_qos);
		vi->vi_egress_qos = NULL;
	}

	free(vi);
	link->l_info = NULL;
}

static int vlan_dump_brief(struct rtnl_link *link, struct nl_dump_params *p,
			   int line)
{
	struct vlan_info *vi = link->l_info;

	dp_dump(p, "vlan-id %d", vi->vi_vlan_id);

	return line;
}

static int vlan_dump_full(struct rtnl_link *link, struct nl_dump_params *p,
			  int line)
{
	struct vlan_info *vi = link->l_info;
	int i, printed;
	char buf[64];

	rtnl_link_vlan_flags2str(vi->vi_flags, buf, sizeof(buf));
	dp_dump_line(p, line++, "    vlan-info id %d <%s>\n",
		vi->vi_vlan_id, buf);

	if (vi->vi_mask & VLAN_HAS_INGRESS_QOS) {
		dp_dump_line(p, line++,
		"      ingress vlan prio -> qos/socket prio mapping:\n");
		for (i = 0, printed = 0; i <= VLAN_PRIO_MAX; i++) {
			if (vi->vi_ingress_qos[i]) {
				if (printed == 0) {
					dp_new_line(p, line);
					dp_dump(p, "      ");
				}
				dp_dump(p, "%x -> %#08x, ",
					i, vi->vi_ingress_qos[i]);
				if (printed++ == 3) {
					dp_dump(p, "\n");
					printed = 0;
				}
			}
		}

		if (printed > 0 && printed != 4)
			dp_dump(p, "\n");
	}

	if (vi->vi_mask & VLAN_HAS_EGRESS_QOS) {
		dp_dump_line(p, line++,
		"      egress qos/socket prio -> vlan prio mapping:\n");
		for (i = 0, printed = 0; i < vi->vi_negress; i++) {
			if (printed == 0) {
				dp_new_line(p, line);
				dp_dump(p, "      ");
			}
			dp_dump(p, "%#08x -> %x, ",
				vi->vi_egress_qos[i].vm_from,
				vi->vi_egress_qos[i].vm_to);
			if (printed++ == 3) {
				dp_dump(p, "\n");
				printed = 0;
			}
		}

		if (printed > 0 && printed != 4)
			dp_dump(p, "\n");
	}

	return line;
}

static int vlan_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct vlan_info *vdst, *vsrc = src->l_info;
	int err;

	dst->l_info = NULL;
	if ((err = rtnl_link_set_info_type(dst, "vlan")) < 0)
		return err;
	vdst = dst->l_info;

	vdst->vi_egress_qos = calloc(vsrc->vi_egress_size,
				     sizeof(struct vlan_map));
	if (!vdst->vi_egress_qos)
		return nl_errno(ENOMEM);

	memcpy(vdst->vi_egress_qos, vsrc->vi_egress_qos,
	       vsrc->vi_egress_size * sizeof(struct vlan_map));

	return 0;
}

static int vlan_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct vlan_info *vi = link->l_info;
	struct nlattr *data;

	if (!(data = nla_nest_start(msg, IFLA_INFO_DATA)))
		return nl_errno(ENOBUFS);

	if (vi->vi_mask & VLAN_HAS_ID)
		NLA_PUT_U16(msg, IFLA_VLAN_ID, vi->vi_vlan_id);

	if (vi->vi_mask & VLAN_HAS_FLAGS) {
		struct ifla_vlan_flags flags = {
			.flags = vi->vi_flags,
			.mask = vi->vi_flags_mask,
		};

		NLA_PUT(msg, IFLA_VLAN_FLAGS, sizeof(flags), &flags);
	}

	if (vi->vi_mask & VLAN_HAS_INGRESS_QOS) {
		struct ifla_vlan_qos_mapping map;
		struct nlattr *qos;
		int i;

		if (!(qos = nla_nest_start(msg, IFLA_VLAN_INGRESS_QOS)))
			goto nla_put_failure;

		for (i = 0; i <= VLAN_PRIO_MAX; i++) {
			if (vi->vi_ingress_qos[i]) {
				map.from = i;
				map.to = vi->vi_ingress_qos[i];

				NLA_PUT(msg, i, sizeof(map), &map);
			}
		}

		nla_nest_end(msg, qos);
	}

	if (vi->vi_mask & VLAN_HAS_EGRESS_QOS) {
		struct ifla_vlan_qos_mapping map;
		struct nlattr *qos;
		int i;

		if (!(qos = nla_nest_start(msg, IFLA_VLAN_EGRESS_QOS)))
			goto nla_put_failure;

		for (i = 0; i < vi->vi_negress; i++) {
			map.from = vi->vi_egress_qos[i].vm_from;
			map.to = vi->vi_egress_qos[i].vm_to;

			NLA_PUT(msg, i, sizeof(map), &map);
		}

		nla_nest_end(msg, qos);
	}

	nla_nest_end(msg, data);

nla_put_failure:

	return 0;
}

static struct rtnl_link_info_ops vlan_info_ops = {
	.io_name		= "vlan",
	.io_alloc		= vlan_alloc,
	.io_parse		= vlan_parse,
	.io_dump[NL_DUMP_BRIEF]	= vlan_dump_brief,
	.io_dump[NL_DUMP_FULL]	= vlan_dump_full,
	.io_clone		= vlan_clone,
	.io_put_attrs		= vlan_put_attrs,
	.io_free		= vlan_free,
};

int rtnl_link_vlan_set_id(struct rtnl_link *link, int id)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	vi->vi_vlan_id = id;
	vi->vi_mask |= VLAN_HAS_ID;

	return 0;
}

int rtnl_link_vlan_get_id(struct rtnl_link *link)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	if (vi->vi_mask & VLAN_HAS_ID)
		return vi->vi_vlan_id;
	else
		return 0;
}

int rtnl_link_vlan_set_flags(struct rtnl_link *link, unsigned int flags)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	vi->vi_flags_mask |= flags;
	vi->vi_flags |= flags;
	vi->vi_mask |= VLAN_HAS_FLAGS;

	return 0;
}

int rtnl_link_vlan_unset_flags(struct rtnl_link *link, unsigned int flags)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	vi->vi_flags_mask |= flags;
	vi->vi_flags &= ~flags;
	vi->vi_mask |= VLAN_HAS_FLAGS;

	return 0;
}

unsigned int rtnl_link_vlan_get_flags(struct rtnl_link *link)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	return vi->vi_flags;
}

int rtnl_link_vlan_set_ingress_map(struct rtnl_link *link, int from,
				   uint32_t to)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	if (from < 0 || from > VLAN_PRIO_MAX)
		return nl_error(EINVAL, "Invalid vlan prio 0..%d",
			VLAN_PRIO_MAX);

	vi->vi_ingress_qos[from] = to;
	vi->vi_mask |= VLAN_HAS_INGRESS_QOS;

	return 0;
}

uint32_t *rtnl_link_vlan_get_ingress_map(struct rtnl_link *link)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops) {
		nl_error(EOPNOTSUPP, "Not a VLAN link");
		return NULL;
	}

	if (vi->vi_mask & VLAN_HAS_INGRESS_QOS)
		return vi->vi_ingress_qos;
	else
		return NULL;
}

int rtnl_link_vlan_set_egress_map(struct rtnl_link *link, uint32_t from, int to)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops)
		return nl_error(EOPNOTSUPP, "Not a VLAN link");

	if (to < 0 || to > VLAN_PRIO_MAX)
		return nl_error(EINVAL, "Invalid vlan prio 0..%d",
			VLAN_PRIO_MAX);

	if (vi->vi_negress >= vi->vi_egress_size) {
		int new_size = vi->vi_egress_size + 32;
		void *ptr;

		ptr = realloc(vi->vi_egress_qos, new_size);
		if (!ptr)
			return nl_errno(ENOMEM);

		vi->vi_egress_qos = ptr;
		vi->vi_egress_size = new_size;
	}

	vi->vi_egress_qos[vi->vi_negress].vm_from = from;
	vi->vi_egress_qos[vi->vi_negress].vm_to = to;
	vi->vi_negress++;
	vi->vi_mask |= VLAN_HAS_EGRESS_QOS;

	return 0;
}

struct vlan_map *rtnl_link_vlan_get_egress_map(struct rtnl_link *link,
					       int *negress)
{
	struct vlan_info *vi = link->l_info;

	if (link->l_info_ops != &vlan_info_ops || !link->l_info_ops) {
		nl_error(EOPNOTSUPP, "Not a VLAN link");
		return NULL;
	}

	if (negress == NULL) {
		nl_error(EINVAL, "Require pointer to store negress");
		return NULL;
	}

	if (vi->vi_mask & VLAN_HAS_EGRESS_QOS) {
		*negress = vi->vi_negress;
		return vi->vi_egress_qos;
	} else {
		*negress = 0;
		return NULL;
	}
}

static void __init vlan_init(void)
{
	rtnl_link_register_info(&vlan_info_ops);
}

static void __exit vlan_exit(void)
{
	rtnl_link_unregister_info(&vlan_info_ops);
}

/** @} */
