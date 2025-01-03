#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#ifdef notdef
#include <unistd.h>
#endif
#include <string.h>
#include "seeding.h"
#include "subr.h"

#define random rand
#define srandom srand

#define MAXGROUPS 100

pv_p	property_list;		/* Starting points for property value list */

pc_p	pairclasses;		/* Pointer to list of pairclasses */
int 	totalpairs;		/* Numbers of pairs */
int	totalgroups;		/* Number of groups */

int	seq_strength = 0;	/* Output pairs in strength order per group */
int	wheelchairmode = 0;	/* id2 is group to be placed in */
int	trad_output = 0;	/* Output pairs in one group-after-group */

gr_t	groups[MAXGROUPS];
int	pairsingroups;		/* Sum of groupsizes */

#define COMMA		','

#define OPTION_STRING	"swt"
#define USAGE_STRING	"[-s] [-w] [-t] [<number>x<grpsize>] ..."

/*
 * Input pairnames
 */

#undef DEB
#ifdef DEB
FILE *debug;

#define DEBUG(x)	x

void pair_dump(pr_p prp)
{

    while (prp) {
	fprintf(debug, "  %x: %x \"%s\" \"%s\" %d", prp,
	prp->pair_next, prp->pair_id1, prp->pair_id2, prp->pair_class);
	fprintf(debug, " %x", prp->pair_property);
	if (prp->pair_property)
	    fprintf(debug, "(%s,%d)\n",
		prp->pair_property->pv_string,
		prp->pair_property->pv_npairs);
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
	    pcp->prc_next, pcp->prc_class, pcp->prc_list, pcp->prc_listsize);
	pair_dump(pcp->prc_list);
	pcp = pcp->prc_next;
    }
    fprintf(debug, "End of pairclass dump\n\n");
}
#else
#define DEBUG(x)
#endif /* DEBUG */

static char digits[] = "0123456789";

void
decode_xstring(char *str, int *number, int *size) {
    int numlen1, numlen2, endstr;

    numlen1 = strspn(str, digits);
    endstr = numlen1;
    numlen2 = 0;
    if (str[endstr] == 'x') {
	numlen2 = strspn(str+endstr+1, digits);
	endstr = numlen1+1+numlen2;
    }
    if (str[endstr] != 0) {
	fprintf(stderr, "String '%s' not valid format\n", str);
	exit(-1);
    }
    if (numlen2) {
	*number = atoi(str);
	*size = atoi(str+numlen1+1);
    } else {
	*number = 1;
	*size = atoi(str);
    }
}

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
    pcp->prc_listsize = 0;
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
    int fg;
    gr_p grp;

    prp = (pr_p) calloc(1, sizeof(pr_t));
    prp->pair_id1 = string_copy(id1);
    prp->pair_id2 = string_copy(id2);
    fg = atoi(id2) - 1;
    prp->pair_class = class;

    if (!wheelchairmode || fg<0 || fg >=totalgroups) {
	/* Enter pair in list at random */
	pcp = pairclass_lookup(class);
	prpp = &pcp->prc_list;
	skip = random()%(pcp->prc_listsize+1);
	while (skip--)
	    prpp = &(*prpp)->pair_next;
	prp->pair_next = *prpp;
	*prpp = prp;

	pcp->prc_listsize++;
    } else {
    	/* prepositioned pair */
	grp = groups+fg;
	DEBUG(fprintf(stderr, "prepos fg=%d grp=%x, pps=%d, sz=%d\n", fg, grp, grp->gr_ppsize, grp->gr_size));
	if (grp->gr_ppsize >= grp->gr_size)
	    abort();
	prpp = &grp->gr_prepos;
	while (*prpp && (*prpp)->pair_class<class)
	    prpp = &(*prpp)->pair_next;
	prp->pair_next = *prpp;
	*prpp = prp;

	grp->gr_ppsize++;
    }
    return prp;
}

