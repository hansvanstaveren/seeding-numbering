#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "seeding.h"
#include "subr.h"

static char rcsid[] = "$Header: /home/sater/bridge/numberlines/RCS/seeding.c,v 1.2 2009/02/24 15:10:07 sater Exp sater $";

#define random rand
#define srandom srand

pt_t	property[MAXPROPERTIES];	/* Starting points for propery value lists */

pc_p	pairclasses;		/* Pointer to list of pairclasses */
int 	totalpairs;		/* Numbers of pairs */
int	totalgroups;		/* Number of groups */

gr_p	groups;

#define COMMA		','

#define OPTION_STRING	""
#define USAGE_STRING	""

/*
 * Input pairnames
 */

#undef DEB
#ifdef DEB
FILE *debug;

#define DEBUG(x)	x;

void pair_dump(pr_p prp)
{
	int i;

	while (prp) {
		fprintf(debug, "  %x: %x \"%s\" \"%s\" %d", prp,
			prp->pair_next, prp->pair_id1, prp->pair_id2, prp->pair_class);
		for(i=0; i<MAXPROPERTIES; i++) {
			fprintf(debug, " %x", prp->pair_prop[i]);
			if (prp->pair_prop[i])
				fprintf(debug, "(%s,%d)", prp->pair_prop[i]->pv_string, prp->pair_prop[i]->pv_npairs);
		}
		fprintf(debug, "\n");
		prp = prp->pair_next;
	}
}

void
pairclass_dump()
{
	pc_p	pcp;

	pcp = pairclasses;
	fprintf(debug, "Pairclass dump:\n");
	while (pcp) {
		fprintf(debug, "%x: %x %d %x %d\n", pcp,
			pcp->prc_next, pcp->prc_class, pcp->prc_list, pcp->prc_size);
		pair_dump(pcp->prc_list);
		pcp = pcp->prc_next;
	}
	fprintf(debug, "End of pairclass dump\n\n");
}
#else
#define DEBUG(x)	;
#endif /* DEBUG */

pc_p
pairclass_lookup(int class)
{
	pc_p	*pcpp = &pairclasses;
	pc_p	pcp;

	/*
	 * Keep list sorted on class
	 */
	while (*pcpp!=0 && (*pcpp)->prc_class < class)
		pcpp = &(*pcpp)->prc_next;
	if (*pcpp!=0 && (*pcpp)->prc_class == class)
		return *pcpp;
	/*
	 * Not found, create it
	 */
	pcp = (pc_p) calloc(1, sizeof(pc_t));
	pcp->prc_class = class;
	pcp->prc_list = 0;
	pcp->prc_size = 0;
	pcp->prc_next = *pcpp;
	*pcpp = pcp;
	return pcp;
}

pr_p
pair_lookup(char *id1, char *id2, int class)
{
	pc_p pcp;	/* class structure */
	pr_p prp, *prpp;	/* pair structure */
	int i, skip;

	pcp = pairclass_lookup(class);

	prp = (pr_p) calloc(1, sizeof(pr_t));
	prp->pair_id1 = string_copy(id1);
	prp->pair_id2 = string_copy(id2);
	prp->pair_class = class;
	for (i=0; i<MAXPROPERTIES; i++)
		prp->pair_prop[i] = 0;

	/* Enter pair in list at random */
	prpp = &pcp->prc_list;
	skip = random()%(pcp->prc_size+1);
	while (skip--)
		prpp = &(*prpp)->pair_next;
	prp->pair_next = *prpp;
	*prpp = prp;

	pcp->prc_size++;
	return prp;
}

pv_p
prop_lookup(int propno, char *propname)
{
	pt_p	ptp;
	pv_p	pvp;

	ptp = &property[propno];
	for(pvp = ptp->pt_vals; pvp; pvp=pvp->pv_next) {
		if (strcmp(propname, pvp->pv_string)==0) {
			pvp->pv_npairs++;
			return pvp;
		}
	}
	pvp = (pv_p) calloc(1, sizeof(pv_t));
	pvp->pv_string = string_copy(propname);
	pvp->pv_npairs = 1;
	pvp->pv_gmembers = (int *) calloc(totalgroups, sizeof(int));
	pvp->pv_ideal = (int *) calloc(totalgroups, sizeof(int));
	pvp->pv_next = ptp->pt_vals;
	ptp->pt_vals = pvp;
	return pvp;
}

void
errorline(int lineno, char *s) {

	fprintf(stderr, "Line %d: error in \"%s\"\n", lineno, s);
}

