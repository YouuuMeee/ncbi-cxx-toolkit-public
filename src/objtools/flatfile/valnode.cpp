/* $Id$
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * File Name: valnode.cpp
 *
 * Author: Alexey Dobronadezhdin
 *
 * File Description:
 *      ValNode functionality from C-toolkit.
 */

#include <ncbi_pch.hpp>

#include "ftacpp.hpp"
#include "ftaerr.hpp"
#include "valnode.h"

#ifdef THIS_FILE
#    undef THIS_FILE
#endif
#define THIS_FILE "valnode.cpp"


BEGIN_NCBI_SCOPE
/*****************************************************************************
 *
 *   ValNodeNew(vnp)
 *      adds after last node in list if vnp not NULL
 *
 *****************************************************************************/
ValNodePtr ValNodeNew(ValNodePtr vnp)
{
    ValNodePtr newnode;

    newnode = new ValNode;
    if (vnp != NULL) {
        while (vnp->next != NULL)
            vnp = vnp->next;
        vnp->next = newnode;
    }
    return newnode;
}

/*****************************************************************************
 *
 *   ValNodeFree(vnp)
 *      frees whole chain of ValNodes
 *       Does NOT free associated data pointers
 *           see ValNodeFreeData()
 *
 *****************************************************************************/
ValNodePtr ValNodeFree(ValNodePtr vnp)
{
    ValNodePtr next;

    while (vnp != NULL) {
        next = vnp->next;
        delete vnp;
        vnp = next;
    }
    return NULL;
}

/*****************************************************************************
 *
 *   ValNodeFreeData(vnp)
 *      frees whole chain of ValNodes
 *       frees associated data pointers - BEWARE of this if these are not
 *           allocated single memory block structures.
 *
 *****************************************************************************/
ValNodePtr ValNodeFreeData(ValNodePtr vnp)
{
    ValNodePtr next;

    while (vnp != NULL) {
        MemFree((char*)vnp->data.ptrvalue);
        next = vnp->next;
        delete vnp;
        vnp = next;
    }
    return NULL;
}

/*****************************************************************************
 *
 *   ValNodeLink(head, newnode)
 *      adds newnode at end of chain
 *      if (*head == NULL) *head = newnode
 *      ALWAYS returns pointer to START of chain
 *
 *****************************************************************************/
ValNodePtr ValNodeLink(ValNodePtr* head, ValNodePtr newnode)
{
    ValNodePtr vnp;

    if (head == NULL)
        return newnode;

    vnp = *head;

    if (vnp != NULL) {
        while (vnp->next != NULL)
            vnp = vnp->next;
        vnp->next = newnode;
    } else
        *head = newnode;

    return *head;
}


static ValNodePtr ValNodeCopyStrExEx(ValNodePtr* head, ValNodePtr* tail, short choice, const char* str, const char* pfx, const char* sfx)
{
    size_t     len, pfx_len, sfx_len, str_len;
    ValNodePtr newnode = NULL, vnp;
    char*      ptr;
    string     tmp;

    if (str == NULL)
        return NULL;

    newnode = ValNodeNew(NULL);
    if (newnode == NULL)
        return NULL;

    str_len = StringLen(str);
    pfx_len = StringLen(pfx);
    sfx_len = StringLen(sfx);

    len = str_len + pfx_len + sfx_len;

    if (head != NULL) {
        if (*head == NULL) {
            *head = newnode;
        }
    }

    if (tail != NULL) {
        if (*tail != NULL) {
            vnp = *tail;
            while (vnp->next != NULL) {
                vnp = vnp->next;
            }
            vnp->next = newnode;
        }
        *tail = newnode;
    }

    ptr = MemNew(len + 2);
    if (ptr == NULL)
        return NULL;

    tmp.reserve(len);
    if (pfx_len > 0) {
        tmp.append(pfx);
    }
    if (str_len > 0) {
        tmp.append(str);
    }
    if (sfx_len > 0) {
        tmp.append(sfx);
    }

    StringCpy(ptr, tmp.c_str());

    if (newnode != NULL) {
        newnode->choice        = (unsigned char)choice;
        newnode->data.ptrvalue = ptr;
    }

    return newnode;
}

ValNodePtr ValNodeCopyStrEx(ValNodePtr* head, ValNodePtr* tail, short choice, const char* str)
{
    return ValNodeCopyStrExEx(head, tail, choice, str, NULL, NULL);
}

static char* ValNodeMergeStrsExEx(ValNodePtr list, char* separator, char* pfx, char* sfx)
{
    size_t     len;
    size_t     lens;
    size_t     pfx_len;
    char*      ptr;
    char*      sep;
    size_t     sfx_len;
    char*      str;
    string     tmp;
    ValNodePtr vnp;

    if (list == NULL)
        return NULL;

    pfx_len = StringLen(pfx);
    sfx_len = StringLen(sfx);

    lens = StringLen(separator);

    for (vnp = list, len = 0; vnp != NULL; vnp = vnp->next) {
        str = (char*)vnp->data.ptrvalue;
        len += StringLen(str);
        len += lens;
    }
    if (len == 0)
        return NULL;
    len += pfx_len + sfx_len;

    ptr = MemNew(len + 2);
    if (ptr == NULL)
        return NULL;

    tmp.reserve(len);
    if (pfx_len > 0) {
        tmp.append(pfx);
    }
    sep = NULL;
    for (vnp = list; vnp != NULL; vnp = vnp->next) {
        if (sep) {
            tmp.append(sep);
        }
        str = (char*)vnp->data.ptrvalue;
        tmp.append(str);
        sep = separator;
    }
    if (sfx_len > 0) {
        tmp.append(sfx);
    }

    StringCpy(ptr, tmp.c_str());

    return ptr;
}


char* ValNodeMergeStrsEx(ValNodePtr list, char* separator)
{
    return ValNodeMergeStrsExEx(list, separator, NULL, NULL);
}

END_NCBI_SCOPE
