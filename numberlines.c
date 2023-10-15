#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "subr.h"

#define random rand
#define srandom srand

int usertime();
void setup_catchint();
int play_in_round();
void read_schedule();

/* #define DEBUG			/* debugging on or off */
 
#define LINESIZE	200		/* max length of input line */
#define MAXCOUNTRIES	50		/* maximum number of countries */

#define MINSECS		30		/* minimum seconds to compute */
#define MAXSECS		900		/* max seconds to compute */

#define INFINITY 	1000000		/* Init score */
#define FRACTION 	1000		/* When computing int + small random */

typedef struct pair pair_t, *pair_p;
typedef struct country country_t, *country_p;
typedef struct country_ref country_ref_t, *country_ref_p;

/*
 * Information per pair
 * Backlinked into country[]
 */
struct pair {
	pair_p		pr_next;	/* linked list of pairs */
	char		*pr_id1;	/* id1 of pair */
	char		*pr_id2;	/* id2 of pair */
	int		pr_strength;	/* strength (unused this program) */
	country_p	pr_country;	/* pointer to country */
	int		pr_lowbnd;	/* lowest possible place in schedule */
	int		pr_highbnd;	/* highest possible place in schedule */
};

/*
 * Information per country
 */
struct country {
	char 		*ct_name;	/* name of country */
	int		ct_npairs;	/* count of pairs in country */
	pair_p		ct_pairs;	/* start of linked list of pairs */
};

/*
 * Separate array pointing into country[]
 * This can be sorted without destroying the linkage from pair to country
 */
struct country_ref {
	country_p	cr_country;	/* country */
	int		cr_score;	/* To order countries on */
};

country_t	countries[MAXCOUNTRIES];
country_ref_t	country_refs[MAXCOUNTRIES];
int ncountries;

int
min(int a, int b) {

	if (a>b)
		return b;
	return a;
}

int
max(int a, int b) {

	if (a>b)
		return a;
	return b;
}

#ifdef DEBUG
/*
 * debugging routine
 */
void
dump_countries() {
	struct country_ref *crp;
	country_p cp;
	pair_p pp;

	for (crp=country_refs;crp<country_refs+ncountries;crp++) {
		cp = crp->cr_country;
		printf("Country %s,%d pairs, score %d\n", cp->ct_name, cp->ct_npairs, crp->cr_score);
		for(pp=cp->ct_pairs;pp;pp=pp->pr_next) {
			printf("\t%s,%s,%d,%d,%d\n", pp->pr_id1, pp->pr_id2,
				pp->pr_strength, pp->pr_lowbnd, pp->pr_highbnd);
		}
	}
}
#endif

/*
 * Enter a pair into the administration on input
 */
void
enter_pair(country, id1, id2, strength, lowbnd, highbnd)
char *country, *id1, *id2, *strength;
int lowbnd, highbnd;
{
	country_p	cp;
	country_ref_p	cr;
	pair_p		pp,*ppp;
	int		i;

	/*
	 * Loop over the country array, to find the right slot, or the first empty slot
	 */
	for (i=0; i<MAXCOUNTRIES; i++) {
		cp = countries+i;
		cr = country_refs+i;
		if (cp->ct_npairs == 0) {
			/*
			 * Empty slot, meaning new country
			 * initialise slot and fall through
			 */
			cp->ct_name = string_copy(country);
			cr->cr_country = cp;
			ncountries++;
			/* fall through */
		}
		if (strcmp(cp->ct_name, country)==0) {
			/*
			 * found country, make pair info and hook into country
			 * linked list
			 */
			pp = (pair_p) calloc(1, sizeof(pair_t));
			pp->pr_id1 = string_copy(id1);
			pp->pr_id2 = string_copy(id2);
			pp->pr_strength = atoi(strength);
			pp->pr_country = cp;
			pp->pr_lowbnd = lowbnd;
			pp->pr_highbnd = highbnd;

			pp->pr_next = cp->ct_pairs;
			cp->ct_pairs = pp;
			cp->ct_npairs++;
#ifdef DEBUG
			printf("Country %s now %d pairs\n", cp->ct_name, cp->ct_npairs);
#endif
			return;
		}

	}
	error("Too many countries");
}

/*
 * read a couple of pairs and enter into admin
 */
void
read_pairs(int npairs,int lb,int hb) {
	int i;
	char line[LINESIZE];
	char pairid1[LINESIZE], pairid2[LINESIZE], strength[LINESIZE], country[LINESIZE];
	char errbuf[2*LINESIZE];


	for (i=1;i<=npairs;i++) {
		fgets(line, LINESIZE, stdin);
		if(sscanf(line,"%[^,],%[^,],%[^,],%[^,\n]\n",
				pairid1, pairid2, strength, country)!=4) {
			sprintf(errbuf, "Pair %d bad format: %s", i, line);
			error(errbuf);
		}
#ifdef DEBUG
		printf("pair %s,%s strngth %s, country %s\n",pairid1,pairid2,strength,country);
#endif
		enter_pair(country,pairid1,pairid2,strength,lb,hb);
	}
}

