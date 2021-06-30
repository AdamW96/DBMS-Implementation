// bsig.c ... functions on Tuple Signatures (bsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);
	//TODO
    Page check_p;
    PageID Last_page=-1;
    Bits query_sig = makePageSig(q->rel, q->qstring);
    Bits check_sig = newBits(bsigBits(q->rel));//make the query into codeword;
    setAllBits(q->pages);//firstly set all bits into 1
    for(int bit_psig=0; bit_psig<psigBits(q->rel); bit_psig++){
        if(!bitIsSet(query_sig, bit_psig)) continue;
        //if the bit set in this condition is 0, then jump to the next bit of page signature
        if(bit_psig/maxBsigsPP(q->rel)!=Last_page){
            check_p=getPage(bsigFile(q->rel),bit_psig/(maxBsigsPP(q->rel)));
            q->nsigpages++;
            Last_page=bit_psig/maxBsigsPP(q->rel);
            getBits(check_p,bit_psig%maxBsigsPP(q->rel),check_sig);
            q->nsigs++;
            for(int bit_bsig=0;bit_bsig<nPsigs(q->rel);bit_bsig++){
                if(bitIsSet(check_sig,bit_bsig)) continue;
                unsetBit(q->pages,bit_bsig);
            }
        }else{//if the current the page is same with the last page
            getBits(check_p,bit_psig%maxBsigsPP(q->rel),check_sig);
            q->nsigs++;
            for(int bit_bsig=0;bit_bsig<nPsigs(q->rel);bit_bsig++){
                if(bitIsSet(check_sig,bit_bsig)) continue;
                unsetBit(q->pages,bit_bsig);
            }
        }
    }
    freeBits(query_sig);
    freeBits(check_sig);
}

