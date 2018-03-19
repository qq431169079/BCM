 /*
 * Abstract Syntax Notation One, ASN.1
 * As defined in ISO/IS 8824 and ISO/IS 8825
 * This implements a subset of the above International Standards that
 * is sufficient to implement SNMP.
 *
 * Encodes abstract data types into a machine independent stream of bytes.
 *
 */
/**********************************************************************
 <:label-BRCM:1988:NONE:standard
 
 :>
    Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include "asn1.h"
#include "agtMib.h"

#define SNMP_VOID void
#ifndef NULL
#define NULL	0
#endif

/*
 * asn_parse_int - pulls a long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_int(data, datalength, type, intp, intsize)
    register unsigned char *data;  /* IN - pointer to start of object */
    register int *datalength;/* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;     /* OUT - asn type of object */
    long *intp;              /* IN/OUT - pointer to start of output buffer */
    int intsize;             /* IN - size of output buffer */
{
    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register unsigned char *bufp = data;
    unsigned long       asn_length;
    register long   value = 0;

    if (intsize != sizeof (long))
    {
        SNMP_DEBUG("not long");
        return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
    {
        SNMP_DEBUG("bad length");
        return NULL;
    }
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    if ((int)asn_length > intsize)
    {
        SNMP_DEBUG("I don't support such large integers");
        return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
        value = -1; /* integer is negative */
    while(asn_length--)
        value = (value << 8) | *bufp++;
    *intp = value;

    SNMP_DEBUG("bufp(%p), *intp(0x%X)", bufp, *intp);

    return bufp;
}


/*
 * asn_parse_unsigned_int - pulls an unsigned long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_unsigned_int(data, datalength, type, intp, intsize)
    register unsigned char *data; /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;      /* OUT - asn type of object */
    unsigned long *intp;      /* IN/OUT - pointer to start of output buffer */
    int intsize;              /* IN - size of output buffer */
{

    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register unsigned char *bufp = data;
    unsigned long       asn_length;
    register unsigned long value = 0;

    if (intsize != sizeof (long))
    {
        SNMP_DEBUG("not long");
        return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
    {
        SNMP_DEBUG("bad length");
        return NULL;
    }
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    if (((int)asn_length > (intsize + 1)) ||
        (((int)asn_length == intsize + 1) && *bufp != 0x00))
    {
        SNMP_DEBUG("I don't support such large integers");
        return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
        value = -1; /* integer is negative */
    while(asn_length--)
        value = (value << 8) | *bufp++;
    *intp = value;
    return bufp;
}


/*
 * asn_build_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_int(data, datalength, type, intp, intsize)
    register unsigned char *data;   /* IN - pointer to start of output buffer */
    register int *datalength;/* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;      /* IN - asn type of object */
    register long *intp;     /* IN - pointer to start of long integer */
    register int intsize;    /* IN - size of *intp */
{

    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register long integer;
    register unsigned long mask;

    if (intsize != sizeof (long))
        return NULL;
    integer = *intp;

    /* Truncate "unnecessary" bytes off of the most significant end of this
     * 2's complement integer.  There should be no sequence of 9
     * consecutive 1's or 0's at the most significant end of the
     * integer.
     */
    mask = 0x1FF << ((8 * (sizeof(long) - 1)) - 1);

    /* mask is 0xFF800000 on a big-endian machine */
    while((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1)
    {
        intsize--;
        integer <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
        return NULL;
    if (*datalength < intsize)
        return NULL;
    *datalength -= intsize;
    mask = 0xFF << (8 * (sizeof(long) - 1));

    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--)
    {
        *data++ = (unsigned char)((integer & mask) >> (8 * (sizeof(long) - 1)));
        integer <<= 8;
    }
    return data;
}


/*
 * asn_build_unsigned_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_unsigned_int(data, datalength, type, intp, intsize)
    register unsigned char *data;   /* IN - pointer to start of output buffer */
    register int *datalength;/* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;      /* IN - asn type of object */
    register unsigned long *intp;   /* IN - pointer to start of long integer */
    register int intsize;    /* IN - size of *intp */
{

    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register unsigned long integer;
    register unsigned long mask;
    int add_null_byte = 0;

    if (intsize != sizeof (long))
        return NULL;
    integer = *intp;
    mask = 0xFF << (8 * (sizeof(long) - 1));

    /* mask is 0xFF000000 on a big-endian machine */
    if ((unsigned char)((integer & mask) >> (8 * (sizeof(long) - 1))) & 0x80)
    {
        /* if MSB is set */
        add_null_byte = 1;
        intsize++;
    }

    /* Truncate "unnecessary" bytes off of the most significant end of this 2's
     * complement integer.  There should be no sequence of 9 consecutive 1's or
     * 0's at the most significant end of the integer.
     */
    mask = 0x1FF << ((8 * (sizeof(long) - 1)) - 1);

    /* mask is 0xFF800000 on a big-endian machine */
    while((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1)
    {
        intsize--;
        integer <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
        return NULL;
    if (*datalength < intsize)
        return NULL;
    *datalength -= intsize;
    if (add_null_byte == 1)
    {
        *data++ = '\0';
        intsize--;
    }
    mask = 0xFF << (8 * (sizeof(long) - 1));

    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--)
    {
        *data++ = (unsigned char)((integer & mask) >> (8 * (sizeof(long) - 1)));
        integer <<= 8;
    }
    return data;
}


/*
 * asn_parse_string - pulls an octet string out of an ASN octet string type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the octet string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_string(data, datalength, type, string, strlength)
    unsigned char *data;      /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;      /* OUT - asn type of object */
    unsigned char *string;    /* IN/OUT - pointer to start of output buffer */
    register int *strlength;  /* IN/OUT - size of output buffer */
{

    /* ASN.1 octet string ::= primstring | cmpdstring
     * primstring ::= 0x04 asnlength byte {byte}*
     * cmpdstring ::= 0x24 asnlength string {string}*
     */
    register unsigned char *bufp = data;
    unsigned long       asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return NULL;
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    if ((int)asn_length > *strlength)
    {
        SNMP_DEBUG("I don't support such long strings");
        return NULL;
    }
    memcpy((char *)string, (char *)bufp, (int)asn_length);
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);
    return bufp + asn_length;
}


/*
 * asn_build_string - Builds an ASN octet string object containing the input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_string(data, datalength, type, string, strlength)
    unsigned char *data;      /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;       /* IN - ASN type of string */
    unsigned char *string;    /* IN - pointer to start of input buffer */
    register int strlength;   /* IN - size of input buffer */
{

    /* ASN.1 octet string ::= primstring | cmpdstring
     * primstring ::= 0x04 asnlength byte {byte}*
     * cmpdstring ::= 0x24 asnlength string {string}*
     * This code will never send a compound string.
     */
    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL)
        return NULL;
    if (*datalength < strlength)
        return NULL;
    memcpy((char *)data, (char *)string, strlength);
    *datalength -= strlength;
    return data + strlength;
}


/*
 * asn_parse_header - interprets the ID and length of the current object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_header(data, datalength, type)
    unsigned char *data;  /* IN - pointer to start of object */
    int *datalength;      /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;  /* OUT - ASN type of object */
{
    register unsigned char *bufp = data;
    register unsigned long header_len;
    unsigned long       asn_length;

    /* this only works on data types < 30, i.e. no extension octets */
    if (IS_EXTENSION_ID(*bufp))
    {
        SNMP_DEBUG("can't process ID >= 30");
        return NULL;
    }
    *type = *bufp;
    bufp = asn_parse_length(bufp + 1, &asn_length);
    if (bufp == NULL)
        return NULL;
    header_len = bufp - data;

    SNMP_DEBUG("data(%p), header_len(%u), asn_length(%d), *datalength(%d), *type(0x%X)",
               data, header_len, asn_length, *datalength, *type);

    if ((int)header_len + (int)asn_length > *datalength)
    {
        SNMP_DEBUG("asn length too long");
        return NULL;
    }
    *datalength = (int)asn_length;

    return bufp;
}

/*
 * asn_build_header - builds an ASN header for an object with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_header(data, datalength, type, length)
    register unsigned char *data; /* IN - pointer to start of object */
    int *datalength;    /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type; /* IN - ASN type of object */
    int length;         /* IN - length of object */
{
    if (*datalength < 1)
        return NULL;
    *data++ = type;
    (*datalength)--;
    return asn_build_length(data, datalength, length);
}

/*
 * asn_build_sequence - builds an ASN header for a sequence with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_sequence(data, datalength, type, length)
    register unsigned char *data; /* IN - pointer to start of object */
    int *datalength;    /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type; /* IN - ASN type of object */
    int length;         /* IN - length of object */
{
    *datalength -= 4;
    if (*datalength < 0)
    {
        *datalength += 4;   /* fix up before punting */
        return NULL;
    }
    *data++ = type;
    *data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
    *data++ = (unsigned char)((length >> 8) & 0xFF);
    *data++ = (unsigned char)(length & 0xFF);
    return data;
}

/*
 * asn_parse_length - interprets the length of the current object.
 *  On exit, length contains the value of this length field.
 *
 *  Returns a pointer to the first byte after this length
 *  field (aka: the start of the data field).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_length(data, datalength)
    unsigned char *data;       /* IN - pointer to start of length field */
    unsigned long *datalength; /* OUT - value of length field */
{
    register unsigned char lengthbyte = *data;
    int i;

    if (lengthbyte & ASN_LONG_LEN)
    {
        lengthbyte &= ~ASN_LONG_LEN;    /* turn MSb off */
        if (lengthbyte == 0)
        {
            SNMP_DEBUG("We don't support indefinite lengths");
            return NULL;
        }
        if (lengthbyte > sizeof(long))
        {
            SNMP_DEBUG("we can't support data lengths that long");
            return NULL;
        }
        *datalength = 0;
        for (i = 0; i < lengthbyte; i++)
        {
           *datalength = data[i + 1] | (*datalength << (8 * i));
        }
        return data + lengthbyte + 1;
    }
    else
    {
        /* short asnlength */
        *datalength = (long)lengthbyte;
        return data + 1;
    }
}

unsigned char *
asn_build_length(data, datalength, length)
    register unsigned char *data;   /* IN - pointer to start of object */
    int *datalength;     /* IN/OUT - number of valid bytes left in buffer */
    register int length; /* IN - length of object */
{
    unsigned char    *start_data = data;

    /* no indefinite lengths sent */
    if (length < 0x80)
    {
        if (*datalength < 1)
        {
            SNMP_DEBUG("build_length");
            return NULL;
        }
        *data++ = (unsigned char)length;
    }
    else
        if (length <= 0xFF)
        {
            if (*datalength < 2)
            {
                SNMP_DEBUG("build_length");
                return NULL;
            }
            *data++ = (unsigned char)(0x01 | ASN_LONG_LEN);
            *data++ = (unsigned char)length;
        }
        else
        {
            /* 0xFF < length <= 0xFFFF */
            if (*datalength < 3)
            {
                SNMP_DEBUG("build_length");
                return NULL;
            }
            *data++ = (unsigned char)(0x02 | ASN_LONG_LEN);
            *data++ = (unsigned char)((length >> 8) & 0xFF);
            *data++ = (unsigned char)(length & 0xFF);
        }
    *datalength -= (data - start_data);
    return data;
}

/*
 * asn_parse_objid - pulls an object indentifier out of an ASN object identifier type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "objid" is filled with the object identifier.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_objid(data, datalength, type, objid, objidlength)
    unsigned char *data;  /* IN - pointer to start of object */
    int *datalength;      /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;  /* OUT - ASN type of object */
    oid *objid;           /* IN/OUT - pointer to start of output buffer */
    int *objidlength;     /* IN/OUT - number of sub-id's in objid */
{

    /* ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
     * subidentifier ::= {leadingbyte}* lastbyte
     * leadingbyte ::= 1 7bitvalue
     * lastbyte ::= 0 7bitvalue
     */
    register unsigned char *bufp = data;
    register oid *oidp = objid + 1;
    register unsigned long subidentifier;
    register long   length;
    unsigned long       asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return NULL;
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);

    /* Handle invalid object identifier encodings of the form 06 00 robustly */
    if (asn_length == 0)
        objid[0] = objid[1] = 0;
    length = asn_length;
    (*objidlength)--;   /* account for expansion of first byte */
    while (length > 0 && (*objidlength)-- > 0)
    {
        subidentifier = 0;
        do
        {
            /* shift and add in low order 7 bits */
            subidentifier =
                (subidentifier << 7) + (*(unsigned char *)bufp & ~ASN_BIT8);
            length--;
        } while (*(unsigned char *)bufp++ & ASN_BIT8);  /* last byte has high bit clear */
        if (subidentifier > (unsigned long)MAX_SUBID)
        {
            SNMP_DEBUG("subidentifier too long");
            return NULL;
        }
        *oidp++ = (oid)subidentifier;
    }

    /* The first two subidentifiers are encoded into the first component
     * with the value (X * 40) + Y, where:
     *  X is the value of the first subidentifier.
     *  Y is the value of the second subidentifier.
     */
    subidentifier = (unsigned long)objid[1];
    if (subidentifier == 0x2B)
    {
        objid[0] = 1;
        objid[1] = 3;
    }
    else
    {
        objid[1] = (unsigned char)(subidentifier % 40);
        objid[0] = (unsigned char)((subidentifier - objid[1]) / 40);
    }
    *objidlength = (int)(oidp - objid);

    SNMP_DEBUG("bufp(%p), *type(0x%X)", bufp, *type);

    return bufp;
}

/*
 * asn_build_objid - Builds an ASN object identifier object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_objid(data, datalength, type, objid, objidlength)
    register unsigned char *data; /* IN - pointer to start of object */
    int *datalength;    /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type; /* IN - ASN type of object */
    oid *objid;         /* IN - pointer to start of input buffer */
    int objidlength;    /* IN - number of sub-id's in objid */
{

    /* ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
     * subidentifier ::= {leadingbyte}* lastbyte
     * leadingbyte ::= 1 7bitvalue
     * lastbyte ::= 0 7bitvalue
     */
    unsigned char buf[MAX_OID_LEN];
    register unsigned char *bp = buf;
    register oid *op = objid;
    int    asnlength;
    register unsigned long subid, mask, testmask;
    register int bits, testbits;

    if (objidlength < 2)
    {
        *bp++ = 0;
        objidlength = 0;
    }
    else
    {
        *bp++ = (unsigned char) (op[1] + (op[0] * 40));
        objidlength -= 2;
        op += 2;
    }
    while(objidlength-- > 0)
    {
        subid = *op++;
        if (subid < 127)
        {
            /* off by one? */
            *bp++ = (unsigned char) subid;
        }
        else
        {
            mask = 0x7F; /* handle subid == 0 case */
            bits = 0;

            /* testmask *MUST* !!!! be of an unsigned type */
            for(testmask = 0x7F, testbits = 0; testmask != 0;
                testmask <<= 7, testbits += 7)
            {
                if (subid & testmask)
                {
                    /* if any bits set */
                    mask = testmask;
                    bits = testbits;
                }
            }

            /* mask can't be zero here */
            for(;mask != 0x7F; mask >>= 7, bits -= 7)
            {

                /* fix a mask that got truncated above */
                if (mask == 0x1E00000)
                    mask = 0xFE00000;
                *bp++ = (unsigned char)(((subid & mask) >> bits) | ASN_BIT8);
            }
            *bp++ = (unsigned char)(subid & mask);
        }
    }
    asnlength = bp - buf;
    data = asn_build_header(data, datalength, type, asnlength);
    if (data == NULL)
        return NULL;
    if (*datalength < asnlength)
        return NULL;
    memcpy((char *)data, (char *)buf, asnlength);
    *datalength -= asnlength;
    return data + asnlength;
}

/*
 * asn_parse_null - Interprets an ASN null type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_null(data, datalength, type)
    unsigned char *data; /* IN - pointer to start of object */
    int *datalength;     /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type; /* OUT - ASN type of object */
{

    /* ASN.1 null ::= 0x05 0x00 */
    register unsigned char   *bufp = data;
    unsigned long       asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return NULL;
    if (asn_length != 0)
    {
        SNMP_DEBUG("Malformed NULL");
        return NULL;
    }
    *datalength -= (bufp - data);
    return bufp + asn_length;
}


/*
 * asn_build_null - Builds an ASN null object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_null(data, datalength, type)
    unsigned char *data; /* IN - pointer to start of object */
    int *datalength;     /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;  /* IN - ASN type of object */
{

    /* ASN.1 null ::= 0x05 0x00 */
    return asn_build_header(data, datalength, type, 0);
}

/*
 * asn_parse_bitstring - pulls a bitstring out of an ASN bitstring type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the bit string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_bitstring(data, datalength, type, string, strlength)
    unsigned char *data;      /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;      /* OUT - asn type of object */
    unsigned char *string;    /* IN/OUT - pointer to start of output buffer */
    register int *strlength;  /* IN/OUT - size of output buffer */
{

    /* bitstring ::= 0x03 asnlength unused {byte}* */
    register unsigned char *bufp = data;
    unsigned long       asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
        return NULL;
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    if ((int)asn_length > *strlength)
    {
        SNMP_DEBUG("I don't support such long bitstrings");
        return NULL;
    }
    if (asn_length < 1)
    {
        SNMP_DEBUG("Invalid bitstring");
        return NULL;
    }

#if 0 /* Comment out so IP Address is parsed successfully. */
    /* Someone mention this should be '(*bufp > 7)' instaed of */
    /* '(*bufp < 0 || *bufp > 7)'. Harrie doesn't know why?? */
    if ((char) *bufp < 0 || (char) *bufp > 7)
    {
        SNMP_DEBUG("Invalid bitstring");
        return NULL;
    }
#endif
    memcpy((char *)string, (char *)bufp, (int)asn_length);
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);
    return bufp + asn_length;
}


/*
 * asn_build_bitstring - Builds an ASN bit string object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_bitstring(data, datalength, type, string, strlength)
    unsigned char *data;      /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;       /* IN - ASN type of string */
    unsigned char *string;    /* IN - pointer to start of input buffer */
    register int strlength;   /* IN - size of input buffer */
{

    /* ASN.1 bit string ::= 0x03 asnlength unused {byte}* */

    /* someone mentioned that '(strlength < 1 || *string < 0 || *string > 7)' */

    /* should be '(strlength < 1 || *string > 7)' Harrie doesn't know why ?? */
    if (strlength < 1 || (char) *string < 0 || (char) *string > 7)
    {
        SNMP_DEBUG("Building invalid bitstring");
        return NULL;
    }
    data = asn_build_header(data, datalength, type, strlength);
    if (data == NULL)
        return NULL;
    if (*datalength < strlength)
        return NULL;
    memcpy((char *)data, (char *)string, strlength);
    *datalength -= strlength;
    return data + strlength;
}


/*
 * asn_parse_unsigned_int64 - pulls a 64 bit unsigned long out of an ASN int
 * type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_parse_unsigned_int64(data, datalength, type, cp, countersize)
    register unsigned char *data;  /* IN - pointer to start of object */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char *type;      /* OUT - asn type of object */
    Counter64 *cp;            /* IN/OUT -pointer to counter struct */
    int countersize;          /* IN - size of output buffer */
{

    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register unsigned char *bufp = data;
    unsigned long       asn_length;
    register unsigned long low = 0, high = 0;
    int intsize = 4;

    if (countersize != sizeof(Counter64))
    {
        SNMP_DEBUG("not right size");
        return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL)
    {
        SNMP_DEBUG("bad length");
        return NULL;
    }
    if ((int)asn_length + (bufp - data) > *datalength)
    {
        SNMP_DEBUG("overflow of message");
        return NULL;
    }
    if (((int)asn_length > (intsize * 2 + 1)) ||
        (((int)asn_length == (intsize * 2) + 1) && *bufp != 0x00))
    {
        SNMP_DEBUG("I don't support such large integers");
        return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
    {
        low = -1; /* integer is negative */
        high = -1;
    }
    while(asn_length--)
    {
        high = (high << 8) | ((low & 0xFF000000) >> 24);
        low = (low << 8) | *bufp++;
    }
    cp->low = low;
    cp->high = high;
    return bufp;
}


/*
 * asn_build_unsigned_int64 - builds an ASN object containing a 64 bit integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 */
unsigned char *
asn_build_unsigned_int64(data, datalength, type, cp, countersize)
    register unsigned char *data;   /* IN - pointer to start of output buffer */
    register int *datalength; /* IN/OUT - number of valid bytes left in buffer */
    unsigned char type;       /* IN - asn type of object */
    Counter64 *cp;            /* IN - pointer to counter struct */
    register int countersize; /* IN - size of *intp */
{

    /* ASN.1 integer ::= 0x02 asnlength byte {byte}* */
    register unsigned long low, high;
    register unsigned long mask, mask2;
    int add_null_byte = 0;
    int intsize;

    if (countersize != sizeof (Counter64))
        return NULL;
    intsize = 8;
    low = cp->low;
    high = cp->high;
    mask = 0xFF << (8 * (sizeof(long) - 1));

    /* mask is 0xFF000000 on a big-endian machine */
    if ((unsigned char)((high & mask) >> (8 * (sizeof(long) - 1))) & 0x80)
    {

        /* if MSB is set */
        add_null_byte = 1;
        intsize++;
    }

    /* Truncate "unnecessary" bytes off of the most significant end of this 2's
     * complement integer.
     * There should be no sequence of 9 consecutive 1's or 0's at the most
     * significant end of the integer.
     */
    mask2 = 0x1FF << ((8 * (sizeof(long) - 1)) - 1);

    /* mask2 is 0xFF800000 on a big-endian machine */
    while((((high & mask2) == 0) || ((high & mask2) == mask2))
        && intsize > 1)
    {
        intsize--;
        high = (high << 8) | ((low & mask) >> (8 * (sizeof(long) - 1)));
        low <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (data == NULL)
        return NULL;
    if (*datalength < intsize)
        return NULL;
    *datalength -= intsize;
    if (add_null_byte == 1)
    {
        *data++ = '\0';
        intsize--;
    }
    while(intsize--)
    {
        *data++ = (unsigned char)((high & mask) >> (8 * (sizeof(long) - 1)));
        high = (high << 8) | ((low & mask) >> (8 * (sizeof(long) - 1)));
        low <<= 8;
    }
    return data;
}

/* parse ASN objId data into oid[] */
void asn_parse_subidentifier(data, datalength, objid, objidlength)
    unsigned char *data;  /* IN - pointer to start of subidentifier (objid type and asnlength had been parsed) */
    int datalength;       /* IN - length of data */
    oid *objid;           /* IN/OUT - pointer to start of output buffer */
    int *objidlength;     /* IN/OUT - number of sub-id's in objid */
{

    /* ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
     * subidentifier ::= {leadingbyte}* lastbyte
     * leadingbyte ::= 1 7bitvalue
     * lastbyte ::= 0 7bitvalue
     */
    register unsigned char *bufp = data;
    register oid *oidp = objid + 1;
    register unsigned long subidentifier;
    register long   length;

    if (datalength == 0) 
      return;

    length = datalength;

    while (length > 0)
    {
        subidentifier = 0;
        do
        {
            /* shift and add in low order 7 bits */
            subidentifier =
                (subidentifier << 7) + (*(unsigned char *)bufp & ~ASN_BIT8);
            length--;
        } while (*(unsigned char *)bufp++ & ASN_BIT8);  /* last byte has high bit clear */
        if (subidentifier > (unsigned long)MAX_SUBID)
        {
            SNMP_DEBUG("subidentifier too long");
            return;
        }
        *oidp++ = (oid)subidentifier;
    } /* while length */

    /* The first two subidentifiers are encoded into the first component
     * with the value (X * 40) + Y, where:
     *  X is the value of the first subidentifier.
     *  Y is the value of the second subidentifier.
     */
    subidentifier = (unsigned long)objid[1];

    if (subidentifier == 0x2B)
    {
        objid[0] = 1;
        objid[1] = 3;
    }
    else
    {
        objid[1] = (unsigned char)(subidentifier % 40);
        objid[0] = (unsigned char)((subidentifier - objid[1]) / 40);
    }
    *objidlength = (int)(oidp - objid);
}