pv_p
prop_lookup(char *propname)
{
    pv_p	pvp;

    for(pvp = property_list; pvp; pvp=pvp->pv_next) {
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
    pvp->pv_next = property_list;
    property_list = pvp;
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
    int errors=0;
    char *id1,*id2;

    while((p = fgets(ibuf, sizeof(ibuf), f)) != NULL) {
	lineno++;
	nlp = strchr(p, '\n');
	/*
	 * format id,fg,class,property
	 */
	commap = strchr(p, COMMA);
	if (commap) {
	    /* comma after id1 */
	    id1 = p;
	    *commap++ = 0;	/* NULL to comma */
	    p = commap;
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
	DEBUG(fprintf(stderr, "%s %s %d found\n", id1, id2, class));
	prp = pair_lookup(id1, id2, class);
	DEBUG(fprintf(stderr, "%s %s %d %x found\n", id1, id2, class, prp));
	p = strchr(commap, COMMA);
	if(p == 0) {
	    prp->pair_property = prop_lookup("");
	    continue;	/* No more props */
	}
	p++;
	commap = strchr(p, COMMA);
	if (commap)
	    *commap = 0;
	/* process string p */
	DEBUG(fprintf(debug, "Property is %s\n", p));
	prp->pair_property = prop_lookup(p);
	prp->pair_random = random();
	DEBUG(fprintf(stderr, "pair id1=%s, rand=%d\n", id1, prp->pair_random));
	totalpairs++;
    }
    return errors==0;
}

pr_t dummypair = {0, 0, 0, 0, 0, 0 };
int grpsize[MAXGROUPS];
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
     */
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
	DEBUG(fprintf(stderr, "Group %d, size %d, largest %d\n", i, gs, largestgroup));
    }
    return 1;
}

void
input_grouparg(char *str) {
    int number, gs;
    int i;

    decode_xstring(str, &number, &gs);
    DEBUG(fprintf(stderr, "decode_xstring %d %d\n", number, gs));
    if (gs < 1 || gs > MAXMEMBERS) {
	fprintf(stderr, "Groupsize must be between 1 and %d\n", MAXMEMBERS);
	return;
    }
    for (i=0; i<number; i++) {
	grpsize[totalgroups++] = gs;
    }
    if (gs > largestgroup)
	largestgroup = gs;
}

int
init_groups()
{
    int i,j,g,size;
    int npair=0;
    gr_p grp;
    pv_p pvp;
    double neuberg;

    if (largestgroup%2==0)
	largestgroup++;
    for(g=0; g<totalgroups; g++) {
	size = grpsize[g];
	npair += size;
	grp = &groups[g];
	grp->gr_size = grp->gr_holestofill = size;
	/*
	 * Use positions like in Neuberg for smaller groups
	 */
	DEBUG(fprintf(debug, "lg=%d, size=%d\n", largestgroup, size));
	for (i=0; i<size; i++) {
	    DEBUG(fprintf(debug, "i=%d\n", i));
	    /* First calculate desired position as float */
	    neuberg = ((double) largestgroup / size * (i+0.5)) - 0.5;
	    DEBUG(fprintf(debug, "nb=%g\n", neuberg));
	    /* Now prepare to round away from the middle */
	    if (neuberg < (largestgroup-1.0)*0.5)
		neuberg -= 0.0001;
	    else
		neuberg += 0.0001;
	    neuberg += 0.5;
	    DEBUG(fprintf(debug, "nbr=%g\n", neuberg));
	    j = neuberg;
	    /*
	     * Insert dummypair:
	     * Will be replaced by real pair later
	     */
	    grp->gr_pairs[j] = &dummypair;
	    DEBUG(fprintf(debug, "Fill %d[%d]\n", g, j));
	}

	for(pvp=property_list; pvp; pvp=pvp->pv_next) {
	    pvp->pv_gmembers[g] = 0;
	    pvp->pv_ideal[g] = MULTIPL * pvp->pv_npairs * size / totalpairs;
	    grp->gr_unbalance += sqr(pvp->pv_ideal[g]);
	    DEBUG(fprintf(debug, "Group %d, size %d, prop %s, ideal %d, unbalance %d\n",
		g, grp->gr_size, pvp->pv_string, pvp->pv_ideal[g], grp->gr_unbalance));
	}
    }
    return npair;
}

