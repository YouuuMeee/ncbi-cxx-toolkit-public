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
 * File Name: block.cpp
 *
 * Author: Karl Sirotkin, Hsiu-Chuan Chen
 *
 * File Description:
 *      Parsing flatfile to blocks in memory.
 *
 */

#include <ncbi_pch.hpp>

#include "ftacpp.hpp"

#include "ftaerr.hpp"
#include "ftablock.h"
#include "indx_blk.h"
#include "indx_def.h"
#include "utilfun.h"

#ifdef THIS_FILE
#    undef THIS_FILE
#endif
#define THIS_FILE "block.cpp"
BEGIN_NCBI_SCOPE

typedef struct _qs_struct {
    char*              accession;
    Int2               version;
    size_t             offset;
    size_t             length;
    struct _qs_struct* next;
} QSStruct, *QSStructPtr;

/**********************************************************/
void GapFeatsFree(GapFeatsPtr gfp)
{
    GapFeatsPtr tgfp;

    for (; gfp; gfp = tgfp) {
        tgfp = gfp->next;
        delete gfp;
    }
}

DataBlk::~DataBlk()
{
    int MAX_HEAD_RECURSION(100);

    mpQscore.clear();
    delete mpData;
    if (mType == ParFlat_ENTRYNODE) {
        delete[] mOffset;
    }
    auto p = mpNext;
    for (int i = 0; p && i < MAX_HEAD_RECURSION; ++i) {
        p = p->mpNext;
    }
    if (! p) {
        delete mpNext;
    } else {
        auto pTail = p->mpNext;
        p->mpNext  = nullptr;
        delete mpNext;
        delete pTail;
    }
}

/**********************************************************
 *
 *   void FreeEntry(entry):
 *
 *      Only free entry itself and ebp->chain data because
 *   ebp->sep has to be used to write out ASN.1 output then
 *   free ebp->sep and ebp itself together.
 *
 *                                              5-12-93
 *
 **********************************************************/

void xFreeEntry(DataBlkPtr entry)
{
    if (entry->mpData) {
        delete entry->mpData;
        entry->mpData = nullptr;
    }

    delete entry;
}

/**********************************************************/

EntryBlk::~EntryBlk()
{
    if (chain) {
        delete chain;
        chain = nullptr;
    }
}

/**********************************************************/
void XMLIndexFree(XmlIndexPtr xip)
{
    XmlIndexPtr xipnext;

    for (; xip != NULL; xip = xipnext) {
        xipnext = xip->next;
        if (xip->subtags != NULL)
            XMLIndexFree(xip->subtags);
        MemFree(xip);
    }
}

/**********************************************************/
void FreeIndexblk(IndexblkPtr ibp)
{
    if (ibp == NULL)
        return;

    if (ibp->gaps != NULL)
        GapFeatsFree(ibp->gaps);

    if (ibp->secaccs != NULL)
        FreeTokenblk(ibp->secaccs);

    if (ibp->xip != NULL)
        XMLIndexFree(ibp->xip);

    delete ibp;
}

/**********************************************************/
static bool AccsCmp(const Indexblk* ibp1, const Indexblk* ibp2)
{
    int i = StringCmp(ibp1->acnum, ibp2->acnum);
    if (i != 0)
        return i < 0;

    if (ibp1->vernum != ibp2->vernum)
        return ibp1->vernum < ibp2->vernum;

    return ibp2->offset < ibp1->offset;
}

/**********************************************************/
static bool QSCmp(const QSStruct* qs1, const QSStruct* qs2)
{
    int i = StringCmp(qs1->accession, qs2->accession);
    if (i != 0)
        return i < 0;

    return qs1->version < qs2->version;
}

/**********************************************************/
static void QSStructFree(QSStructPtr qssp)
{
    QSStructPtr tqssp;

    for (; qssp != NULL; qssp = tqssp) {
        tqssp = qssp->next;
        if (qssp->accession != NULL)
            MemFree(qssp->accession);
        MemFree(qssp);
    }
}

/**********************************************************/
static bool QSNoSequenceRecordErr(bool accver, QSStructPtr qssp)
{
    if (accver)
        ErrPostEx(SEV_FATAL, ERR_QSCORE_NoSequenceRecord, "Encountered Quality Score data for a record \"%s.%d\" that does not exist in the file of sequence records being parsed.", qssp->accession, qssp->version);
    else
        ErrPostEx(SEV_FATAL, ERR_QSCORE_NoSequenceRecord, "Encountered Quality Score data for a record \"%s\" that does not exist in the file of sequence records being parsed.", qssp->accession);
    return false;
}