/*
 * Section doing actual trials
 */

pair_p *currentnumbering, *bestnumbering;
int totalpairs;

void
init_numbering(int npairs) {

	/*
	 * Allocate the necessary storage for current and best result
	 */
	currentnumbering = (pair_p *) calloc((npairs+1),sizeof(pair_p));
	bestnumbering = (pair_p *) calloc((npairs+1),sizeof(pair_p));
	totalpairs = npairs;
}

void
clear_numbering() {
	int i;

	/*
	 * Clear the current result
	 */
	for (i=1;i<=totalpairs;i++)
		currentnumbering[i] = 0;
}

void
save_numbering() {
	int i;

	/*
	 * Current result clearly good, copy it to best
	 */
	for (i=1;i<=totalpairs;i++)
		bestnumbering[i] = currentnumbering[i];
}

void
output_numbering() {
	int i;
	pair_p pp;

	/*
	 * Output the result of this program
	 */
	for(i=1;i<=totalpairs;i++) {
		pp = bestnumbering[i];
		printf("%s,%s,%d,%s\n", pp->pr_id1, pp->pr_id2, pp->pr_strength, pp->pr_country->ct_name);
	}
}

/*
 * section doing analysis per country
 */

#define MAXPAIRS	50

struct pair_order {
	pair_p	po_pair;
	int	po_pref;	/* meant to do smart ordering, seems not needed */
} pair_order[MAXPAIRS];
int n_pair_order;

int
compare_pair(po1, po2) struct pair_order *po1, *po2;
{

	return po2->po_pref - po1->po_pref;
}

void
order_pairs(cp) country_p cp;
{
	pair_p pp;
	int i;

	if (cp->ct_npairs > MAXPAIRS)
		error("Too many pairs in a country");
	pp = cp->ct_pairs;
	for (i=0; i<cp->ct_npairs; i++) {
		/*
		 * enter pair into pair_order[]
		 * could do something smart with ordering here
		 * experience shows random is good enough for now
		 * 
		 * The whole idea with schedules in multiple groups(prob 2)
		 * is to alternate groups in order to book meetings early
		 * but depends on schedule
		 */
		pair_order[i].po_pair = pp;
		pair_order[i].po_pref = random();	/* for now */
		pp = pp->pr_next;
	}
	/*
	 * order pairs by sorting
	 */
	qsort(pair_order, cp->ct_npairs, sizeof(struct pair_order), compare_pair);
	n_pair_order = cp->ct_npairs;
}

int
play_last_round(pos, pp) int pos; pair_p pp;
{
	int i;
	int score, worstround;
	pair_p sittingpair;

	/*
	 * return the last round with a same country meeting if we added
	 * pair pp at position pos
	 */
#ifdef DEBUG
	printf("play_last_round(%d, %x) -> ", pos, pp);
#endif
	worstround = 0;
	for(i=1; i<= totalpairs; i++) {
		sittingpair = currentnumbering[i];
		if (sittingpair && sittingpair->pr_country==pp->pr_country) {
			/*
			 * Same country players
			 */
			score = play_in_round(pos, i);
			if (score > worstround)
				worstround = score;
		}
	}
#ifdef DEBUG
	printf("%d\n", worstround);
#endif
	return worstround;
}

int
number_pairs() {
	int i;
	int lb,hb;
	int position, bestposition, score, lowest_score;
	int high_lowest_score;

	high_lowest_score = 0;
	
	/*
	 * Loop over all pairs from this country
	 */
	for(i=0; i< n_pair_order;i++) {
		lb = pair_order[i].po_pair->pr_lowbnd; 
		hb = pair_order[i].po_pair->pr_highbnd; 
		lowest_score = INFINITY;
		/*
		 * try all possible empty positions in the range for this pair
		 */
		for (position=lb;position<=hb;position++) {
			if (currentnumbering[position]!=0)
				continue;
			/*
			 * This is a possible place
			 * compute score which is in effect the last round
			 * where this pair meets countrymen, plus a small
			 * random offset
			 */
			score = play_last_round(position, pair_order[i].po_pair);
			score = FRACTION*score+random()%FRACTION;
			if (score < lowest_score) {
				/*
				 * found better than sofar
				 */
				bestposition = position;
				lowest_score = score;
			}
		}
		/*
		 * put pair into the computed place
		 */
#ifdef DEBUG
		printf("Entered pair %d(%d, %d) at position %d(%d)\n", i, lb, hb, bestposition, lowest_score);
#endif
		if (lowest_score == INFINITY)
			error("no place for pair in number_pars");
		currentnumbering[bestposition] = pair_order[i].po_pair;
		/*
		 * compute the highest of all low scores
		 * so the last round where any of these pairs plays countrymen
		 */
		if (lowest_score/FRACTION > high_lowest_score)
			high_lowest_score = lowest_score/FRACTION;
	}

#ifdef DEBUG
	printf("number_pairs() returns %d\n", high_lowest_score);
#endif
	return high_lowest_score;
}