int
input_pairs(FILE *f)
{
	char ibuf[256];
	char *p, *commap, *nlp;
	int class;
	pr_p prp;
	int lineno = 0;
	int propno;
	int errors=0;
	char *id1, *id2;

	while((p = fgets(ibuf, sizeof(ibuf), f)) != NULL) {
		lineno++;
		nlp = strchr(p, '\n');
		/*
		 * format id1,id2,class,properties
		 */
		commap = strchr(p, COMMA);
		if (commap) {
			id1 = p;
			p = commap;
			*p++ = 0;
			commap = strchr(p, COMMA);
			id2 = p;
		}
		if (commap)
			class = atoi(commap+1);
		if (!commap || !nlp || class==0) {
			errorline(lineno, ibuf);
			errors++;
			continue;
		}
		*commap++ = 0;
		*nlp = 0;
		prp = pair_lookup(id1, id2, class);
		p = strchr(commap, COMMA);
		for(propno = 0; propno < MAXPROPERTIES; propno++) {
			if(p == 0) {
				prp->pair_prop[propno] = prop_lookup(propno, "");
				continue;	/* No more props */
			}
			p++;
			commap = strchr(p, COMMA);
			if (commap)
				*commap = 0;
			/* process string p */
			DEBUG(fprintf(debug, "Property %d is %s\n", propno, p));
			prp->pair_prop[propno] = prop_lookup(propno, p);
			p = commap;
		}
		totalpairs++;
			
	}
	return errors==0;
}

pr_t dummypair = {0, 0, 0, 0, {0} };
int *grpsize;
int largestgroup;

int
input_groupsize(FILE *f)
{
	char ibuf[256], *p;
	int i,gs;

	/*
	 * Read first line, containing number of groups
	 */

	p = fgets(ibuf, sizeof(ibuf), f);
	if (p == NULL)
		return 0;
	totalgroups = atoi(p);
	if (totalgroups <= 1) {
		fprintf(stderr, "Number of groups should be >1\n");
		return 0;
	}

	/*
	 * First allocate storage for groups
	 */
	grpsize = (int *) calloc(totalgroups, sizeof(int));
	groups = (gr_p) calloc(totalgroups, sizeof(gr_t));
	for (i=0; i<totalgroups; i++) {
		p = fgets(ibuf, sizeof(ibuf), f);
		if (p == NULL) 
			return 0;
		gs = atoi(p);
		if (gs < 1 || gs > MAXMEMBERS) {
			fprintf(stderr, "Groupsize must be between 1 and %d\n", MAXMEMBERS);
			return 0;
		}
		grpsize[i] = gs;
		if (gs > largestgroup)
			largestgroup = gs;
	}
	return 1;
}

int
init_groups()
{
	int i,j,g,size;
	int npair=0;
	gr_p grp;
	pt_p ptp;
	pv_p pvp;

	if (largestgroup%2==0)
		largestgroup++;
	for(g=0; g<totalgroups; g++) {
		size = grpsize[g];
		npair += size;
		grp = &groups[g];
		grp->gr_size = size;
#ifdef GROUPS_IN_MIDDLE
		/*
		 * Place dummy pair in middle group positions
		 */
		p = (MAXMEMBERS-size)/2; 
		for (j=p; j<p+size; j++) {
			grp->gr_pairs[j] = &dummypair;
			DEBUG(fprintf(debug, "Fill %d[%d]\n", g, j))
		}
#else /* GROUPS_IN_MIDDLE */
		/*
		 * Use positions like in Neuberg for smaller groups
		 */
		{
		double neuberg;

		DEBUG(fprintf(debug, "lg=%d, size=%d\n", largestgroup, size))
		for (i=0; i<size; i++) {
			DEBUG(fprintf(debug, "i=%d\n", i))
			/* First calculate desired position as float */
			neuberg = ((double) largestgroup / size *
					(i+0.5)) - 0.5;
			DEBUG(fprintf(debug, "nb=%g\n", neuberg))
			/* Now prepare to round away from the middle */
			if (neuberg < (largestgroup-1.0)*0.5)
				neuberg -= 0.0001;
			else
				neuberg += 0.0001;
			neuberg += 0.5;
			DEBUG(fprintf(debug, "nbr=%g\n", neuberg))
			j = neuberg;
			grp->gr_pairs[j] = &dummypair;
			DEBUG(fprintf(debug, "Fill %d[%d]\n", g, j))
		}
		}

#endif /* GROUPS_IN_MIDDLE */

		for(ptp=property;ptp<property+MAXPROPERTIES;ptp++) {
			for(pvp=ptp->pt_vals; pvp; pvp=pvp->pv_next) {
				pvp->pv_gmembers[g] = 0;
				pvp->pv_ideal[g] = MULTIPL * pvp->pv_npairs * size / totalpairs;
				grp->gr_unbalance += sqr(pvp->pv_ideal[g]);
				DEBUG(fprintf(debug, "Group %d, size %d, prop %s, ideal %d, unbalance %d\n",
	g, grp->gr_size, pvp->pv_string, pvp->pv_ideal[g], grp->gr_unbalance));
			}
		}
	}
	if (npair != totalpairs) {
		fprintf(stderr, "Groups should have %d pairs, there are %d pairs\n", npair, totalpairs);
		exit(-1);
	}
	return 0;
}