pr_p
best_pair(int g, pc_p pcp)
{
    pr_p prp, bestpair, *prpp;
    pv_p pvp;
    int unbaldiff, bestunbaldiff;
    int gm, id;

    bestunbaldiff = 100000000;	/* INFINITY */
    for (prp=pcp->prc_list; prp; prp=prp->pair_next) {
	unbaldiff = 0;
	pvp = prp->pair_property;
	gm = pvp->pv_gmembers[g];
	id = pvp->pv_ideal[g];
	unbaldiff += (sqr(gm+MULTIPL-id)-sqr(gm-id));
	DEBUG(fprintf(debug, "gm=%d, id=%d, unbaldiff=%d\n", gm, id, unbaldiff));
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
    pcp->prc_listsize--;

    /*
     * Update group counters and unbalance
     */
    
    pvp = bestpair->pair_property;
    pvp->pv_gmembers[g] += MULTIPL;
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
    gr_p grp;
	
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
	grp = groups+g;
	if (grp->gr_pairs[p]) {
	    /*
	     * This is a place where a pair should be placed.
	     * First try a prepositioned pair of this class or stronger
	     * otherwise find a good pair based on class and property
	     */
	    if (grp->gr_prepos && (grp->gr_holestofill <= grp->gr_ppsize || grp->gr_prepos->pair_class <= pcp->prc_class)) {
		/* Take this pair */
		grp->gr_pairs[p] = grp->gr_prepos;
		grp->gr_prepos = grp->gr_prepos->pair_next;
		grp->gr_ppsize--;
		pairshandled++;
	    } else {
		grp->gr_pairs[p] = best_pair(g, pcp);
		pairshandled++;
		/*
		 * Take care that this might be the last pair in its class to be handled
		 */
		if (pcp->prc_listsize == 0)
		    pcp = pcp->prc_next;
	    }
	    grp->gr_holestofill--;
	}
	/*
	 * To next place, check direction of seesaw and handle boundaries
	 */
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

int pair_compar(const void *vp1, const void *vp2) {
	pr_p p1, p2;

	p1 = * ((pr_p *) vp1);
	p2 = * ((pr_p *) vp2);

	/*
	 * one is a hole, or real compare
	 * holes at the end
	 */

	if (p1 == 0)
	    return (p2==0 ? 0 : 1);
	if (p2 == 0)
	    return -1;
	DEBUG(fprintf(stderr, "compar %d %d\n", p1->pair_random, p2->pair_random));
	return (p1->pair_random - p2->pair_random);
}

void
output_groups()
{
    int g, p;
    gr_p grp;
    pr_p prp;
    FILE *outf;
    char fname[200];
    int numberlen;
    int unbal_sum_square;

    sprintf(fname, "%d", totalgroups);
    numberlen = strlen(fname);

    unbal_sum_square = 0;
    for (g=0; g<totalgroups; g++) {
	grp = groups+g;
	fprintf(stderr, "Group %d, size %d, unbalance %d\n", g+1, grp->gr_size, grp->gr_unbalance);
	unbal_sum_square += grp->gr_unbalance*grp->gr_unbalance;;
	if (grp->gr_size == 0)
	    continue;

	/*
	 * You should randomize pairs in genereal
	 */

	if (!seq_strength) {
	    qsort((void *) grp->gr_pairs, MAXMEMBERS, sizeof(grp->gr_pairs[0]), pair_compar);
	}

	sprintf(fname, "seeded%0*d.txt", numberlen, g+1);
	outf = trad_output ? stdout : fopen(fname, "w");
	for (p=0; p<MAXMEMBERS; p++) {
	    prp = grp->gr_pairs[p];
	    if (prp == 0)
		continue;
	    fprintf(outf, "%s,%s,%d,%s\n", prp->pair_id1, prp->pair_id2,
		prp->pair_class, prp->pair_property->pv_string);
	}
	if (!trad_output)
		fclose(outf);
    }
    fprintf(stderr, "Unbalance sum of squares: %.1f\n", (double) unbal_sum_square/10000.0);
}

int
main (int argc, char *argv[])
{
    int c;
    int n,s;
    void decode_xstring();
    int index;

#ifdef DEB
    debug = fopen("debug", "w");
    setbuf(debug, NULL);
#endif
    DEBUG(fprintf(stderr, "Starting\n"));
    while ((c = getopt(argc, argv, OPTION_STRING)) != -1) {
	switch(c) {
	case 's':
	    seq_strength = 1;
	    break;
	case 'w':
	    wheelchairmode = 1;
	    break;
	case 't':
	    trad_output = 1;
	    break;
	case '?':
	    fprintf(stderr, "Usage: %s %s\n", argv[0], USAGE_STRING);
	    exit(-1);
	}
    }
    for (index = optind; index < argc; index++)
	input_grouparg(argv[index]);

    srandom(getpid());

    /*
     * Backwards compatibility, if no groupsizes on command line read from file
     */
    if (totalgroups==0 && !input_groupsize(stdin))
	return -1;
    pairsingroups = init_groups();
    if (!input_pairs(stdin))
	return -1;
    if (pairsingroups != totalpairs) {
	fprintf(stderr, "Defined groups have %d pairs, there are %d pairs\n", pairsingroups, totalpairs);
	exit(-1);
    }
    DEBUG(pairclass_dump());
    DEBUG(fprintf(debug, "calling seed_pairs\n"));
    seed_pairs();
    output_groups();
}
