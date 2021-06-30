// psig.c ... functions on page signatures (psig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"
#include "bits.h"

static Bits psig_codeword(char *attr, int m, int k){
    Bits codeword=newBits(m);
    int index_k=0;
    srandom(hash_any(attr, strlen(attr)));
    while(index_k<k){
        int i=random() % m;
        if(!(bitIsSet(codeword, i))){
            setBit(codeword,i);
            index_k++;
        }
    }
    return codeword;
}

static Bits psig_codeword_catc(char *attr, int m, int k, int attr_m){
    Bits codeword=newBits(m);
    int index_k=0;
    srandom(hash_any(attr, strlen(attr)));
    while( index_k<attr_m/(2*k) ){//record the number of 1 bit in the bit string;
        int i=random()%attr_m;
        if(!(bitIsSet(codeword, i))){
            setBit(codeword,i);
            index_k++;
        }
    }
    return codeword;
}

Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
    Bits page_sig=newBits(psigBits(r));
    char **All_attributes=tupleVals(r,t);
    if(sigType(r)=='s'){
        for(int i=0;i<nAttrs(r);i++){
            if(strcmp(All_attributes[i],"?")!=0){
                Bits mask=psig_codeword(All_attributes[i],psigBits(r),codeBits(r));
                orBits(page_sig, mask);
                freeBits(mask);
            }
        }
    }
    else{//if signature type = 'c'
        int pk =maxTupsPP(r);
        int catc_m = psigBits(r)/nAttrs(r);
        int pending_catcM=catc_m+psigBits(r)%nAttrs(r);
        int shift_bits=0;
        for(int i=0;i<nAttrs(r);i++){
            if(strcmp(All_attributes[i],"?")!=0){
                if(i==0){//that means we need to pending this atrr
                    page_sig=psig_codeword_catc(All_attributes[i],psigBits(r),pk,pending_catcM);
                    shift_bits+=pending_catcM;
                }else{
                    Bits mask=psig_codeword_catc(All_attributes[i],psigBits(r),pk,catc_m);
                    shiftBits(mask,shift_bits);
                    orBits(page_sig, mask);
                    shift_bits +=catc_m;
                    freeBits(mask);
                }
            }else{
                if(i==0) shift_bits+=pending_catcM;
                else shift_bits+=catc_m;
            }
        }
    }
    return page_sig;

}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	//TODO
    unsetAllBits(q->pages);//initial the pages of q
    Bits query_sig=makePageSig(q->rel, q->qstring);
    Bits check_sig=newBits(psigBits(q->rel));
    Page check_page;
    PageID psig_number;
    for(int pageid=0;pageid<nPsigPages(q->rel);pageid++){
        check_page=getPage(psigFile(q->rel),pageid);
        for(int psig_id=0;psig_id<pageNitems(check_page);psig_id++){
            getBits(check_page,psig_id,check_sig);
            if(isSubset(query_sig,check_sig)){//if match successfully
                psig_number = pageid*maxPsigsPP(q->rel)+psig_id+1;
                if(!bitIsSet(q->pages,psig_number-1)){
                    setBit(q->pages,psig_number-1);
                }
            }//if the query has been matched
            q->nsigs=pageid*maxPsigsPP(q->rel)+psig_id+1;
        }
    }
    q->nsigpages =nPsigPages(q->rel) ;
    freeBits(query_sig);
    freeBits(check_sig);
    free(check_page);
}