pr_p
best_pair(int g, pc_p pcp)
{
	pr_p prp, bestpair, *prpp;
	pv_p pvp;
	int unbaldiff, bestunbaldiff;
	int prop;
	int gm, id;

	bestunbaldiff = 100000000;	/* INFINITY */
	for (prp=pcp->prc_list; prp; prp=prp->pair_next) {
		unbaldiff = 0;
		for(prop=0; prop<MAXPROPERTIES; prop++) {
			pvp = prp->pair_prop[prop];
#ifdef notdef
			if (pvp == 0)
				continue;
#endif
			gm = pvp->pv_gmembers[g];
			id = pvp->pv_ideal[g];
			unbaldiff += (sqr(gm+MULTIPL-id)-sqr(gm-id));
			DEBUG(fprintf(debug, "prop%d, gm=%d, id=%d, unbaldiff=%d\n", prop, gm, id, unbaldiff));
		}
		DEBUG(fprintf(debug, "Pair %s %s, unb=%d, best=%d\n",
			prp->pair_id1, prp->pair_id2, unbaldiff, bestunbaldiff));
		if (unbaldiff < bestunbaldiff) {
			bestunbaldiff = unbaldiff;
			bestpair = prp;
		}
	}

	/*
	 * Unhook bestpair from list
	 */
	prpp = &pcp->prc_list;
	while ((*prpp) != bestpair)
		prpp = &(*prpp)->pair_next;
	*prpp = bestpair->pair_next;
	bestpair->pair_next = 0;
	pcp->prc_size--;

	/*
	 * Update group counters and unbalance
	 */
	
	for (prop=0; prop<MAXPROPERTIES; prop++) {
		pvp = bestpair->pair_prop[prop];
#ifdef notdef
		if (pvp == 0)
			continue;
#endif
		pvp->pv_gmembers[g] += MULTIPL;
	}
	groups[g].gr_unbalance += bestunbaldiff;
	return bestpair;
}

void
seed_pairs()
{
	int g,p;
	int direction;
	int pairshandled;
	pc_p pcp;
	
	g = 0;		/* Start the seesaw at group 0 */
	p = 0;		/* Start the seesaw at pair 0 */
	direction = 1;	/* Start going up the groups */
	pcp = pairclasses;	/* Start with the strongest pairs */
	pairshandled = 0;
	while (pairshandled < totalpairs) {
		DEBUG(fprintf(debug, "g=%d, p=%d, dir=%d, ph=%d\n", g,p,direction,pairshandled));
		if (p >= MAXMEMBERS) {
			fprintf(stderr, "internal seed_pairs error\n");
			abort();
		}
		if (groups[g].gr_pairs[p]) { /* filled with dummy */
			groups[g].gr_pairs[p] = best_pair(g, pcp);
			pairshandled++;
			if (pcp->prc_size == 0)
				pcp = pcp->prc_next;
		}
		if (direction == 1) {
			g++;
			if (g == totalgroups) {
				g = totalgroups-1;
				p++;
				direction = -1;
			}
		} else {
			g--;
			if (g < 0) {
				g = 0;
				p++;
				direction = 1;
			}
		}
	}
}

output_groups()
{
	int g, p;
	gr_p grp;
	pr_p prp;
	int prop;

	for (g=0; g<totalgroups; g++) {
		grp = groups+g;
		if (grp->gr_size == 0)
			continue;
		DEBUG(fprintf(stderr, "Group %d, size %d, unbalance %d\n", g, grp->gr_size, grp->gr_unbalance));
		for (p=0; p<MAXMEMBERS; p++) {
			prp = grp->gr_pairs[p];
			if (prp == 0)
				continue;
			printf("%s,%s,%d", prp->pair_id1, prp->pair_id2, prp->pair_class);
			for (prop=0; prop < MAXPROPERTIES; prop++)
				printf(",%s", prp->pair_prop[prop]->pv_string);
			printf("\n");
		}
	}
}

int
main (int argc, char *argv[])
{
	int c;

#ifdef DEB
	debug = fopen("debug", "w");
	setbuf(debug, NULL);
#endif
	while ((c = getopt(argc, argv, OPTION_STRING)) != -1) {
		switch(c) {
		case '?':
			fprintf(stderr, "Usage: %s %s\n", argv[0], USAGE_STRING);
			exit(-1);
		}
	}
	srandom(getpid());

	if (!input_groupsize(stdin))
		return -1;
	if (!input_pairs(stdin))
		return -1;
	DEBUG(pairclass_dump())
	init_groups();
	DEBUG(fprintf(debug, "calling seed_pairs\n"));
	seed_pairs();
	output_groups();
}
