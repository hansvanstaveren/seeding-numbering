#define MAXPROPERTIES	3
#define MAXMEMBERS	299
#define MULTIPL		10

#define sqr(n)	((n)*(n))
#undef DEB		/* debug */

typedef struct propertyval pv_t, *pv_p;
struct propertyval {
	pv_p	pv_next;
	char	*pv_string;
	int	pv_npairs;
	int	*pv_gmembers;	/* scaled by MULTIPL */
	int	*pv_ideal;	/* scaled by MULTIPL */
};

typedef struct property pt_t, *pt_p;
struct property {
	pv_p	pt_vals;
};

typedef struct pair pr_t, *pr_p;
struct pair {
	pr_p	pair_next;
	char	*pair_id1;
	char	*pair_id2;
	int	pair_class;
	pv_p	pair_prop[MAXPROPERTIES];
};

typedef struct pairclass pc_t, *pc_p;
struct pairclass {
	pc_p	prc_next;
	int	prc_class;
	pr_p	prc_list;
	int	prc_size;
};

typedef struct group gr_t, *gr_p;
struct group {
	pr_p	gr_pairs[MAXMEMBERS];
	int	gr_size;
	int	gr_unbalance;
};
