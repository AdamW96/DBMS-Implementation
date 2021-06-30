// tsig.c ... functions on Tuple Signatures (tsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"


// make a tuple signature
static Bits tsig_codeword(char *attr, int m, int k){
    Bits codeword=newBits(m);
    int index_k=0;
    srandom(hash_any(attr, strlen(attr)));
    while( index_k<k ){//record the number of 1 bit in the bit string;
        int i=random()%m;
        if(!(bitIsSet(codeword, i))){
            setBit(codeword,i);
            index_k++;
        }
    }
    return codeword;
}
static Bits tsig_codeword_catc(char *attr, int m, int k, int attr_m){
    Bits codeword=newBits(m);
    int index_k=0;
    srandom(hash_any(attr, strlen(attr)));
    while( index_k<attr_m/2 ){//record the number of 1 bit in the bit string;
        int i=random()%attr_m;
        if(!(bitIsSet(codeword, i))){
            setBit(codeword,i);
            index_k++;
        }
    }
    return codeword;
}

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
//    int k=codeBits(r);
//    int tm=tsigBits(r);
    Bits tuple_signature=newBits(tsigBits(r));
    char **All_attributes=tupleVals(r,t);
	if(sigType(r)=='s'){
        for(int i=0;i<nAttrs(r);i++){
            if(strcmp(All_attributes[i],"?")!=0){
                Bits mask=tsig_codeword(All_attributes[i],tsigBits(r),codeBits(r));
                orBits(tuple_signature, mask);
                freeBits(mask);
            }
        }
	}
	else{//if signature type = 'c'
	    int catc_m = tsigBits(r)/nAttrs(r);
	    int pending_catcM=catc_m+tsigBits(r)%nAttrs(r);
	    int shift_bits=0;
        for(int i=0;i<nAttrs(r);i++){
            if(strcmp(All_attributes[i],"?")!=0){
                if(i==0){//that means we need to pending this atrr
                    tuple_signature=tsig_codeword_catc(All_attributes[i],tsigBits(r),codeBits(r),pending_catcM);
                    shift_bits+=pending_catcM;
                }else{
                    Bits mask=tsig_codeword_catc(All_attributes[i],tsigBits(r),codeBits(r),catc_m);
                    shiftBits(mask,shift_bits);
                    orBits(tuple_signature, mask);
                    shift_bits +=catc_m;
                    freeBits(mask);
                }
            }else{
                if(i==0) shift_bits+=pending_catcM;
                else shift_bits+=catc_m;
            }
        }
	}
	return tuple_signature;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	//TODO
	unsigned int number_sigs;
    PageID data_pageid;
    Bits query_sig=makeTupleSig(q->rel, q->qstring);
    Bits check_sig = newBits(tsigBits(q->rel));
    Page TsigPage = newPage();
    unsetAllBits(q->pages);     //inital all bits in q->pages
    for(int pageid=0; pageid<nTsigPages(q->rel);pageid++){
        TsigPage=getPage(tsigFile(q->rel),pageid);
        for(int tupleid=0; tupleid<pageNitems(TsigPage);tupleid++){
            getBits(TsigPage,tupleid,check_sig);
            if(isSubset(query_sig,check_sig)){//if match successfully
                number_sigs = pageid*maxTsigsPP(q->rel)+tupleid+1;
                data_pageid = number_sigs/maxTupsPP(q->rel);
                if(!bitIsSet(q->pages,data_pageid)){
                    setBit(q->pages,data_pageid);
                }// if the position bit is 0, then we set it into 1
            }
            q->nsigs++;
        }
    }
    q->nsigpages=nTsigPages(q->rel);
    freeBits(check_sig);
    freeBits(query_sig);
    free(TsigPage);
	// The printf below is primarily for debugging
	// Remove it before submitting this function
//	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