/**********************************************************/
bool QSIndex(ParserPtr pp, IndBlkNextPtr ibnp)
{
    QSStructPtr  qssp;
    QSStructPtr  tqssp;
    QSStructPtr  tqsspprev;
    char*        p;
    char*        q;
    bool         ret;
    size_t       i;
    Int4         count;
    Int4         j;
    Int4         k;
    Int4         l;
    Int2         m;
    Char         buf[1024];

    if (pp->qsfd == NULL)
        return true;

    qssp      = (QSStructPtr)MemNew(sizeof(QSStruct));
    tqssp     = qssp;
    tqsspprev = NULL;
    count     = 0;
    while (fgets(buf, 1023, pp->qsfd) != NULL) {
        if (buf[0] != '>')
            continue;

        p = StringChr(buf, ' ');
        if (p == NULL)
            continue;

        i  = (size_t)StringLen(buf);
        *p = '\0';

        q = StringChr(buf, '.');
        if (q != NULL)
            *q++ = '\0';

        count++;
        tqssp->next      = (QSStructPtr)MemNew(sizeof(QSStruct));
        tqssp            = tqssp->next;
        tqssp->accession = StringSave(buf + 1);
        tqssp->version   = (q == NULL) ? 0 : atoi(q);
        tqssp->offset    = (size_t)ftell(pp->qsfd) - i;
        if (tqsspprev != NULL)
            tqsspprev->length = tqssp->offset - tqsspprev->offset;
        tqssp->next = NULL;

        tqsspprev = tqssp;
    }
    tqssp->length = (size_t)ftell(pp->qsfd) - tqssp->offset;

    tqssp = qssp;
    qssp  = tqssp->next;
    MemFree(tqssp);

    if (qssp == NULL) {
        ErrPostEx(SEV_FATAL, ERR_QSCORE_NoScoreDataFound, "No correctly formatted records containing quality score data were found within file \"%s\".", pp->qsfile);
        return false;
    }

    vector<QSStructPtr> qsspp(count);
    tqssp = qssp;
    for (j = 0; j < count && tqssp; j++, tqssp = tqssp->next)
        qsspp[j] = tqssp;

    if (count > 1) {
        std::sort(qsspp.begin(), qsspp.end(), QSCmp);

        for (j = 0, count--; j < count; j++)
            if (StringCmp(qsspp[j]->accession, qsspp[j + 1]->accession) == 0)
                if (pp->accver == false ||
                    qsspp[j]->version == qsspp[j + 1]->version)
                    break;

        if (j < count) {
            if (pp->accver)
                ErrPostEx(SEV_FATAL, ERR_QSCORE_RedundantScores, "Found more than one set of Quality Score for accession \"%s.%d\".", qsspp[j]->accession, qsspp[j]->version);
            else
                ErrPostEx(SEV_FATAL, ERR_QSCORE_RedundantScores, "Found more than one set of Quality Score for accession \"%s\".", qsspp[j]->accession);

            QSStructFree(qssp);
            return false;
        }
        count++;
    }

    vector<IndexblkPtr> ibpp(pp->indx);
    for (j = 0; j < pp->indx && ibnp; j++, ibnp = ibnp->next)
        ibpp[j] = ibnp->ibp;

    if (pp->indx > 1)
        std::sort(ibpp.begin(), ibpp.end(), AccsCmp);

    for (ret = true, j = 0, k = 0; j < count; j++) {
        if (k == pp->indx) {
            ret = QSNoSequenceRecordErr(pp->accver, qsspp[j]);
            continue;
        }
        for (; k < pp->indx; k++) {
            l = StringCmp(qsspp[j]->accession, ibpp[k]->acnum);
            if (l < 0) {
                ret = QSNoSequenceRecordErr(pp->accver, qsspp[j]);
                break;
            }
            if (l > 0)
                continue;
            m = qsspp[j]->version - ibpp[k]->vernum;
            if (m < 0) {
                ret = QSNoSequenceRecordErr(pp->accver, qsspp[j]);
                break;
            }
            if (m > 0)
                continue;
            ibpp[k]->qsoffset = qsspp[j]->offset;
            ibpp[k]->qslength = qsspp[j]->length;
            k++;
            break;
        }
    }

    QSStructFree(qssp);

    return (ret);
}

END_NCBI_SCOPE
