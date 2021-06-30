// bits.c ... functions on bit-strings
// part of signature indexed files
// Bit-strings are arbitrarily long byte arrays
// Least significant bits (LSB) are in array[0]
// Most significant bits (MSB) are in array[nbytes-1]

// Written by John Shepherd, March 2019

#include <assert.h>
#include "defs.h"
#include "bits.h"
#include "page.h"
typedef struct _BitsRep {
	Count  nbits;		  // how many bits
	Count  nbytes;		  // how many bytes in array
	Byte   bitstring[1];  // array of bytes to hold bits
	                      // actual array size is nbytes
} BitsRep;

// create a new Bits object

Bits newBits(int nbits)
{
	Count nbytes = iceil(nbits,8);
	Bits new = malloc(2*sizeof(Count) + nbytes);
	new->nbits = nbits;
	new->nbytes = nbytes;
	memset(&(new->bitstring[0]), 0, nbytes);
	return new;
}

// release memory associated with a Bits object

void freeBits(Bits b)
{
	//TODO
	free(b);
}

// check if the bit at position is 1

Bool bitIsSet(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	//TODO
	int WhichByte = position/8;
	int WhichBit = position%8;
	unsigned char mask = (1<<WhichBit);
	if (b->bitstring[WhichByte]&mask){
	    return TRUE;
	}
	return FALSE; // remove this
}

// check whether one Bits b1 is a subset of Bits b2

Bool isSubset(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	//TODO
	int index=1;
	for(int i=0; i<b1->nbytes;i++){
	    if((b1->bitstring[i]&b2->bitstring[i])!=b1->bitstring[i]){
            index=0;
	        break;
	    }
	}
    if(index ==1){
        return TRUE;
    }
    else return FALSE;
	 // remove this
}

// set the bit at position to 1

void setBit(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	//TODO
	int WhichByte=position/8;
	int WhichBit=position%8;
    unsigned char setbit=(1<<WhichBit);
	b->bitstring[WhichByte]=b->bitstring[WhichByte]|setbit;
}

// set all bits to 1

void setAllBits(Bits b)
{
	assert(b != NULL);
	//TODO
    unsigned char mask=0xFF;
	for(int i=0;i<b->nbytes;i++){
	    b->bitstring[i]=b->bitstring[i]|mask;
	}

}

// set the bit at position to 0

void unsetBit(Bits b, int position)
{
	assert(b != NULL);
	assert(0 <= position && position < b->nbits);
	//TODO
    int WhichByte = position/8;
    int WhichBit = position%8;
    unsigned char mask=~(0x1<<WhichBit);
    b->bitstring[WhichByte] = b->bitstring[WhichByte]& mask;
}

// set all bits to 0

void unsetAllBits(Bits b)
{
	assert(b != NULL);
	//TODO
    unsigned char mask=0x00;
	for(int i=0;i<b->nbytes;i++){
        b->bitstring[i]=b->bitstring[i]|mask;
	}
}

// bitwise AND ... b1 = b1 & b2

void andBits(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	//TODO
	for(int i=0;i<b1->nbytes;i++){
        b1->bitstring[i]=b1->bitstring[i]&b2->bitstring[i];
	}
}

// bitwise OR ... b1 = b1 | b2

void orBits(Bits b1, Bits b2)
{
	assert(b1 != NULL && b2 != NULL);
	assert(b1->nbytes == b2->nbytes);
	//TODO
    for(int i=0;i<b1->nbytes;i++){
        b1->bitstring[i]=b1->bitstring[i]|b2->bitstring[i];
    }
}

// left-shift ... b1 = b1 << n
// negative n gives right shift
void set_pending(Bits b){
    int pending=b->nbytes*8-b->nbits;
    unsigned char mask;
    unsigned char temp;
    for(int i =0;i<8-pending;i++){
        temp=(1<<i);
        mask=mask|temp;
    }//first set a index to & with the pending byte
    //if pending=3,then mask=00011111
    b->bitstring[b->nbytes-1]&=mask;
}
void shift_left(Bits b, int n){
    int CurPosition;
    unsigned char mask;
    set_pending(b);
    for(int i=b->nbytes-1;i>=0;i--){
        for(int j=7;j>=0;j--){
            mask=1<<j;
            CurPosition=i*8+j;
            if(b->bitstring[i]&mask){
                if(CurPosition+n<b->nbits){
                    setBit(b,CurPosition+n);
                    unsetBit(b,CurPosition);
                }else unsetBit(b,CurPosition);
            }
            else{
                if(CurPosition+n<b->nbits){
                    unsetBit(b,CurPosition+n);
                }
            }
        }
    }
    set_pending(b);
}
void shift_right(Bits b, int n){
    int CurPosition;
    unsigned char mask;
    set_pending(b);
    for(int i=0;i<b->nbytes;i++){
        for(int j=0;j<8;j++){
            mask=1<<j;
            CurPosition=i*8+j;
            if(b->bitstring[i]&mask){
                if(CurPosition-n>=0){
                    setBit(b,CurPosition-n);
                    unsetBit(b,CurPosition);
                }else unsetBit(b,CurPosition);
            }
            else{
                if(CurPosition-n>=0 &&CurPosition-n<b->nbits ){
                    unsetBit(b,CurPosition-n);
                }
            }
        }
    }
    set_pending(b);
}

void shiftBits(Bits b, int n)
{
    // TODO
    if(n > 0){
        shift_left(b, n);
    }
    else if(n < 0){
        n=-n;
        shift_right(b, n);
    }
}

// get a bit-string (of length b->nbytes)
// from specified position in Page buffer
// and place it in a BitsRep structure

void getBits(Page p, Offset pos, Bits b)
{
	//TODO
    unsigned char *begin = addrInPage(p, pos, b->nbytes);
    memcpy(b->bitstring, begin, b->nbytes);

}

// copy the bit-string array in a BitsRep
// structure to specified position in Page buffer

void putBits(Page p, Offset pos, Bits b)
{
	//TODO
    unsigned char *begin = addrInPage(p, pos, b->nbytes);
    memcpy(begin, b->bitstring, b->nbytes);
}

// show Bits on stdout
// display in order MSB to LSB
// do not append '\n'

void showBits(Bits b)
{
	assert(b != NULL);
    //printf("(%d,%d)",b->nbits,b->nbytes);
//    strcpy(b->bitstring,"012");
	for (int i = b->nbytes-1; i >= 0; i--) {
		for (int j = 7; j >= 0; j--) {
            unsigned char mask = (1 << j);
			if (b->bitstring[i] & mask)
				putchar('1');
			else
				putchar('0');
		}
	}
}
//added by myself, used to debug, remember delete at last
void printbits(Bits b){
    printf("(%d,%d)",b->nbits,b->nbytes);
    printf("%s",b->bitstring);
}
