// query.c ... query scan functions
// part of signature indexed files
// Manage creating and using Query objects
// Written by John Shepherd, March 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q)
{
	if (*q == '\0') return 0;
	char *c;
	int nattr = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nattr++;
	return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs)
{
	Query new = malloc(sizeof(QueryRep));
	assert(new != NULL);
	if (!checkQuery(r,q)) return NULL;
	new->rel = r;
	new->qstring = q;
	new->nsigs = new->nsigpages = 0;
	new->ntuples = new->ntuppages = new->nfalse = 0;
	new->pages = newBits(nPages(r));
	switch (sigs) {
	case 't': findPagesUsingTupSigs(new); break;
	case 'p': findPagesUsingPageSigs(new); break;
	case 'b': findPagesUsingBitSlices(new); break;
	default:  setAllBits(new->pages); break;
	}
	new->curpage = 0;
	return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void count_tuple_match(Query q, int pid, int* number){
    Tuple check_tuple;
    Page check_page = getPage(dataFile(q->rel), pid);
    for(int tuple_id =0; tuple_id<pageNitems(check_page);tuple_id++){
        check_tuple = getTupleFromPage(q->rel, check_page, tuple_id);//extract tuple from the goal page
        q->ntuples++;//add one to the tuples query has checked
        if(tupleMatch(q->rel,check_tuple,q->qstring)){
            showTuple(q->rel,check_tuple);
            (*number)++;
        }
    }
    free(check_page);
}

void scanAndDisplayMatchingTuples(Query q)
{
	assert(q != NULL);
	//TODO
    int number_match;
    for(int pid=0; pid<nPages(q->rel);pid++){
        if(bitIsSet(q->pages,pid)){//if this page is set to 1,then we can go ahead, or jump to the next pid
            number_match=0;//record the number of tuple which is matched successfully.
            count_tuple_match(q,pid,&number_match);
            q->ntuppages++;
            if(number_match==0) q->nfalse++;
        }
    }
}

// print statistics on query

void queryStats(Query q)
{
	printf("# sig pages read:    %d\n", q->nsigpages);
	printf("# signatures read:   %d\n", q->nsigs);
	printf("# data pages read:   %d\n", q->ntuppages);
	printf("# tuples examined:   %d\n", q->ntuples);
	printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q->pages);
	free(q);
}