int
compare_country_ref(cr1, cr2) country_ref_p cr1, cr2;
{

	return cr2->cr_score - cr1->cr_score;
}

int
numbering_try(roundp, squarep, bestlr) int *roundp, *squarep; int bestlr; {
	country_ref_p	cr;
	int lr,worstround, square;

#ifdef DEBUG
	printf("Numbering_try()\n");
#endif
	/*
	 * clear array of pairs
	 * set return value to no meetings of countrymen (0)
	 */
	clear_numbering();
	worstround = 0; square = 0;
	for (cr=country_refs; cr<country_refs+ncountries; cr++)
		/* add up to 25% random to mix it up a bit */
		cr->cr_score = (4*FRACTION+random()%FRACTION)*cr->cr_country->ct_npairs;
	qsort(country_refs, ncountries, sizeof(country_ref_t), compare_country_ref);
	for (cr=country_refs; cr<country_refs+ncountries; cr++) {
		order_pairs(cr->cr_country);
		lr = number_pairs();
#ifdef DEBUG
		printf("Numbering_try() lr=%d, worstround=%d\n", lr, worstround);
#endif
		/*
		 * lr is last round any pairs of this country meet
		 * compute worst over all countries in worstround
		 */
		if (lr >worstround) {
			worstround = lr;
			if (worstround > bestlr)
				break;
		}
		square += lr*lr;
	}
	*roundp = worstround;
	*squarep = square;
}

int
main() {
	int ngroups, *groupsizes;
	int lr, bestlr;
	int square, bestsquare;
	int tryno, besttryno;
	int i, lownr, highnr;
	int maxtries, newmaxtries, timelimtries;
	int ut;
	extern int interrupted;

	srandom(time(NULL)+getpid());
	read_schedule(&ngroups, &groupsizes);
#ifdef DEBUG
	printf("There are %d groups: sizes", ngroups);
	for(i=0; i<ngroups; i++)
		printf(" %d", groupsizes[i]);
	printf("\n");
#endif
	highnr = 0;
	for(i=0; i<ngroups; i++) {
		lownr = highnr+1;
		highnr = lownr-1+groupsizes[i];
		read_pairs(groupsizes[i], lownr, highnr);
	}
#ifdef DEBUG
	dump_countries();
#endif
	init_numbering(highnr);

	setup_catchint();
	maxtries = 2000000;
	bestlr = INFINITY;
	bestsquare = INFINITY;
	tryno = 0;
	do {
		tryno++;
		numbering_try(&lr, &square, bestlr);
#ifdef DEBUG
		printf("main() tryno=%d, lr=%d, bestlr=%d, square=%d, bestsquare=%d\n", tryno, lr, bestlr, square, bestsquare);
#endif
		if (lr < bestlr || (lr == bestlr && square < bestsquare)) {
			besttryno = tryno;
			fprintf(stderr, "At try %d(%d) round %d/%d beats %d/%d\n", besttryno, maxtries, lr, square, bestlr, bestsquare);
			newmaxtries = 10*tryno;
			if (maxtries < newmaxtries) {
				ut = usertime();
				timelimtries = MAXSECS/ut*tryno;
				/*
				 * Avoid taking more than a certain amount of time
				 */
				newmaxtries = min(newmaxtries, timelimtries);
				if (maxtries < newmaxtries) {
					fprintf(stderr, "Extending search to %d tries\n", newmaxtries);
					maxtries = newmaxtries;
				}
			}
			save_numbering();
			bestlr = lr;
			bestsquare = square; /* even if worse: lr takes precedence */
		}
		if (tryno >= maxtries && tryno/besttryno<1000) {
			ut = usertime();
			timelimtries = MINSECS/ut*tryno;
			/*
			 * Do not finish with undue haste
			 */
			newmaxtries = max(maxtries, timelimtries);
			if (maxtries < newmaxtries) {
				fprintf(stderr, "Extending search to %d tries\n", newmaxtries);
				maxtries = newmaxtries;
			}
		}
	} while(tryno<maxtries && !interrupted);
	fprintf(stderr, "Found best possibility at try %d(%d), score %d/%d\n", besttryno, tryno, bestlr, bestsquare);
	output_numbering();
	return 0;
}
