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
 * File Name: asci_blk.cpp
 *
 * Author: Karl Sirotkin, Hsiu-Chuan Chen
 *
 * File Description:
 *      Common for all formats function processing ascii blocks to asn.
 *
 */

#include <ncbi_pch.hpp>

#include <set>

#include "ftacpp.hpp"

#include <objects/biblio/Id_pat.hpp>
#include <objects/biblio/Id_pat_.hpp>
#include <objects/seqloc/Textseq_id.hpp>
#include <objects/seqloc/PDB_seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seg_ext.hpp>
#include <objects/general/Int_fuzz.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/general/Date.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seq/Numbering.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seq/seqport_util.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <serial/iterator.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqblock/EMBL_block.hpp>
#include <objects/seq/MolInfo.hpp>

#include "index.h"
#include "genbank.h"
#include "embl.h"
#include "sprot.h"

#include <objtools/flatfile/flatdefn.h>

#include "ftaerr.hpp"
#include "indx_blk.h"
#include "asci_blk.h"
#include "utilfun.h"
#include "fta_xml.h"

#include "add.h"

#ifdef THIS_FILE
#    undef THIS_FILE
#endif
#define THIS_FILE "asci_blk.cpp"

#define Seq_descr_pub_same 50

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const char* magic_phrases[] = {
    "*** SEQUENCING IN PROGRESS ***",
    "***SEQUENCING IN PROGRESS***",
    "WORKING DRAFT SEQUENCE",
    "LOW-PASS SEQUENCE SAMPLING",
    "*** IN PROGRESS ***",
    nullptr
};

extern vector<string> genbankKeywords;
extern vector<string> emblKeywords;
extern vector<string> swissProtKeywords;

/**********************************************************/
void ShrinkSpaces(char* line)
{
    char* p;
    char* q;
    bool  got_nl;

    if (! line || *line == '\0')
        return;

    for (p = line; *p != '\0'; p++) {
        if (*p == '\t')
            *p = ' ';
        if ((*p == ',' && p[1] == ',') || (*p == ';' && p[1] == ';'))
            p[1] = ' ';
        if ((p[1] == ',' || p[1] == ';') && p[0] == ' ') {
            p[0] = p[1];
            p[1] = ' ';
        }
    }

    for (p = line, q = line; *p != '\0';) {
        *q = *p;
        if (*p == ' ' || *p == '\n') {
            for (got_nl = false; *p == ' ' || *p == '\n'; p++) {
                if (*p == '\n')
                    got_nl = true;
            }

            if (got_nl)
                *q = '\n';
        } else
            p++;
        q++;
    }
    if (q > line) {
        for (q--; q > line && (*q == ' ' || *q == ';' || *q == '\n');)
            q--;
        if (*q != ' ' && *q != ';' && *q != '\n')
            q++;
    }
    *q = '\0';

    for (p = line; *p == ' ' || *p == ';' || *p == '\n';)
        p++;
    if (p > line)
        fta_StringCpy(line, p);
}

/**********************************************************
 *
 *   static void InsertDatablkVal(dbp, type, offset, len):
 *
 *      Allocate a memory, then assign data-block value
 *   to a new node.
 *      dbp points to the new node if dbp is NULL.
 *
 *                                              3-18-93
 *
 **********************************************************/
static void InsertDatablkVal(DataBlkPtr* dbp, Int2 type, char* offset, size_t len)
{
    DataBlk* ldp = new DataBlk(*dbp, type, offset, len);
    if (! *dbp) {
        *dbp = ldp;
    }
}

/**********************************************************
 *
 *   char* GetGenBankBlock(chain, ptr, retkw, eptr):
 *
 *      Enters knowing current keyword.type and offset,
 *   finds the length of the current keyword block,
 *   and builds the block to "chain".
 *      Since each key-word block always start at first
 *   column of the line, the loop stops when it found the
 *   first none (blank, newline, or tab) character after
 *   the newline character.
 *      Each data block will append to the "chain".
 *      Return a pointer points to next key-word block.
 *
 *                                              3-21-93
 *
 **********************************************************/
void xGetGenBankBlocks(Entry& entry)
{
    vector<string> lines;
    NStr::Split(entry.mBaseData, "\n", lines);

    vector<string> sectionLines;
    int            currentKw = ParFlat_LOCUS;
    int            nextKw;
    string         sectionText;
    for (const string& line : lines) {
        nextKw = SrchKeyword(line, genbankKeywords);
        if (nextKw == ParFlat_UNKW) {
            nextKw = currentKw;
        }
        if (nextKw != currentKw || NStr::StartsWith(line, "REFERENCE")) {
            auto* secPtr = new Section(currentKw, sectionLines);
            // secPtr->DumpText(cerr);
            entry.mSections.push_back(secPtr);
            currentKw = nextKw;
            sectionLines.clear();
            sectionLines.push_back(line);
            continue;
        }
        sectionLines.push_back(line);
    }
    entry.mSections.push_back(new Section(currentKw, sectionLines));
}

char* GetGenBankBlock(DataBlkPtr* chain, char* ptr, Int2* retkw, char* eptr)
{
    char* offset;
    int   curkw;
    int   nextkw;
    Int4  len;

    len    = 0;
    offset = ptr;
    curkw  = *retkw;

    do /* repeat loop until it finds next key-word */
    {
        for (; ptr < eptr && *ptr != '\n'; ptr++)
            len++;
        if (ptr >= eptr)
            return (ptr);

        ++ptr; /* newline character */
        ++len;

        nextkw = SrchKeyword(CTempString(ptr, eptr - ptr), genbankKeywords);
        if (nextkw == ParFlat_UNKW) /* it can be "XX" line,
                                            treat as same line */
            nextkw = curkw;

        if (StringEquN(ptr, "REFERENCE", 9)) /* treat as one block */
            break;
    } while (nextkw == curkw);

    nextkw = SrchKeyword(ptr, genbankKeywords);

    InsertDatablkVal(chain, curkw, offset, len);
    *retkw = nextkw;
    return (ptr);
}


/**********************************************************
 *
 *   static void GetGenBankRefType(dbp, bases):
 *
 *      Check the data in the "REFERENCE" line,
 *      - ParFlat_REF_END if it contains
 *        "(bases 1 to endbases)", pub for "descr"
 *        or no base range at all;
 *      - ParFlat_REF_SITES if it contains "(sites)",
 *        for ImpFeatPub;
 *      - ParFlat_REF_BTW, otherwise, for SeqFeatPub.
 *
 *                                              5-19-93
 *
 **********************************************************/
static void GetGenBankRefType(DataBlkPtr dbp, size_t bases)
{
    char* bptr;
    char* eptr;

    bptr = dbp->mOffset;
    eptr = bptr + dbp->len;

    const string s    = to_string(bases);
    const string str  = "(bases 1 to " + s + ")";
    const string str1 = "(bases 1 to " + s + ";";
    const string str2 = "(residues 1 to " + s + "aa)";

    string ref(bptr, bptr + dbp->len);

    while (bptr < eptr && *bptr != '\n' && *bptr != '(')
        bptr++;
    while (*bptr == ' ')
        bptr++;

    if (*bptr == '\n')
        dbp->mType = ParFlat_REF_NO_TARGET;
    else if (NStr::Find(ref, str) != NPOS || NStr::Find(ref, str1) != NPOS ||
             NStr::Find(ref, str2) != NPOS)
        dbp->mType = ParFlat_REF_END;
    else if (NStr::Find(ref, "(sites)") != NPOS)
        dbp->mType = ParFlat_REF_SITES;
    else
        dbp->mType = ParFlat_REF_BTW;
}

/**********************************************************
 *
 *   static void BuildFeatureBlock(dbp):
 *
 *      The feature key in column 6-20.
 *
 *                                              5-3-93
 *
 **********************************************************/
static void BuildFeatureBlock(DataBlkPtr dbp)
{
    char* bptr;
    char* eptr;
    char* ptr;
    bool  skip;

    bptr = dbp->mOffset;
    eptr = bptr + dbp->len;
    ptr  = SrchTheChar(bptr, eptr, '\n');

    if (! ptr)
        return;

    bptr = ptr + 1;

    while (bptr < eptr) {
        InsertDatablkVal(reinterpret_cast<DataBlk**>(&dbp->mpData), ParFlat_FEATBLOCK, bptr, eptr - bptr);

        do {
            bptr = SrchTheChar(bptr, eptr, '\n');
            bptr++;

            skip = false;
            if (! StringEquN(bptr, "XX", 2))
                ptr = bptr + ParFlat_COL_FEATKEY;
            else
                skip = true;
        } while ((*ptr == ' ' && ptr < eptr) || skip);
    }
}

/**********************************************************/
static void fta_check_mult_ids(DataBlkPtr dbp, const char* mtag, const char* ptag)
{
    char* p;
    Char  ch;
    Int4  muids;
    Int4  pmids;

    if (! dbp || ! dbp->mOffset || (! mtag && ! ptag))
        return;

    ch                     = dbp->mOffset[dbp->len];
    dbp->mOffset[dbp->len] = '\0';

    size_t mlen = mtag ? StringLen(mtag) : 0;
    size_t plen = ptag ? StringLen(ptag) : 0;

    muids = 0;
    pmids = 0;
    for (p = dbp->mOffset;; p++) {
        p = StringChr(p, '\n');
        if (! p)
            break;
        if (mtag && StringEquN(p + 1, mtag, mlen))
            muids++;
        else if (ptag && StringEquN(p + 1, ptag, plen))
            pmids++;
    }
    dbp->mOffset[dbp->len] = ch;

    if (muids > 1) {
        ErrPostEx(SEV_ERROR, ERR_REFERENCE_MultipleIdentifiers, "Reference has multiple MEDLINE identifiers. Ignoring all but the first.");
    }
    if (pmids > 1) {
        ErrPostEx(SEV_ERROR, ERR_REFERENCE_MultipleIdentifiers, "Reference has multiple PUBMED identifiers. Ignoring all but the first.");
    }
}

/**********************************************************
 *
 *   void GetGenBankSubBlock(entry, bases):
 *
 *                                              4-7-93
 *
 **********************************************************/
void GetGenBankSubBlock(const DataBlk& entry, size_t bases)
{
    DataBlkPtr dbp;

    dbp = TrackNodeType(entry, ParFlat_SOURCE);
    if (dbp) {
        BuildSubBlock(dbp, ParFlat_ORGANISM, "  ORGANISM");
        GetLenSubNode(dbp);
    }

    dbp = TrackNodeType(entry, ParFlat_REFERENCE);
    for (; dbp; dbp = dbp->mpNext) {
        if (dbp->mType != ParFlat_REFERENCE)
            continue;

        fta_check_mult_ids(dbp, "  MEDLINE", "   PUBMED");
        BuildSubBlock(dbp, ParFlat_AUTHORS, "  AUTHORS");
        BuildSubBlock(dbp, ParFlat_CONSRTM, "  CONSRTM");
        BuildSubBlock(dbp, ParFlat_TITLE, "  TITLE");
        BuildSubBlock(dbp, ParFlat_JOURNAL, "  JOURNAL");
        BuildSubBlock(dbp, ParFlat_MEDLINE, "  MEDLINE");
        BuildSubBlock(dbp, ParFlat_PUBMED, "   PUBMED");
        BuildSubBlock(dbp, ParFlat_STANDARD, "  STANDARD");
        BuildSubBlock(dbp, ParFlat_REMARK, "  REMARK");
        GetLenSubNode(dbp);
        GetGenBankRefType(dbp, bases);
    }

    dbp = TrackNodeType(entry, ParFlat_FEATURES);
    for (; dbp; dbp = dbp->mpNext) {
        if (dbp->mType != ParFlat_FEATURES)
            continue;

        BuildFeatureBlock(dbp);
        GetLenSubNode(dbp);
    }
}

//  ----------------------------------------------------------------------------
void xGetGenBankSubBlocks(Entry& entry, size_t bases)
//  ----------------------------------------------------------------------------
{
    for (auto secPtr : entry.mSections) {
        auto secType = secPtr->mType;
        if (secType == ParFlat_SOURCE) {
            secPtr->xBuildSubBlock(ParFlat_ORGANISM, "  ORGANISM");
            // GetLenSubNode(dbp);
        }
        if (secType == ParFlat_REFERENCE) {
            // fta_check_mult_ids(dbp, "  MEDLINE", "   PUBMED");
            secPtr->xBuildSubBlock(ParFlat_AUTHORS, "  AUTHORS");
            secPtr->xBuildSubBlock(ParFlat_CONSRTM, "  CONSRTM");
            secPtr->xBuildSubBlock(ParFlat_TITLE, "  TITLE");
            secPtr->xBuildSubBlock(ParFlat_JOURNAL, "  JOURNAL");
            secPtr->xBuildSubBlock(ParFlat_MEDLINE, "  MEDLINE");
            secPtr->xBuildSubBlock(ParFlat_PUBMED, "   PUBMED");
            secPtr->xBuildSubBlock(ParFlat_STANDARD, "  STANDARD");
            secPtr->xBuildSubBlock(ParFlat_REMARK, "  REMARK");
            // GetLenSubNode(dbp);
            // GetGenBankRefType(dbp, bases);
        }
        if (secType == ParFlat_FEATURES) {
            secPtr->xBuildFeatureBlocks();
            // GetLenSubNode(dbp);
        }
    }
}

/**********************************************************
 *
 *   char* GetEmblBlock(chain, ptr, retkw, format, eptr):
 *
 *      Enters knowing current keyword.type and offset,
 *   finds the length of the current keyword block, and
 *   builds the block to "chain".
 *      Loop will continue until it finds the next keyword
 *   or next "RN" after the newline character.
 *      Each data block will append to the "chain".
 *      Return a pointer points to next key-word block.
 *
 *                                              3-21-93
 *
 *      The OS block can be
 *      - OS OS OC OC XX OG  ==> this normal
 *        or
 *      - OS OC OC XX OS OS OC OC XX OG  ==> this hybrids
 *      For case 2, it need to make two OS block.
 *
 *                                              12-15-93
 *
 **********************************************************/
char* GetEmblBlock(DataBlkPtr* chain, char* ptr, short* retkw, Parser::EFormat format, char* eptr)
{
    char* offset;
    Int2  curkw;
    Int2  nextkw;
    bool  seen_oc = false;

    size_t len = 0;
    offset     = ptr;
    curkw      = *retkw;

    do /* repeat loop until it finds next key-word */
    {
        for (; ptr < eptr && *ptr != '\n'; ptr++)
            len++;
        if (ptr >= eptr) {
            *retkw = ParFlat_END;
            return (ptr);
        }
        ++ptr; /* newline character */
        ++len;

        nextkw = SrchKeyword(
            CTempString(ptr, eptr - ptr),
            (format == Parser::EFormat::SPROT) ? swissProtKeywords : emblKeywords);
        if (nextkw == ParFlat_UNKW) /* it can be "XX" line,
                                            treat as same line */
            nextkw = curkw;
        if (StringEquN(ptr, "RN", 2)) /* treat each RN per block */
            break;
        if (StringEquN(ptr, "ID", 2)) /* treat each ID per block */
            break;

        if (StringEquN(ptr, "OC", 2))
            seen_oc = true;

        if (StringEquN(ptr, "OS", 2) && seen_oc)
            break; /* treat as next OS block */

    } while (nextkw == curkw);

    InsertDatablkVal(chain, curkw, offset, len);

    *retkw = nextkw;
    return (ptr);
}

/**********************************************************
 *
 *   static bool TrimEmblFeatBlk(dbp):
 *
 *      Routine return TRUE if found FT data.
 *      The routine do the following things:
 *      - only leave last one FH line;
 *      - replace all "FT" to "  " in the beginning of line.
 *
 *                                              6-15-93
 *
 **********************************************************/
static bool TrimEmblFeatBlk(DataBlkPtr dbp)
{
    char* bptr;
    char* eptr;
    char* ptr;
    bool  flag = false;

    bptr = dbp->mOffset;
    eptr = bptr + dbp->len;
    ptr  = SrchTheChar(bptr, eptr, '\n');

    while (ptr && ptr + 1 < eptr) {
        if (ptr[2] == 'H') {
            dbp->len     = dbp->len - (ptr - dbp->mOffset + 1);
            dbp->mOffset = ptr + 1;

            bptr = dbp->mOffset;
            eptr = bptr + dbp->len;
        } else {
            bptr = ptr + 1;

            if (bptr[1] == 'T') {
                flag    = true;
                *bptr   = ' ';
                bptr[1] = ' ';
            }
        }

        ptr = SrchTheChar(bptr, eptr, '\n');
    }

    return (flag);
}

/**********************************************************
 *
 *   static bool GetSubNodeType(subkw, retbptr, eptr):
 *
 *      Return TRUE and memory location which has
 *   the "subkw".
 *
 *                                              6-15-93
 *
 **********************************************************/
static bool GetSubNodeType(const char* subkw, char** retbptr, char* eptr)
{
    char* bptr;
    char* ptr;

    bptr          = *retbptr;
    size_t sublen = StringLen(subkw);

    while (bptr < eptr) {
        if (StringEquN(bptr, subkw, sublen)) {
            *retbptr = bptr;
            return true;
        }

        ptr = SrchTheChar(bptr, eptr, '\n');
        if (ptr)
            bptr = ptr;
        bptr++;
    }

    *retbptr = bptr;
    return false;
}

/**********************************************************
 *
 *   static void GetEmblRefType(bases, source, dbp):
 *
 *      If there is no "RP" line, default, or there is "RP"
 *   line and it contains "1-endbases", then
 *   type = ParFlat_REF_END, pub for "descr".
 *      Otherwise, ParFlat_REF_BTW, for SeqFeatPub.
 *
 *                                              6-15-93
 *
 **********************************************************/
static void GetEmblRefType(size_t bases, Parser::ESource source, DataBlkPtr dbp)
{
    char* ptr;
    char* bptr;
    char* eptr;
    char* sptr;

    bptr = dbp->mOffset;
    eptr = bptr + dbp->len;

    if (! GetSubNodeType("RP", &bptr, eptr)) {
        if (source == Parser::ESource::EMBL)
            dbp->mType = ParFlat_REF_NO_TARGET;
        else
            dbp->mType = ParFlat_REF_END;
        return;
    }

    const string str = " 1-" + to_string(bases);
    ptr = SrchTheStr(bptr, eptr, str.c_str());
    if (ptr) {
        dbp->mType = ParFlat_REF_END;
        return;
    }

    if (source == Parser::ESource::EMBL) {
        ptr = SrchTheStr(bptr, eptr, " 0-0");
        if (ptr) {
            dbp->mType = ParFlat_REF_NO_TARGET;
            return;
        }
    }

    dbp->mType = ParFlat_REF_BTW;
    if (source == Parser::ESource::NCBI) {
        for (sptr = bptr + 1; sptr < eptr && *sptr != 'R';)
            sptr++;
        if (SrchTheStr(bptr, sptr, "sites"))
            dbp->mType = ParFlat_REF_SITES;
    }
}

/**********************************************************
 *
 *   void GetEmblSubBlock(bases, source, entry):
 *
 *      To build feature block:
 *      - report error if no FT data in the FH block;
 *      - to fit genbank feature table parsing:
 *        - only leave first FH line;
 *        - replace "FT" to "  ";
 *        - delete any XX blocks.
 *
 *                                              5-27-93
 *
 **********************************************************/
void GetEmblSubBlock(size_t bases, Parser::ESource source, const DataBlk& entry)
{
    DataBlkPtr  temp;
    DataBlkPtr  curdbp;
    DataBlkPtr  predbp;
    EntryBlkPtr ebp;

    temp = TrackNodeType(entry, ParFlat_OS);
    for (; temp; temp = temp->mpNext) {
        if (temp->mType != ParFlat_OS)
            continue;

        BuildSubBlock(temp, ParFlat_OC, "OC");
        BuildSubBlock(temp, ParFlat_OG, "OG");
        GetLenSubNode(temp);
    }

    temp = TrackNodeType(entry, ParFlat_RN);
    for (; temp; temp = temp->mpNext) {
        if (temp->mType != ParFlat_RN)
            continue;

        fta_check_mult_ids(temp, "RX   MEDLINE;", "RX   PUBMED;");
        BuildSubBlock(temp, ParFlat_RC, "RC");
        BuildSubBlock(temp, ParFlat_RP, "RP");
        BuildSubBlock(temp, ParFlat_RX, "RX");
        BuildSubBlock(temp, ParFlat_RG, "RG");
        BuildSubBlock(temp, ParFlat_RA, "RA");
        BuildSubBlock(temp, ParFlat_RT, "RT");
        BuildSubBlock(temp, ParFlat_RL, "RL");
        GetEmblRefType(bases, source, temp);
        GetLenSubNode(temp);
    }

    ebp    = static_cast<EntryBlk*>(entry.mpData);
    temp   = ebp->chain;
    predbp = temp;
    curdbp = temp->mpNext;
    while (curdbp) {
        if (curdbp->mType != ParFlat_FH) {
            predbp = curdbp;
            curdbp = curdbp->mpNext;
            continue;
        }

        if (TrimEmblFeatBlk(curdbp)) {
            BuildFeatureBlock(curdbp);
            GetLenSubNode(curdbp);

            predbp = curdbp;
            curdbp = curdbp->mpNext;
        } else /* report error, free this node */
        {
            ErrPostStr(SEV_WARNING, ERR_FEATURE_NoFeatData, "No feature data in the FH block (Embl)");

            predbp->mpNext = curdbp->mpNext;
            curdbp->mpNext = nullptr;
            delete curdbp;
            curdbp = predbp->mpNext;
        }
    }
}

/**********************************************************
 *
 *   void BuildSubBlock(dbp, subtype, subkw):
 *
 *      Some of sub-keyword may not be exist in every entry.
 *
 *                                              4-7-93
 *
 **********************************************************/
void BuildSubBlock(DataBlkPtr dbp, Int2 subtype, const char* subkw)
{
    char* bptr;
    char* eptr;

    bptr = dbp->mOffset;
    eptr = bptr + dbp->len;

    if (GetSubNodeType(subkw, &bptr, eptr)) {
        InsertDatablkVal(reinterpret_cast<DataBlk**>(&dbp->mpData), subtype, bptr, eptr - bptr);
    }
}

/**********************************************************
 *
 *   void GetLenSubNode(dbp):
 *
 *      Recalculate the length for the node which has
 *   subkeywords.
 *
 *                                              4-7-93
 *
 **********************************************************/
void GetLenSubNode(DataBlkPtr dbp)
{
    DataBlkPtr curdbp;
    DataBlkPtr ndbp;
    DataBlkPtr ldbp;
    char*      offset;
    char*      s;
    Int2       n;
    bool       done = false;

    if (! dbp->mpData) /* no sublocks in this block */
        return;

    offset = dbp->mOffset;
    for (s = offset; *s != '\0' && isdigit(*s) == 0;)
        s++;
    n    = atoi(s);
    ldbp = nullptr;
    for (ndbp = static_cast<DataBlk*>(dbp->mpData); ndbp; ndbp = ndbp->mpNext) {
        size_t l = ndbp->mOffset - offset;
        if (l > 0 && l < dbp->len) {
            dbp->len = l;
            ldbp     = ndbp;
        }
    }

    if (ldbp != dbp->mpData && ldbp) {
        ErrPostEx(SEV_WARNING, ERR_FORMAT_LineTypeOrder, "incorrect line type order for reference %d", n);
        done = true;
    }

    curdbp = static_cast<DataBlk*>(dbp->mpData);
    for (; curdbp->mpNext; curdbp = curdbp->mpNext) {
        offset = curdbp->mOffset;
        ldbp   = nullptr;
        for (ndbp = static_cast<DataBlk*>(dbp->mpData); ndbp; ndbp = ndbp->mpNext) {
            size_t l = ndbp->mOffset - offset;
            if (l > 0 && l < curdbp->len) {
                curdbp->len = l;
                ldbp        = ndbp;
            }
        }
        if (ldbp != curdbp->mpNext && ldbp && ! done) {
            ErrPostEx(SEV_WARNING, ERR_FORMAT_LineTypeOrder, "incorrect line type order for reference %d", n);
        }
    }
}

/**********************************************************/
CRef<CPatent_seq_id> MakeUsptoPatSeqId(const char* acc)
{
    CRef<CPatent_seq_id> pat_id;
    const char*          p;
    const char*          q;

    if (! acc || *acc == '\0')
        return (pat_id);

    pat_id = new CPatent_seq_id;

    p = StringChr(acc, '|');

    q = StringChr(p + 1, '|');
    pat_id->SetCit().SetCountry(string(p + 1, q));

    p = StringChr(q + 1, '|');
    pat_id->SetCit().SetId().SetNumber(string(q + 1, p));

    q = StringChr(p + 1, '|');
    pat_id->SetCit().SetDoc_type(string(p + 1, q));

    pat_id->SetSeqid(atoi(q + 1));

    return (pat_id);
}

/**********************************************************
 *
 *   static Uint ValidSeqType(accession, type, is_nuc, is_tpa):
 *
 *                                              9-16-93
 *
 **********************************************************/
static Uint1 ValidSeqType(const char* accession, Uint1 type)
{
    // CSeq_id::E_Choice cho;

    if (type == CSeq_id::e_Swissprot || type == CSeq_id::e_Pir || type == CSeq_id::e_Prf ||
        type == CSeq_id::e_Pdb || type == CSeq_id::e_Other)
        return (type);

    if (type != CSeq_id::e_Genbank && type != CSeq_id::e_Embl && type != CSeq_id::e_Ddbj &&
        type != CSeq_id::e_Tpg && type != CSeq_id::e_Tpe && type != CSeq_id::e_Tpd)
        return (CSeq_id::e_not_set);

    if (! accession)
        return (type);

    const auto cho = CSeq_id::GetAccType(CSeq_id::IdentifyAccession(accession));
    /*
    if (is_nuc)
        cho = GetNucAccOwner(accession);
    else
        cho = GetProtAccOwner(accession);
    */
    if ((type == CSeq_id::e_Genbank || type == CSeq_id::e_Tpg) &&
        (cho == CSeq_id::e_Genbank || cho == CSeq_id::e_Tpg))
        return (cho);
    else if ((type == CSeq_id::e_Ddbj || type == CSeq_id::e_Tpd) &&
             (cho == CSeq_id::e_Ddbj || cho == CSeq_id::e_Tpd))
        return (cho);
    else if ((type == CSeq_id::e_Embl || type == CSeq_id::e_Tpe) &&
             (cho == CSeq_id::e_Embl || cho == CSeq_id::e_Tpe))
        return (cho);
    return type;
}

/**********************************************************
 *
 *   CRef<CSeq_id> MakeAccSeqId(acc, seqtype, accver, vernum,
 *                                                   is_nuc, is_tpa):
 *
 *                                              5-10-93
 *
 **********************************************************/
CRef<CSeq_id> MakeAccSeqId(const char* acc, Uint1 seqtype, bool accver, Int2 vernum)
{
    CRef<CSeq_id> id;

    if (! acc || *acc == '\0')
        return id;

    seqtype = ValidSeqType(acc, seqtype);

    if (seqtype == CSeq_id::e_not_set)
        return id;

    CRef<CTextseq_id> text_id(new CTextseq_id);
    text_id->SetAccession(acc);

    if (accver && vernum > 0)
        text_id->SetVersion(vernum);

    id = new CSeq_id;
    SetTextId(seqtype, *id, *text_id);
    return id;
}

/**********************************************************
 *
 *   SeqIdPtr MakeLocusSeqId(locus, seqtype):
 *
 *                                              5-13-93
 *
 **********************************************************/
CRef<CSeq_id> MakeLocusSeqId(const char* locus, CSeq_id::E_Choice seqtype)
{
    CRef<CSeq_id> res;
    if (! locus || *locus == '\0')
        return res;

    CRef<CTextseq_id> text_id(new CTextseq_id);
    text_id->SetName(locus);

    res.Reset(new CSeq_id);
    SetTextId(seqtype, *res, *text_id);

    return res;
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************/
static CRef<CSeq_id> MakeSegSetSeqId(const char* accession, const string& locus, Uint1 seqtype, bool is_tpa)
{
    CRef<CSeq_id> res;
    if (locus.empty())
        return res;

    seqtype = ValidSeqType(accession, seqtype);

    if (seqtype == CSeq_id::e_not_set)
        return res;

    CRef<CTextseq_id> text_id(new CTextseq_id);
    text_id->SetName(locus);

    res.Reset(new CSeq_id);
    SetTextId(seqtype, *res, *text_id);

    return res;
}
// LCOV_EXCL_STOP

/**********************************************************
 *
 *   char* SrchNodeSubType(entry, type, subtype, len):
 *
 *      Return a memory location of the node which has
 *   the "subtype".
 *
 *                                              4-7-93
 *
 **********************************************************/
char* SrchNodeSubType(const DataBlk& entry, Int2 type, Int2 subtype, size_t* len)
{
    DataBlkPtr mdbp;
    DataBlkPtr sdbp;

    *len = 0;
    mdbp = TrackNodeType(entry, type);
    if (! mdbp)
        return nullptr;

    sdbp = static_cast<DataBlk*>(mdbp->mpData);

    while (sdbp && sdbp->mType != subtype)
        sdbp = sdbp->mpNext;

    if (! sdbp)
        return nullptr;

    *len = sdbp->len;
    return (sdbp->mOffset);
}

/**********************************************************/
static void SetEmptyId(CBioseq& bioseq)
{
    CRef<CObject_id> emptyId(new CObject_id);
    emptyId->SetId8(0);

    CRef<CSeq_id> seqId(new CSeq_id);
    seqId->SetLocal(*emptyId);

    bioseq.SetId().push_back(seqId);
}

/**********************************************************/
CRef<CBioseq> CreateEntryBioseq(ParserPtr pp)
{
    IndexblkPtr ibp;

    char* locus;
    const char* acc;
    Uint1 seqtype;

    CRef<CBioseq> res(new CBioseq);

    /* create the entry framework */

    ibp   = pp->entrylist[pp->curindx];
    locus = ibp->locusname;
    acc   = ibp->acnum;

    /* get the SeqId */
    if (pp->source == Parser::ESource::USPTO) {
        CRef<CSeq_id>        id(new CSeq_id);
        CRef<CPatent_seq_id> psip = MakeUsptoPatSeqId(acc);
        id->SetPatent(*psip);
        return (res);
    }
    if (pp->source == Parser::ESource::EMBL && ibp->is_tpa)
        seqtype = CSeq_id::e_Tpe;
    else
        seqtype = ValidSeqType(acc, pp->seqtype);

    if (seqtype == CSeq_id::e_not_set) {
        if (acc && ! NStr::IsBlank(acc)) {
            auto pId = Ref(new CSeq_id(CSeq_id::e_Local, acc));
            res->SetId().push_back(move(pId));
        } else if (pp->mode == Parser::EMode::Relaxed && locus) {
            auto pId = Ref(new CSeq_id(CSeq_id::e_Local, locus));
            res->SetId().push_back(move(pId));
        } else {
            SetEmptyId(*res);
        }
    } else if ((! locus || *locus == '\0') && (! acc || *acc == '\0')) {
        SetEmptyId(*res);
    } else {
        CRef<CTextseq_id> textId(new CTextseq_id);

        if (ibp->embl_new_ID == false && locus && *locus != '\0' &&
            (! acc || ! StringEqu(acc, locus)))
            textId->SetName(locus);

        if (acc && *acc != '\0')
            textId->SetAccession(acc);

        if (pp->accver && ibp->vernum > 0)
            textId->SetVersion(ibp->vernum);

        CRef<CSeq_id> seqId(new CSeq_id);
        if (SetTextId(seqtype, *seqId, *textId))
            res->SetId().push_back(seqId);
        else
            SetEmptyId(*res);
    }

    return res;
}

/**********************************************************
 *
 *   char* GetDescrComment(offset, len, col_data, is_htg):
 *
 *      Return a pointer to a string comment.
 *      Strip tailing or leading blanks, unless the
 *   following rules occurrs (all the length will count
 *   leading or tailing blanks):
 *      - replace "\n" to "~~ ~~" if the length of a
 *        line <= 12, except first blank line;
 *      - if the column 13 is blank in the current line
 *        and the previous line does not be added "~" at
 *        end, then add "~" the beginning of the line
 *        (indent format);
 *      - replace "\n" to "~" if the length of a
 *        line < 50 and (not a last line or not a first
 *        line);
 *     -- otherwise, change "\n" to a space.
 *
 *                                              4-28-93
 *
 **********************************************************/
char* GetDescrComment(char* offset, size_t len, Int2 col_data, bool is_htg, bool is_pat)
{
    char* p;
    char* q;
    char* r;
    char* str;

    bool  within = false;
    char* bptr   = offset;
    char* eptr   = bptr + len;
    char* com    = MemNew(len + 1);

    for (str = com; bptr < eptr; bptr = p + 1) {
        p = SrchTheChar(bptr, eptr, '\n');

        /* skip HTG generated comments starting with '*' */
        if ((is_htg && bptr[col_data] == '*') ||
            StringEquN(bptr, "XX", 2))
            continue;

        if (! within) {
            *p = '\0';
            r  = StringStr(bptr, "-START##");
            *p = '\n';
            if (r)
                within = true;
        }

        q = bptr;
        if (*q == 'C')
            q++;
        if (*q == 'C')
            q++;
        while (*q == ' ')
            q++;
        if (q == p) {
            if (*(str - 1) != '~')
                *str++ = '~';
            *str++ = '~';
            continue;
        }

        if (p - bptr < col_data)
            continue;

        bptr += col_data;
        size_t size = p - bptr;

        if (*bptr == ' ' && *(str - 1) != '~')
            *str++ = '~';
        MemCpy(str, bptr, size);
        str += size;
        if (is_pat && size > 4 &&
            q[0] >= 'A' && q[0] <= 'Z' && q[1] >= 'A' && q[1] <= 'Z' &&
            StringEquN(q + 2, "   ", 3))
            *str++ = '~';
        else if (size < 50 || within)
            *str++ = '~';
        else
            *str++ = ' ';

        if (within) {
            *p = '\0';
            r  = StringStr(bptr, "-END##");
            *p = '\n';
            if (r)
                within = false;
        }
    }

    for (p = com;;) {
        p = StringStr(p, "; ");
        if (! p)
            break;
        for (p += 2, eptr = p; *eptr == ' ';)
            eptr++;
        if (eptr > p)
            fta_StringCpy(p, eptr);
    }
    for (p = com; *p == ' ';)
        p++;
    if (p > com)
        fta_StringCpy(com, p);
    for (p = com; *p != '\0';)
        p++;
    if (p > com) {
        for (p--;; p--) {
            if (*p == ' ' || *p == '\t' || *p == ';' || *p == ',' ||
                *p == '.' || *p == '~') {
                if (p > com)
                    continue;
                *p = '\0';
            }
            break;
        }
        if (*p != '\0') {
            p++;
            if (StringEquN(p, "...", 3))
                p[3] = '\0';
            else if (StringChr(p, '.')) {
                *p   = '.';
                p[1] = '\0';
            } else
                *p = '\0';
        }
    }
    if (*com != '\0')
        return (com);
    MemFree(com);
    return nullptr;
}

/**********************************************************/
static void fta_fix_secondaries(TokenBlkPtr secs)
{
    TokenBlkPtr tbp;
    char*       p;

    if (! secs || ! secs->next || ! secs->str ||
        ! secs->next->str || fta_if_wgs_acc(secs->str) != 0 ||
        ! StringEqu(secs->next->str, "-"))
        return;

    tbp        = new TokenBlk;
    tbp->str   = StringSave(secs->str);
    tbp->next  = secs->next;
    secs->next = tbp;

    for (p = tbp->str; *(p + 1) != '\0';)
        p++;
    *p = '1';
}


/**********************************************************/
/*
static void fta_fix_secondaries(list<string>& secondaries)
{
    if (secondaries.size() < 2) {
        return;
    }

    auto it = secondaries.begin();
    const auto& first = *it;
    const auto& second = *next(it);

    if (first.empty()||
        second.empty() ||
        fta_if_wgs_acc(second.c_str()) != 0 ||
        second != "-") {
        return;
    }

    string newSecondary = *it;
    newSecondary.back() = '1';
    ++it;
    secondaries.insert(it, newSecondary);
}
*/

/**********************************************************
 *
 *   void GetExtraAccession(ibp, allow_uwsec, source, accessions):
 *
 *      Skip first accession, put remaining accessions
 *   to link list 'accessions'.
 *      Each accession separated by ";" or blanks.
 *
 **********************************************************/
void GetExtraAccession(IndexblkPtr ibp, bool allow_uwsec, Parser::ESource source, TAccessionList& accessions)
{
    TokenBlkPtr tbp;
    Int4        pri_acc;
    Int4        sec_acc;
    const char* text;
    char*       acc;
    char*       p;
    size_t      i = 0;

    bool unusual_wgs;
    bool unusual_wgs_msg;
    bool is_cp;

    CSeq_id::E_Choice pri_owner;
    CSeq_id::E_Choice sec_owner;

    if (! ibp->secaccs) {
        return;
    }

    acc       = StringSave(ibp->acnum);
    is_cp     = (acc[0] == 'C' && acc[1] == 'P');
    pri_acc   = fta_if_wgs_acc(acc);
    pri_owner = GetNucAccOwner(acc);
    if (pri_acc == 1 || pri_acc == 4) {
        for (p = acc; (*p >= 'A' && *p <= 'Z') || *p == '_';)
            p++;
        *p = '\0';
        i  = StringLen(acc);
    }

    if (source == Parser::ESource::EMBL) {
        fta_fix_secondaries(ibp->secaccs);
    }

    unusual_wgs = false;
    for (tbp = ibp->secaccs; tbp; tbp = tbp->next) {
        p = tbp->str;
        if (p[0] == '-' && p[1] == '\0') {
            tbp = tbp->next;
            if (! tbp)
                break;
            if (! accessions.empty()) {
                accessions.back() += '-';
                accessions.back() += tbp->str;
            }
            continue;
        }

        DelNoneDigitTail(p);
        sec_acc = fta_if_wgs_acc(p);

        unusual_wgs_msg = true;
        if (sec_acc == 0 || sec_acc == 3 ||
            sec_acc == 4 || sec_acc == 6 ||
            sec_acc == 10 || sec_acc == 12) /*  0 = AAAA01000000,
                                                    3 = AAAA00000000,
                                                    4 = GAAA01000000,
                                                    6 = GAAA00000000,
                                                   10 = KAAA01000000,
                                                   12 = KAAA00000000 */
        {
            if (ibp->is_contig &&
                (ibp->wgssec.empty() || NStr::CommonSuffixSize(ibp->wgssec, p) >= 4))
                unusual_wgs_msg = false;
            if (ibp->wgssec.empty())
                ibp->wgssec = p;
        }

        sec_owner = GetNucAccOwner(p);

        if (sec_acc < 0 || sec_acc == 2) {
            if (pri_acc == 1 || pri_acc == 5 || pri_acc == 11) {
                if (! allow_uwsec) {
                    ErrPostEx(SEV_REJECT, ERR_ACCESSION_WGSWithNonWGS_Sec, "This WGS/TSA/TLS record has non-WGS/TSA/TLS secondary accession \"%s\". WGS/TSA/TLS records are not currently allowed to replace finished sequence records, scaffolds, etc. without human review and confirmation.", p);
                    ibp->drop = true;
                } else {
                    ErrPostEx(SEV_WARNING, ERR_ACCESSION_WGSWithNonWGS_Sec, "This WGS/TSA/TLS record has non-WGS/TSA/TLS secondary accession \"%s\". This is being allowed via the use of a special parser flag.", p);
                }
            }

            accessions.push_back(p);
            continue;
        }

        if (sec_acc == 3 || sec_acc == 6) /* like AAAA00000000 */
        {
            if (pri_owner == CSeq_id::e_Embl && sec_owner == CSeq_id::e_Embl &&
                (pri_acc == 1 || pri_acc == 5 || pri_acc == 11) &&
                source == Parser::ESource::EMBL)
                continue;
            if (source != Parser::ESource::EMBL && source != Parser::ESource::DDBJ &&
                source != Parser::ESource::Refseq) {
                ErrPostEx(SEV_REJECT, ERR_ACCESSION_WGSMasterAsSecondary, "WGS/TSA/TLS master accession \"%s\" is not allowed to be used as a secondary accession number.", p);
                ibp->drop = true;
            }
            continue;
        }

        if (pri_acc == 1 || pri_acc == 5 || pri_acc == 11) /* WGS/TSA/TLS
                                                                   contig */
        {
            i = (StringEquN(p, "NZ_", 3)) ? 7 : 4;
            if (! StringEquN(p, ibp->acnum, i)) {
                if (! allow_uwsec) {
                    ErrPostEx(SEV_REJECT, ERR_ACCESSION_UnusualWGS_Secondary, "This record has one or more WGS/TSA/TLS secondary accession numbers which imply that a WGS/TSA/TLS project is being replaced (either by another project or by finished sequence). This is not allowed without human review and confirmation.");
                    ibp->drop = true;
                } else if (! is_cp || source != Parser::ESource::NCBI) {
                    ErrPostEx(SEV_WARNING, ERR_ACCESSION_UnusualWGS_Secondary, "This record has one or more WGS/TSA/TLS secondary accession numbers which imply that a WGS/TSA project is being replaced (either by another project or by finished sequence). This is being allowed via the use of a special parser flag.");
                }
            }
        } else if (pri_acc == 2) /* WGS scaffold */
        {
            if (sec_acc == 1 || sec_acc == 5 || sec_acc == 11) /* WGS/TSA/TLS
                                                                   contig */
            {
                ErrPostEx(SEV_REJECT, ERR_ACCESSION_ScfldHasWGSContigSec, "This record, which appears to be a scaffold, has one or more WGS/TSA/TLS contig accessions as secondary. Currently, it does not make sense for a contig to replace a scaffold.");
                ibp->drop = true;
            }
        } else if (unusual_wgs_msg) {
            if (! allow_uwsec) {
                if (! unusual_wgs) {
                    if (sec_acc == 1 || sec_acc == 5 || sec_acc == 11)
                        text = "WGS/TSA/TLS contig secondaries are present, implying that a scaffold is replacing a contig";
                    else
                        text = "This record has one or more WGS/TSA/TLS secondary accession numbers which imply that a WGS/TSA/TLS project is being replaced (either by another project or by finished sequence)";
                    ErrPostEx(SEV_REJECT, ERR_ACCESSION_UnusualWGS_Secondary, "%s. This is not allowed without human review and confirmation.", text);
                }
                unusual_wgs = true;
                ibp->drop   = true;
            } else if (! is_cp || source != Parser::ESource::NCBI) {
                if (! unusual_wgs) {
                    if (sec_acc == 1 || sec_acc == 5 || sec_acc == 11)
                        text = "WGS/TSA/TLS contig secondaries are present, implying that a scaffold is replacing a contig";
                    else
                        text = "This record has one or more WGS/TSA/TLS secondary accession numbers which imply that a WGS/TSA/TLS project is being replaced (either by another project or by finished sequence)";
                    ErrPostEx(SEV_WARNING, ERR_ACCESSION_UnusualWGS_Secondary, "%s. This is being allowed via the use of a special parser flag.", text);
                }
                unusual_wgs = true;
            }
        }

        if (pri_acc == 1 || pri_acc == 5 || pri_acc == 11) {
            if (StringEquN(acc, p, i) && p[i] >= '0' && p[i] <= '9') {
                if (sec_acc == 1 || sec_acc == 5 || pri_acc == 11)
                    accessions.push_back(p);
            } else if (allow_uwsec) {
                accessions.push_back(p);
            }
        } else if (pri_acc == 2) {
            if (sec_acc == 0 || sec_acc == 4) /* like AAAA10000000 */
                accessions.push_back(p);
        } else if (allow_uwsec || (! unusual_wgs_msg && (source == Parser::ESource::DDBJ || source == Parser::ESource::EMBL))) {
            accessions.push_back(p);
        }
    }

    MemFree(acc);
}

/**********************************************************/
static void fta_fix_tpa_keywords(TKeywordList& keywords)
{
    const char* p;

    for (string& key : keywords) {
        if (key.empty())
            continue;

        if (NStr::CompareNocase(key.c_str(), "TPA") == 0)
            key = "TPA";
        else if (StringEquNI(key.c_str(), "TPA:", 4)) {
            string buf("TPA:");

            for (p = key.c_str() + 4; *p == ' ' || *p == '\t';)
                p++;

            buf += p;
            if (fta_is_tpa_keyword(buf.c_str())) {
                for (string::iterator p = buf.begin() + 4; p != buf.end(); ++p) {
                    if (*p >= 'A' && *p <= 'Z')
                        *p |= 040;
                }
            }

            swap(key, buf);
        }
    }
}

//  ----------------------------------------------------------------------------
void xFixEMBLKeywords(
    string& keywordData)
//  ----------------------------------------------------------------------------
{
    const string problematic("WGS Third Party Data");
    const string desired("WGS; Third Party Data");

    if (keywordData.empty()) {
        return;
    }
    auto wgsStart = NStr::FindNoCase(keywordData, problematic);
    if (wgsStart == string::npos) {
        return;
    }
    auto afterProblematic = keywordData[wgsStart + problematic.size()];
    if (afterProblematic != ';' && afterProblematic != '.') {
        return;
    }

    string fixedKeywords;
    if (wgsStart > 0) {
        auto semiBefore = keywordData.rfind(';', wgsStart - 1);
        if (semiBefore == string::npos) {
            return;
        }
        for (auto i = semiBefore + 1; i < wgsStart; ++i) {
            if (keywordData[i] != ' ') {
                return;
            }
        }
        fixedKeywords = keywordData.substr(0, wgsStart - 1);
    }
    fixedKeywords += desired;
    fixedKeywords += keywordData.substr(wgsStart + problematic.size());
    keywordData = fixedKeywords;
}


//  ----------------------------------------------------------------------------
void GetSequenceOfKeywords(
    const DataBlk& entry,
    int            type,
    int            col_data,
    TKeywordList&  keywords)
//  ----------------------------------------------------------------------------
{
    //  Expectation: Each keyword separated by ";", the last one ends with "."

    keywords.clear();
    auto keywordData = xGetNodeData(entry, type);
    if (keywordData.empty()) {
        return;
    }
    xGetBlkDataReplaceNewLine(keywordData, col_data);
    if (type == ParFlatSP_KW) {
        xStripECO(keywordData);
    }
    xFixEMBLKeywords(keywordData);

    NStr::Split(keywordData, ";", keywords);
    auto it   = keywords.begin();
    auto last = --keywords.end();
    while (it != keywords.end()) {
        auto& keyword = *it;
        NStr::TruncateSpacesInPlace(keyword);
        if (it == last) {
            NStr::TrimSuffixInPlace(keyword, ".");
            NStr::TruncateSpacesInPlace(keyword);
        }
        if (keyword.empty()) {
            keywords.erase(it++);
        } else {
            it++;
        }
    }

    fta_fix_tpa_keywords(keywords);
}


/**********************************************************
 *
 *   Int4 ScanSequence(warn, seqptr, bsp, conv,
 *                     replacechar, numns):
 *
 *      Scans a block of text converting characters to
 *   sequence and storing in the ByteStorePtr bsp.
 *      conv is a 255 Uint1 array where cells are indexed
 *   by the ASCII value of the character in ptr:
 *      - a value of 0 indicates skip;
 *      - a value of 1 indicates an character is
 *        unexpected (error);
 *      - otherwise, it is a IUPACaa (protein) or a IUPACna
 *        (nucleic acid) letter.
 *      Function returns count of valid characters
 *   converted to sequence.
 *
 *      When sequence is presented in columns, this
 *   function should be called once per line, so that
 *   numbers can be recognized as errors.
 *
 *                                              3-30-93
 *
 *      In order to skip the input flatfile put residue
 *   label count at end, add blank variable to assume each
 *   line only allow 6 blanks between residue.
 *
 *                                              7-28-93
 *
 **********************************************************/
Int4 ScanSequence(bool warn, char** seqptr, std::vector<char>& bsp, unsigned char* conv, Char replacechar, int* numns)
{
    Int2           blank;
    Int2           count;
    Uint1          residue;
    char*          ptr;
    static Uint1   buf[133];
    unsigned char* bu;

    blank = count = 0;
    ptr           = *seqptr;

    bu = buf;
    while (*ptr != '\n' && *ptr != '\0' && blank < 6 && count < 100) {
        if (numns && (*ptr == 'n' || *ptr == 'N'))
            (*numns)++;

        residue = conv[(int)*ptr];

        if (*ptr == ' ')
            blank++;

        if (residue > 2) {
            *bu++ = residue;
            count++;
        } else if (residue == 1 && (warn || isalpha(*ptr) != 0)) {
            /* it can be punctuation or alpha character */
            *bu++ = replacechar;
            count++;
            ErrPostEx(SEV_ERROR, ERR_SEQUENCE_BadResidue, "Invalid residue [%c]", *ptr);
            return (0);
        }
        ptr++;
    }

    *seqptr = ptr;
    std::copy(buf, bu, std::back_inserter(bsp));
    // BSWrite(bsp, buf, (Int4)(bu - buf));
    return (count);
}

/**********************************************************
 *
 *   bool GetSeqData(pp, entry, bsp, nodetype, seqconv,
 *                      seq_data_type):
 *
 *      Replace any bad residue to "N" if DNA sequence,
 *   "X" if protein sequence.
 *      PIR format allow punctuation in the sequence data,
 *   so no warning message if found punctuation in the
 *   sequence data.
 *                           Tatiana (mv from ScanSequence)
 *
 *                                              04-19-94
 *
 **********************************************************/
bool GetSeqData(ParserPtr pp, const DataBlk& entry, CBioseq& bioseq, Int4 nodetype, unsigned char* seqconv, Uint1 seq_data_type)
{
    // ByteStorePtr bp;
    IndexblkPtr ibp;
    char*       seqptr;
    char*       endptr;
    char*       str;
    Char        replacechar;
    size_t      len = 0;
    Int4        numns;

    ibp = pp->entrylist[pp->curindx];

    bioseq.SetInst().SetLength(static_cast<TSeqPos>(ibp->bases));

    if (ibp->is_contig || ibp->is_mga)
        return true;

    if (pp->format == Parser::EFormat::XML) {
        str    = XMLFindTagValue(entry.mOffset, ibp->xip, INSDSEQ_SEQUENCE);
        seqptr = str;
        if (seqptr) {
            len = StringLen(seqptr);
            if (pp->source != Parser::ESource::USPTO || ! ibp->is_prot)
                for (char* p = seqptr; *p != '\0'; p++)
                    if (*p >= 'A' && *p <= 'Z')
                        *p |= 040; // tolower
        }
    } else {
        str    = nullptr;
        seqptr = xSrchNodeType(entry, nodetype, &len);
    }

    if (! seqptr)
        return false;

    endptr = seqptr + len;

    if (pp->format == Parser::EFormat::GenBank || pp->format == Parser::EFormat::EMBL ||
        pp->format == Parser::EFormat::XML)
        replacechar = 'N';
    else
        replacechar = 'X';

    /* the sequence data will be located in next line of nodetype */
    if (pp->format == Parser::EFormat::XML) {
        while (*seqptr == ' ' || *seqptr == '\n' || *seqptr == '\t')
            seqptr++;
    } else {
        while (*seqptr != '\n')
            seqptr++;
        while (isalpha(*seqptr) == 0) /* skip leading blanks and digits */
            seqptr++;
    }

    std::vector<char> buf;
    size_t            seqlen = 0;
    for (numns = 0; seqptr < endptr;) {
        len = ScanSequence(true, &seqptr, buf, seqconv, replacechar, &numns);
        if (len == 0) {
            if (str)
                MemFree(str);
            return false;
        }

        seqlen += len;
        while (isalpha(*seqptr) == 0 && seqptr < endptr)
            seqptr++;
    }

    if (seqlen != bioseq.GetLength()) {
        ErrPostEx(SEV_WARNING, ERR_SEQUENCE_SeqLenNotEq, "Measured seqlen [%ld] != given [%ld]", (long int)seqlen, (long int)bioseq.GetLength());
    }

    if (str)
        MemFree(str);

    if (seq_data_type == CSeq_data::e_Iupacaa) {
        if (bioseq.GetLength() < 10) {
            if (pp->source == Parser::ESource::DDBJ || pp->source == Parser::ESource::EMBL) {
                if (ibp->is_pat == false)
                    ErrPostEx(SEV_WARNING, ERR_SEQUENCE_TooShort, "This sequence for this record falls below the minimum length requirement of 10 basepairs.");
                else
                    ErrPostEx(SEV_INFO, ERR_SEQUENCE_TooShortIsPatent, "This sequence for this patent record falls below the minimum length requirement of 10 basepairs.");
            } else {
                if (ibp->is_pat == false)
                    ErrPostEx(SEV_REJECT, ERR_SEQUENCE_TooShort, "This sequence for this record falls below the minimum length requirement of 10 basepairs.");
                else
                    ErrPostEx(SEV_REJECT, ERR_SEQUENCE_TooShortIsPatent, "This sequence for this patent record falls below the minimum length requirement of 10 basepairs.");
                ibp->drop = true;
            }
        }
        if (seqlen == static_cast<Uint4>(numns)) {
            ErrPostEx(SEV_REJECT, ERR_SEQUENCE_AllNs, "This nucleotide sequence for this record contains nothing but unknown (N) basepairs.");
            ibp->drop = true;
        }
    }

    bioseq.SetInst().SetSeq_data().Assign(CSeq_data(buf, static_cast<CSeq_data::E_Choice>(seq_data_type)));

    return true;
}

/**********************************************************
 *
 *   unsigned char* GetDNAConv():
 *
 *      DNA conversion table array.
 *
 *                                              3-29-93
 *
 **********************************************************/
unique_ptr<unsigned char[]> GetDNAConv(void)
{

    unique_ptr<unsigned char[]> dnaconv(new unsigned char[255]());
    MemSet((char*)dnaconv.get(), (Uint1)1, (size_t)255);

    dnaconv[32] = 0; /* blank */

    CSeqportUtil::TPair range = CSeqportUtil::GetCodeIndexFromTo(eSeq_code_type_iupacna);
    for (CSeqportUtil::TIndex i = range.first; i <= range.second; ++i) {
        const string& code = CSeqportUtil::GetCode(eSeq_code_type_iupacna, i);

        dnaconv[static_cast<int>(code[0])] = code[0];
        dnaconv[(int)tolower(code[0])]     = code[0];
    }

    return dnaconv;
}

/**********************************************************
 *
 *   unsigned char* GetProteinConv():
 *
 *      Protein conversion table array.
 *
 *                                              3-29-93
 *
 **********************************************************/
unique_ptr<unsigned char[]> GetProteinConv(void)
{
    // unsigned char*        protconv;
    unique_ptr<unsigned char[]> protconv(new unsigned char[255]());

    // protconv = (unsigned char*)MemNew((size_t)255);                  /* proteins */
    MemSet((char*)protconv.get(), (Uint1)1, (size_t)255); /* everything
                                                                an error */
    protconv[32] = 0;                                     /* blank */

    CSeqportUtil::TPair range = CSeqportUtil::GetCodeIndexFromTo(eSeq_code_type_iupacaa);
    for (CSeqportUtil::TIndex i = range.first; i <= range.second; ++i) {
        const string& code     = CSeqportUtil::GetCode(eSeq_code_type_iupacaa, i);
        protconv[(int)code[0]] = code[0]; /* swiss-prot, pir uses upper case
                                             protein code */
    }

    return (protconv);
}

/***********************************************************/
static CSeq_descr::Tdata::const_iterator GetDescrByChoice(const CSeq_descr& descr, Uint1 choice)
{
    const CSeq_descr::Tdata& descr_list = descr.Get();

    CSeq_descr::Tdata::const_iterator cur_descr = descr_list.begin();
    for (; cur_descr != descr_list.end(); ++cur_descr) {
        if ((*cur_descr)->Which() == choice)
            break;
    }

    return cur_descr;
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************
 *
 *   static void GetFirstSegDescrChoice(bio_set, choice,
 *                                      descr_new):
 *
 *                                              10-14-93
 *
 **********************************************************/
static void GetFirstSegDescrChoice(CBioseq& bioseq, Uint1 choice, CSeq_descr& descr_new)
{
    CSeq_descr&        descr      = bioseq.SetDescr();
    CSeq_descr::Tdata& descr_list = descr.Set();

    // Don't use GetDescrByChoice here just because GCC version does not support erase(const_iterator)
    CSeq_descr::Tdata::iterator cur_descr = descr_list.begin();
    for (; cur_descr != descr_list.end(); ++cur_descr) {
        if ((*cur_descr)->Which() == choice) {
            /* found the "choice" node, isolated node */
            descr_new.Set().push_back(*cur_descr);
            descr_list.erase(cur_descr);
            break;
        }
    }
}
// LCOV_EXCL_STOP

// SameCitation and 'PubEquivMatch' have a bit different logic,
// so below is an additional function that makes a check
// for equality according to 'PubEquivMatch' rules
static bool SameCitation_PubEquivMatch_Logic(const CPub_equiv& a, const CPub_equiv& b)
{
    for (const CRef<CPub>& it1 : a.Get()) {
        for (const CRef<CPub>& it2 : b.Get()) {
            if (it1->SameCitation(*it2)) {
                bool same = true;

                if (it1->Which() == CPub::e_Gen && it2->Which() == CPub::e_Gen) {
                    const CCit_gen& cit_a = it1->GetGen();
                    const CCit_gen& cit_b = it2->GetGen();

                    if (cit_a.IsSetSerial_number() && cit_b.IsSetSerial_number() && cit_a.GetSerial_number() == cit_b.GetSerial_number()) {
                        // The special condition of 'PubEquivMatch'
                        //    a->volume == NULL && b->volume == NULL &&
                        //    a->issue == NULL && b->issue == NULL &&
                        //    a->pages == NULL && b->pages == NULL &&
                        //    a->title == NULL && b->title == NULL &&
                        //    a->cit == NULL && b->cit == NULL &&
                        //    a->authors == NULL && b->authors == NULL &&
                        //    a->muid == -1 && b->muid == -1 &&
                        //    a->journal == NULL && b->journal == NULL &&
                        //    a->date == NULL && b->date == NULL &&
                        //    a->serial_number != -1 && b->serial_number != -1

                        if (! cit_a.IsSetVolume() && ! cit_b.IsSetVolume() &&
                            ! cit_a.IsSetIssue() && ! cit_b.IsSetIssue() &&
                            ! cit_a.IsSetPages() && ! cit_b.IsSetPages() &&
                            ! cit_a.IsSetTitle() && ! cit_b.IsSetTitle() &&
                            ! cit_a.IsSetCit() && ! cit_b.IsSetCit() &&
                            ! cit_a.IsSetAuthors() && ! cit_b.IsSetAuthors() &&
                            ! cit_a.IsSetMuid() && ! cit_b.IsSetMuid() &&
                            ! cit_a.IsSetJournal() && ! cit_b.IsSetJournal() &&
                            ! cit_a.IsSetDate() && ! cit_b.IsSetDate())
                            same = false; // SIC!!!
                    }
                }

                if (same)
                    return true;
            }
        }
    }

    return false;
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************
 *
 *   static bool CheckSegPub(pub, entries, same_pub_descr):
 *
 *                                              5-21-93
 *
 **********************************************************/
static bool CheckSegPub(const CPubdesc& pub, TEntryList& entries, std::set<CSeqdesc*>& same_pub_descr)
{
    if (! pub.IsSetPub() || ! pub.GetPub().IsSet() || pub.GetPub().Get().empty())
        return true;

    CRef<CPub> pub_ref = pub.GetPub().Get().front();

    if (! pub_ref->IsGen() || ! pub_ref->GetGen().IsSetSerial_number())
        return true;

    int num0 = pub_ref->GetGen().GetSerial_number();

    TEntryList::iterator next_seq = entries.begin();
    for (++next_seq; next_seq != entries.end(); ++next_seq) {
        if (! (*next_seq)->IsSetDescr())
            continue;

        CSeq_descr& descr = (*next_seq)->SetDescr();

        bool not_found = true;
        for (auto& cur_descr : descr.Set()) {
            if (! cur_descr->IsPub() || ! cur_descr->GetPub().IsSetPub() || ! cur_descr->GetPub().GetPub().IsSet() ||
                cur_descr->GetPub().GetPub().Get().empty())
                continue;

            const CPubdesc& cur_pub     = cur_descr->GetPub();
            const CPub&     cur_pub_ref = *cur_pub.GetPub().Get().front();

            if (! cur_pub_ref.IsGen() || ! cur_pub_ref.GetGen().IsSetSerial_number())
                continue;

            int num = cur_pub_ref.GetGen().GetSerial_number();

            if (! SameCitation_PubEquivMatch_Logic(cur_pub.GetPub(), pub.GetPub()))
                continue;

            if (num == num0) {
                same_pub_descr.insert(cur_descr); // store pointer to the same descr for future use
                not_found = false;
                break;
            }

            ErrPostStr(SEV_WARNING, ERR_SEGMENT_PubMatch, "Matching references with different serial numbers");
        }

        if (not_found)
            break;
    }

    return (next_seq == entries.end());
}
// LCOV_EXCL_STOP

/***********************************************************/
static void RemoveDescrByChoice(CSeq_descr& descr, Uint1 choice)
{
    CSeq_descr::Tdata& descr_list = descr.Set();

    for (CSeq_descr::Tdata::iterator cur_descr = descr_list.begin(); cur_descr != descr_list.end();) {
        if ((*cur_descr)->Which() == choice)
            cur_descr = descr_list.erase(cur_descr);
        else
            ++cur_descr;
    }
}

/**********************************************************
 *
 *   static void CleanUpSeqDescrChoice(entries, choice):
 *
 *                                              5-21-93
 *
 **********************************************************/
static void CleanUpSeqDescrChoice(TEntryList& entries, Uint1 choice)
{
    TEntryList::iterator next_seq = entries.begin();
    ++next_seq;

    for (; next_seq != entries.end(); ++next_seq)
        RemoveDescrByChoice((*next_seq)->SetDescr(), choice);
}

/**********************************************************
 *
 *   static void CleanUpSeqDescrPub(entries, to_clean):
 *
 *                                              1-13-16
 *
 **********************************************************/
static void CleanUpSeqDescrPub(TEntryList& entries, std::set<CSeqdesc*>& to_clean)
{
    TEntryList::iterator next_seq = entries.begin();
    ++next_seq;

    for (; next_seq != entries.end(); ++next_seq) {
        CSeq_descr::Tdata& descr_list = (*next_seq)->SetDescr().Set();
        for (CSeq_descr::Tdata::iterator cur_descr = descr_list.begin(); cur_descr != descr_list.end();) {
            std::set<CSeqdesc*>::iterator it = to_clean.find(*cur_descr);
            if (it != to_clean.end()) {
                cur_descr = descr_list.erase(cur_descr);
                to_clean.erase(it);
            } else
                ++cur_descr;
        }
    }
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************
 *
 *   static void GetSegPub(entries, descr):
 *
 *                                              5-21-93
 *
 **********************************************************/
static void GetSegPub(TEntryList& entries, CSeq_descr& descr)
{
    CBioseq&           bioseq     = entries.front()->SetSeq();
    CSeq_descr::Tdata& descr_list = bioseq.SetDescr().Set();

    for (CSeq_descr::Tdata::iterator cur_descr = descr_list.begin(); cur_descr != descr_list.end();) {
        if ((*cur_descr)->IsPub()) {
            CPubdesc& pubdesc = (*cur_descr)->SetPub();

            std::set<CSeqdesc*> same_pub_descr;
            if (CheckSegPub(pubdesc, entries, same_pub_descr)) {
                descr.Set().push_back(*cur_descr);
                cur_descr = descr_list.erase(cur_descr);

                CleanUpSeqDescrPub(entries, same_pub_descr);
            } else
                ++cur_descr;
        } else
            ++cur_descr;
    }
}

/**********************************************************
 *
 *   static bool CheckSegDescrChoice(entry, choice):
 *
 *                                              5-18-93
 *
 **********************************************************/
static bool CheckSegDescrChoice(const TEntryList& entries, Uint1 choice)
{
    string org;
    CDate  date;
    Int4   modif = -1;

    bool no_problem_found = true;
    for (TEntryList::const_iterator seq = entries.begin(); seq != entries.end(); ++seq) {
        const CSeq_descr&        descr      = (*seq)->GetDescr();
        const CSeq_descr::Tdata& descr_list = descr.Get();

        CSeq_descr::Tdata::const_iterator cur_descr = GetDescrByChoice(descr, choice);

        if (cur_descr == descr_list.end()) {
            no_problem_found = false;
            break;
        }

        if (choice == CSeqdesc::e_Org) {
            if (org.empty())
                org = (*cur_descr)->GetOrg().GetTaxname();
            else if (org != (*cur_descr)->GetOrg().GetTaxname()) {
                no_problem_found = false;
                break;
            }
        } else if (choice == CSeqdesc::e_Modif) {
            Int4 val = *(*cur_descr)->GetModif().begin();
            if (modif == -1)
                modif = val;
            else if (modif != val) {
                no_problem_found = false;
                break;
            }
        } else /* Seq_descr_update_date */
        {
            if (date.Which() == CDate::e_not_set)
                date.Assign((*cur_descr)->GetUpdate_date());
            else if (date.Compare((*cur_descr)->GetUpdate_date()) != CDate::eCompare_same) {
                no_problem_found = false;
                break;
            }
        }
    }

    return no_problem_found;
}
// LCOV_EXCL_STOP

/**********************************************************
 *
 *   static char* GetBioseqSetDescrTitle(descr):
 *
 *      Copy title from the first one, truncate before
 *   "complete cds" or "exon".
 *
 *                                              5-18-93
 *
 **********************************************************/
static char* GetBioseqSetDescrTitle(const CSeq_descr& descr)
{
    const Char* title;
    const Char* ptr;

    char* str;

    const CSeq_descr::Tdata& descr_list = descr.Get();

    CSeq_descr::Tdata::const_iterator cur_descr = descr_list.begin();
    for (; cur_descr != descr_list.end(); ++cur_descr) {
        if ((*cur_descr)->IsTitle())
            break;
    }

    if (cur_descr == descr_list.end())
        return nullptr;

    title = (*cur_descr)->GetTitle().c_str();

    ptr = StringStr(title, "complete cds");
    if (! ptr) {
        ptr = StringStr(title, "exon");
    }

    if (ptr) {
        str = StringSave(string(title, ptr).c_str());
        CleanTailNoneAlphaChar(str);
    } else {
        str = StringSave(title);
    }

    return str;
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************
 *
 *   static void SrchSegDescr(TEntryList& entries, CSeq_descr& descr):
 *
 *      Copy title from first one, truncate before
 *   "complete cds" or "exon"
 *      org, if they are all from one organism, then move
 *   the data to this set, and make NULL to the sep chains
 *   in which sep->mpData->descr->choice = Seq_descr_org.
 *      modif, if they are all same modifier, then move
 *   the data to this set, and make NULL to the sep chains
 *   in which sep->mpData->descr->choice = Seq_descr_modif.
 *
 **********************************************************/
static void SrchSegDescr(TEntryList& entries, CSeq_descr& descr)
{
    CRef<CSeq_entry>& entry  = entries.front();
    CBioseq&          bioseq = entry->SetSeq();

    char* title = GetBioseqSetDescrTitle(bioseq.GetDescr());
    if (title) {
        CRef<CSeqdesc> desc_new(new CSeqdesc);
        desc_new->SetTitle(title);
        descr.Set().push_back(desc_new);
    }

    if (CheckSegDescrChoice(entries, CSeqdesc::e_Org)) {
        GetFirstSegDescrChoice(bioseq, CSeqdesc::e_Org, descr);
        CleanUpSeqDescrChoice(entries, CSeqdesc::e_Org);
    }
    if (CheckSegDescrChoice(entries, CSeqdesc::e_Modif)) {
        GetFirstSegDescrChoice(bioseq, CSeqdesc::e_Modif, descr);
        CleanUpSeqDescrChoice(entries, CSeqdesc::e_Modif);
    }

    GetSegPub(entries, descr);

    if (CheckSegDescrChoice(entries, CSeqdesc::e_Update_date)) {
        GetFirstSegDescrChoice(bioseq, CSeqdesc::e_Update_date, descr);
        CleanUpSeqDescrChoice(entries, CSeqdesc::e_Update_date);
    }
}

/**********************************************************/
static void GetSegSetDblink(CSeq_descr& descr, TEntryList& entries /*SeqEntryPtr headsep*/, bool* drop)
{
    if (entries.empty())
        return;

    CRef<CSeqdesc> gpid,
        dblink,
        cur_gpid,
        cur_dblink;

    Uint4 dblink_count = 0;
    Uint4 gpid_count   = 0;

    bool bad_gpid   = false;
    bool bad_dblink = false;

    for (auto& entry : entries) {
        cur_gpid.Reset();
        cur_dblink.Reset();

        CSeq_descr::Tdata& descr_list = entry->SetDescr();

        for (CSeq_descr::Tdata::iterator cur_descr = descr_list.begin(); cur_descr != descr_list.end();) {
            if (! (*cur_descr)->IsUser()) {
                ++cur_descr;
                continue;
            }

            const CUser_object& user = (*cur_descr)->GetUser();
            if (! user.CanGetType() || user.GetType().GetStr().empty()) {
                ++cur_descr;
                continue;
            }

            string type_str = user.GetType().GetStr();

            if (type_str == "DBLink") {
                if (cur_dblink.NotEmpty())
                    continue;

                dblink_count++;
                cur_dblink = *cur_descr;

                if (dblink.Empty())
                    dblink = cur_dblink;

                cur_descr = descr_list.erase(cur_descr);
            } else if (type_str == "GenomeProjectsDB") {
                if (cur_gpid.NotEmpty())
                    continue;

                gpid_count++;
                cur_gpid = *cur_descr;

                if (gpid.Empty())
                    gpid = cur_gpid;

                cur_descr = descr_list.erase(cur_descr);
            } else
                ++cur_descr;
        }

        if (cur_dblink.NotEmpty()) {
            if (dblink.Empty())
                dblink = cur_dblink;
            else {
                if (! cur_dblink->Equals(*dblink)) {
                    bad_dblink = true;
                    break;
                }
            }
        }

        if (cur_gpid.NotEmpty()) {
            if (gpid.Empty())
                gpid = cur_gpid;
            else {
                if (! cur_gpid->Equals(*gpid)) {
                    bad_gpid = true;
                    break;
                }
            }
        }
    }

    if (bad_dblink == false && bad_gpid == false) {
        if (dblink_count > 0 && entries.size() != dblink_count)
            bad_dblink = true;
        if (gpid_count > 0 && entries.size() != gpid_count)
            bad_gpid = true;
    }

    if (bad_dblink) {
        ErrPostEx(SEV_REJECT, ERR_SEGMENT_DBLinkMissingOrNonUnique, "One or more member of segmented set has missing or non-unique DBLink user-object. Entry dropped.");
        *drop = true;
    }

    if (bad_gpid) {
        ErrPostEx(SEV_REJECT, ERR_SEGMENT_GPIDMissingOrNonUnique, "One or more member of segmented set has missing or non-unique GPID user-object. Entry dropped.");
        *drop = true;
    }

    if (bad_gpid || bad_dblink ||
        (dblink.Empty() && gpid.Empty()) ||
        descr.Get().empty())
        return;

    if (dblink.NotEmpty())
        descr.Set().push_back(dblink);
    if (gpid.NotEmpty())
        descr.Set().push_back(gpid);
}

/**********************************************************
 *
 *   static void GetBioseqSetDescr(entries, descr, drop)
 *
 *                                              1-20-16
 *
 **********************************************************/
static void GetBioseqSetDescr(TEntryList& entries, CSeq_descr& descr, bool* drop)
{
    SrchSegDescr(entries, descr); /* get from ASN.1 tree */
    GetSegSetDblink(descr, entries, drop);
}

/**********************************************************
 *
 *   static const char *GetMoleculeClassString(mol):
 *
 *                                              6-25-93
 *
 **********************************************************/
static const char* GetMoleculeClassString(Uint1 mol)
{
    if (mol == 0)
        return ("not-set");
    if (mol == 1)
        return ("DNA");
    if (mol == 2)
        return ("RNA");
    if (mol == 3)
        return ("AA");
    if (mol == 4)
        return ("NA");
    return ("other");
}

/**********************************************************
 *
 *   static CSeq_inst::EMol SrchSegSeqMol(entries):
 *
 *                                              5-14-93
 *
 **********************************************************/
static CSeq_inst::EMol SrchSegSeqMol(const TEntryList& entries)
{
    const CBioseq&  orig_bioseq = entries.front()->GetSeq();
    CSeq_inst::EMol mol         = orig_bioseq.GetInst().GetMol();

    for (const auto& entry : entries) {
        const CBioseq& cur_bioseq = entry->GetSeq();
        if (mol == cur_bioseq.GetInst().GetMol())
            continue;

        ErrPostEx(SEV_WARNING, ERR_SEGMENT_DiffMolType, "Different molecule type in the segment set, \"%s\" to \"%s\"", GetMoleculeClassString(mol), GetMoleculeClassString(cur_bioseq.GetInst().GetMol()));

        return CSeq_inst::eMol_na;
    }

    return mol;
}

/**********************************************************
 *
 *   static Int4 SrchSegLength(entries):
 *
 *                                              5-14-93
 *
 **********************************************************/
static Int4 SrchSegLength(const TEntryList& entries)
{
    Int4 length = 0;

    for (const auto& entry : entries) {
        const CBioseq& cur_bioseq = entry->GetSeq();
        length += cur_bioseq.GetLength();
    }

    return (length);
}

/**********************************************************
 *
 *   static CRef<CBioseq> GetBioseq(pp, orig_bioseq, slp):
 *
 *                                              5-12-93
 *
 **********************************************************/
static CRef<CBioseq> GetBioseq(ParserPtr pp, const TEntryList& entries, const CSeq_loc& slp)
{
    IndexblkPtr   ibp = pp->entrylist[pp->curindx];
    CRef<CBioseq> bioseq(new CBioseq);

    {
        string locusname = "SEG_";
        locusname.append(ibp->blocusname);
        bioseq->SetId().push_back(MakeSegSetSeqId(ibp->acnum, locusname, pp->seqtype, ibp->is_tpa));
    }

    if (pp->seg_acc) {
        string locusname = "SEG_";
        locusname.append(ibp->acnum);
        bioseq->SetId().push_back(MakeSegSetSeqId(ibp->acnum, locusname, pp->seqtype, ibp->is_tpa));
    }

    const CSeq_entry& first_entry = *(entries.front());
    const CBioseq&    original    = first_entry.GetSeq();

    char* title = GetBioseqSetDescrTitle(original.GetDescr());

    if (title) {
        CRef<CSeqdesc> descr(new CSeqdesc);
        descr->SetTitle(title);

        MemFree(title);
        bioseq->SetDescr().Set().push_back(descr);
    }

    CSeq_inst& inst = bioseq->SetInst();
    inst.SetRepr(CSeq_inst::eRepr_seg);
    inst.SetMol(SrchSegSeqMol(entries));

    bool need_null = false;

    CRef<CSeq_loc> null_loc(new CSeq_loc());
    null_loc->SetNull();

    for (CSeq_loc::const_iterator seq_it = slp.begin(); seq_it != slp.end(); ++seq_it) {
        if (need_null)
            inst.SetExt().SetSeg().Set().push_back(null_loc);
        else
            need_null = true;

        CRef<CSeq_loc> seqloc(new CSeq_loc());
        seqloc->Assign(seq_it.GetEmbeddingSeq_loc());
        inst.SetExt().SetSeg().Set().push_back(seqloc);
    }

    inst.SetLength(SrchSegLength(entries));
    inst.SetFuzz().SetLim(CInt_fuzz::eLim_gt);

    return bioseq;
}
// LCOV_EXCL_STOP

/**********************************************************
 *
 *   void GetSeqExt(pp, slp):
 *
 *                                              5-12-93
 *
 **********************************************************/
void GetSeqExt(ParserPtr pp, CSeq_loc& seq_loc)
{
    const Indexblk* ibp;

    ibp = pp->entrylist[pp->curindx];

    CRef<CSeq_id> id = MakeAccSeqId(ibp->acnum, pp->seqtype, pp->accver, ibp->vernum);

    if (id.NotEmpty()) {
        CSeq_loc loc;
        loc.SetWhole(*id);

        seq_loc.Add(loc);
    }
}

// LCOV_EXCL_START
// Excluded per Mark's request on 12/14/2016
/**********************************************************
 *
 *   SeqEntryPtr BuildBioSegHeader(pp, headsep, seqloc):
 *
 *                                              2-24-94
 *
 **********************************************************/
void BuildBioSegHeader(ParserPtr pp, TEntryList& entries, const CSeq_loc& seqloc)
{
    if (entries.empty())
        return;

    IndexblkPtr ibp = pp->entrylist[pp->curindx];

    CRef<CBioseq> bioseq = GetBioseq(pp, entries, seqloc); /* Bioseq, ext */

    CRef<CSeq_entry> bioseq_entry(new CSeq_entry);
    bioseq_entry->SetSeq(*bioseq);

    CRef<CBioseq_set> bioseq_set(new CBioseq_set);
    bioseq_set->SetSeq_set().assign(entries.begin(), entries.end());
    bioseq_set->SetClass(CBioseq_set::eClass_parts);

    CRef<CSeq_entry> bioseq_set_entry(new CSeq_entry);
    bioseq_set_entry->SetSet(*bioseq_set);

    CRef<CBioseq_set> bioseq_set_head(new CBioseq_set);
    bioseq_set_head->SetSeq_set().push_back(bioseq_entry);
    bioseq_set_head->SetSeq_set().push_back(bioseq_set_entry);

    CRef<CSeq_descr> descr(new CSeq_descr);
    GetBioseqSetDescr(bioseq_set->SetSeq_set(), *descr, &ibp->drop);
    bioseq_set_head->SetDescr(*descr);
    bioseq_set_head->SetClass(CBioseq_set::eClass_segset);

    CRef<CSeq_entry> bioseq_set_head_entry(new CSeq_entry);
    bioseq_set_head_entry->SetSet(*bioseq_set_head);

    entries.clear();
    entries.push_back(bioseq_set_head_entry);
}

/**********************************************************
 *
 *   bool IsSegBioseq(const CSeq_id& id):
 *
 *                                              8-16-93
 *
 **********************************************************/
bool IsSegBioseq(const CSeq_id& id)
{
    if (id.Which() == CSeq_id::e_Patent)
        return false;

    const CTextseq_id* text_id = id.GetTextseq_Id();

    if (! text_id)
        return (false);

    if (! text_id->IsSetAccession() && text_id->IsSetName() &&
        StringEquN(text_id->GetName().c_str(), "SEG_", 4))
        return (true);
    return (false);
}
// LCOV_EXCL_STOP

/**********************************************************
 *
 *   char* check_div(pat_acc, pat_ref, est_kwd, sts_kwd,
 *                     gss_kwd, if_cds, div, tech, bases,
 *                     source, drop):
 *
 *                                              8-16-93
 *
 *      gss and 1000 limit added.
 *                                              9-09-96
 *
 **********************************************************/
bool check_div(bool pat_acc, bool pat_ref, bool est_kwd, bool sts_kwd, bool gss_kwd, bool if_cds, string& div, CMolInfo::TTech* tech, size_t bases, Parser::ESource source, bool& drop)
{
    if (div.empty())
        return false;

    if (pat_acc || pat_ref || StringEqu(div.c_str(), "PAT")) {
        if (pat_ref == false) {
            ErrPostEx(SEV_REJECT, ERR_DIVISION_MissingPatentRef, "Record in the patent division lacks a reference to a patent document. Entry dropped.");
            drop = true;
        }
        if (est_kwd) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_PATHasESTKeywords, "EST keywords present on patent sequence.");
        }
        if (sts_kwd) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_PATHasSTSKeywords, "STS keywords present on patent sequence.");
        }
        if (gss_kwd) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_PATHasGSSKeywords, "GSS keywords present on patent sequence.");
        }
        if (if_cds && source != Parser::ESource::EMBL) {
            ErrPostEx(SEV_INFO, ERR_DIVISION_PATHasCDSFeature, "CDS features present on patent sequence.");
        }
        if (! StringEqu(div.c_str(), "PAT")) {
            if (pat_acc)
                ErrPostEx(SEV_WARNING, ERR_DIVISION_ShouldBePAT, "Based on the accession number prefix letters, this is a patent sequence, but the division code is not PAT.");

            ErrPostEx(SEV_INFO, ERR_DIVISION_MappedtoPAT, "Division %s mapped to PAT based on %s.", div.c_str(), (pat_acc == false) ? "patent reference" : "accession number");
            div = "PAT";
        }
    } else if (est_kwd) {
        if (if_cds) {
            if (StringEqu(div.c_str(), "EST")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_ESTHasCDSFeature, "Coding region features exist and division is EST; EST might not be appropriate.");
            } else {
                ErrPostEx(SEV_INFO, ERR_DIVISION_NotMappedtoEST, "EST keywords exist, but this entry was not mapped to the EST division because of the presence of CDS features.");
                if (*tech == CMolInfo::eTech_est)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else if (bases > 1000) {
            if (StringEqu(div.c_str(), "EST")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_LongESTSequence, "Division code is EST, but the length of the sequence is %ld.", bases);
            } else {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_NotMappedtoEST, "EST keywords exist, but this entry was not mapped to the EST division because of the sequence length %ld.", bases);
                if (*tech == CMolInfo::eTech_est)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else {
            if (! StringEqu(div.c_str(), "EST"))
                ErrPostEx(SEV_INFO, ERR_DIVISION_MappedtoEST, "%s division mapped to EST.", div.c_str());
            *tech = CMolInfo::eTech_est;
            div.clear();
        }
    } else if (StringEqu(div.c_str(), "EST")) {
        ErrPostEx(SEV_WARNING, ERR_DIVISION_MissingESTKeywords, "Division is EST, but entry lacks EST-related keywords.");
        if (sts_kwd) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_ESTHasSTSKeywords, "STS keywords present on EST sequence.");
        }
        if (if_cds) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_ESTHasCDSFeature, "Coding region features exist and division is EST; EST might not be appropriate.");
        }
    } else if (sts_kwd) {
        if (if_cds) {
            if (StringEqu(div.c_str(), "STS")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_STSHasCDSFeature, "Coding region features exist and division is STS; STS might not be appropriate.");
            } else {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_NotMappedtoSTS, "STS keywords exist, but this entry was not mapped to the STS division because of the presence of CDS features.");
                if (*tech == CMolInfo::eTech_sts)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else if (bases > 1000) {
            if (StringEqu(div.c_str(), "STS")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_LongSTSSequence, "Division code is STS, but the length of the sequence is %ld.", bases);
            } else {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_NotMappedtoSTS, "STS keywords exist, but this entry was not mapped to the STS division because of the sequence length %ld.", bases);
                if (*tech == CMolInfo::eTech_sts)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else {
            if (! StringEqu(div.c_str(), "STS"))
                ErrPostEx(SEV_INFO, ERR_DIVISION_MappedtoSTS, "%s division mapped to STS.", div.c_str());
            *tech = CMolInfo::eTech_sts;
            div.clear();
        }
    } else if (StringEqu(div.c_str(), "STS")) {
        ErrPostEx(SEV_WARNING, ERR_DIVISION_MissingSTSKeywords, "Division is STS, but entry lacks STS-related keywords.");
        if (if_cds) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_STSHasCDSFeature, "Coding region features exist and division is STS; STS might not be appropriate.");
        }
    } else if (gss_kwd) {
        if (if_cds) {
            if (StringEqu(div.c_str(), "GSS")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_GSSHasCDSFeature, "Coding region features exist and division is GSS; GSS might not be appropriate.");
            } else {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_NotMappedtoGSS, "GSS keywords exist, but this entry was not mapped to the GSS division because of the presence of CDS features.");
                if (*tech == CMolInfo::eTech_survey)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else if (bases > 2500) {
            if (StringEqu(div.c_str(), "GSS")) {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_LongGSSSequence, "Division code is GSS, but the length of the sequence is %ld.", bases);
            } else {
                ErrPostEx(SEV_WARNING, ERR_DIVISION_NotMappedtoGSS, "GSS keywords exist, but this entry was not mapped to the GSS division because of the sequence length %ld.", bases);
                if (*tech == CMolInfo::eTech_survey)
                    *tech = CMolInfo::eTech_unknown;
            }
        } else {
            if (! StringEqu(div.c_str(), "GSS"))
                ErrPostEx(SEV_INFO, ERR_DIVISION_MappedtoGSS, "%s division mapped to GSS.", div.c_str());
            *tech = CMolInfo::eTech_survey;
            div.clear();
        }
    } else if (StringEqu(div.c_str(), "GSS")) {
        ErrPostEx(SEV_WARNING, ERR_DIVISION_MissingGSSKeywords, "Division is GSS, but entry lacks GSS-related keywords.");
        if (if_cds) {
            ErrPostEx(SEV_WARNING, ERR_DIVISION_GSSHasCDSFeature, "Coding region features exist and division is GSS; GSS might not be appropriate.");
        }
    } else if (StringEqu(div.c_str(), "TSA")) {
        *tech = CMolInfo::eTech_tsa;
        div.clear();
    }

    return ! div.empty();
}

/**********************************************************/
CRef<CSeq_id> StrToSeqId(const char* pch, bool pid)
{
    long  lID;
    char* pchEnd;

    CRef<CSeq_id> id;

    /* Figure out--what source is it */
    if (*pch == 'd' || *pch == 'e') {
        /* Get ID */
        errno = 0; /* clear errors, the error flag from stdlib */
        lID   = strtol(pch + 1, &pchEnd, 10);

        if (! ((lID == 0 && pch + 1 == pchEnd) || (lID == LONG_MAX && errno == ERANGE))) {
            /* Allocate new SeqId */

            id = new CSeq_id;
            CRef<CObject_id> tag(new CObject_id);
            tag->SetStr(string(pch, pchEnd - pch));

            CRef<CDbtag> dbtag(new CDbtag);
            dbtag->SetTag(*tag);
            dbtag->SetDb(pid ? "PID" : "NID");

            id->SetGeneral(*dbtag);
        }
    }

    return id;
}

/**********************************************************/
void AddNIDSeqId(CBioseq& bioseq, const DataBlk& entry, Int2 type, Int2 coldata, Parser::ESource source)
{
    DataBlkPtr dbp;
    char*      offset;

    dbp = TrackNodeType(entry, type);
    if (! dbp)
        return;

    offset            = dbp->mOffset + coldata;
    CRef<CSeq_id> sid = StrToSeqId(offset, false);
    if (sid.Empty())
        return;

    if (! (*offset == 'g' && (source == Parser::ESource::DDBJ || source == Parser::ESource::EMBL)))
        bioseq.SetId().push_back(sid);
}

/**********************************************************/
static void CheckDivCode(TEntryList& seq_entries, ParserPtr pp)
{
    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetDescr()) {
                CGB_block*      gb_block = nullptr;
                CMolInfo*       molinfo  = nullptr;
                CMolInfo::TTech tech     = CMolInfo::eTech_unknown;

                for (auto& descr : bioseq->SetDescr().Set()) {
                    if (descr->IsGenbank() && ! gb_block)
                        gb_block = &descr->SetGenbank();
                    else if (descr->IsMolinfo() && ! molinfo) {
                        molinfo = &descr->SetMolinfo();
                        tech    = molinfo->GetTech();
                    }

                    if (gb_block && molinfo)
                        break;
                }

                if (! gb_block)
                    continue;

                IndexblkPtr ibp = pp->entrylist[pp->curindx];

                if (tech == CMolInfo::eTech_tsa &&
                    ! NStr::CompareNocase(ibp->division, "TSA"))
                    continue;

                if (! gb_block->IsSetDiv()) {
                    ErrPostEx(SEV_WARNING, ERR_DIVISION_GBBlockDivision, "input division code is preserved in GBBlock");
                    gb_block->SetDiv(ibp->division);
                }
            }
        }
    }
}

/**********************************************************/
static const CBioSource* GetTopBiosource(const CSeq_entry& entry)
{
    const TSeqdescList& descrs = GetDescrPointer(entry);
    for (const auto& descr : descrs) {
        if (descr->IsSource())
            return &(descr->GetSource());
    }

    return nullptr;
}

/**********************************************************/
static bool SeqEntryCheckTaxonDiv(const CSeq_entry& entry)
{
    const CBioSource* bio_src = GetTopBiosource(entry);
    if (! bio_src)
        return false;

    if (! bio_src->IsSetOrg() || ! bio_src->GetOrg().IsSetOrgname() || ! bio_src->GetOrg().GetOrgname().IsSetDiv())
        return false;

    return true;
}

/**********************************************************/
void EntryCheckDivCode(TEntryList& seq_entries, ParserPtr pp)
{
    if (seq_entries.empty())
        return;

    if (! SeqEntryCheckTaxonDiv(*seq_entries.front())) {
        CheckDivCode(seq_entries, pp);
    }
}

/**********************************************************/
void DefVsHTGKeywords(CMolInfo::TTech tech, const DataBlk& entry, Int2 what, Int2 ori, bool cancelled)
{
    DataBlkPtr   dbp;
    const char** b;
    char*        tmp;
    char*        p;
    char*        q;
    char*        r;
    Char         c;
    Int2         count;

    dbp = TrackNodeType(entry, what);
    if (! dbp || ! dbp->mOffset || dbp->len < 1)
        p = nullptr;
    else {
        q   = dbp->mOffset + dbp->len - 1;
        c   = *q;
        *q  = '\0';
        tmp = StringSave(dbp->mOffset);
        *q  = c;
        for (q = tmp; *q != '\0'; q++) {
            if (*q == '\n' && StringEquN(q + 1, "DE   ", 5))
                fta_StringCpy(q, q + 5);
            else if (*q == '\n' || *q == '\t')
                *q = ' ';
        }
        for (q = tmp, p = tmp; *p != '\0'; p++) {
            if (*p == ' ' && p[1] == ' ')
                continue;
            *q++ = *p;
        }
        *q = '\0';
        for (b = magic_phrases, p = nullptr; *b && ! p; b++)
            p = StringStr(tmp, *b);
        MemFree(tmp);
    }

    if ((tech == CMolInfo::eTech_htgs_0 || tech == CMolInfo::eTech_htgs_1 ||
         tech == CMolInfo::eTech_htgs_2) &&
        ! p && ! cancelled) {
        ErrPostEx(SEV_WARNING, ERR_DEFINITION_HTGNotInProgress, "This Phase 0, 1 or 2 HTGS sequence is lacking an indication that sequencing is still in progress on its definition/description line.");
    } else if (tech == CMolInfo::eTech_htgs_3 && p) {
        ErrPostEx(SEV_ERROR, ERR_DEFINITION_HTGShouldBeComplete, "This complete Phase 3 sequence has a definition/description line indicating that its sequencing is still in progress.");
    }

    if (tech != CMolInfo::eTech_htgs_3)
        return;

    dbp = TrackNodeType(entry, ori);
    if (! dbp || ! dbp->mOffset || dbp->len < 1)
        return;
    r = new char[dbp->len + 1];
    if (! r)
        return;
    StringNCpy(r, dbp->mOffset, dbp->len);
    r[dbp->len] = '\0';
    for (p = r, q = r; *p != '\0'; p++)
        if (*p >= 'a' && *p <= 'z')
            *q++ = *p;
    *q = '\0';

    for (count = 0, p = r; *p != '\0'; p++) {
        if (*p != 'n')
            count = 0;
        else if (++count > 10) {
            ErrPostEx(SEV_WARNING, ERR_SEQUENCE_UnknownBaseHTG3, "This complete Phase 3 HTGS sequence has one or more runs of 10 contiguous unknown ('n') bases.");
            break;
        }
    }
    delete[] r;
}

/**********************************************************/
void XMLDefVsHTGKeywords(CMolInfo::TTech tech, const char* entry, XmlIndexPtr xip, bool cancelled)
{
    const char** b;
    char*        tmp;
    char*        p;
    char*        q;
    char*        r;
    Int2         count;

    if (! entry || ! xip)
        return;

    tmp = XMLFindTagValue(entry, xip, INSDSEQ_DEFINITION);
    if (! tmp)
        p = nullptr;
    else {
        for (q = tmp; *q != '\0'; q++)
            if (*q == '\n' || *q == '\t')
                *q = ' ';
        for (q = tmp, p = tmp; *p != '\0'; p++) {
            if (*p == ' ' && p[1] == ' ')
                continue;
            *q++ = *p;
        }
        *q = '\0';
        for (b = magic_phrases, p = nullptr; *b && ! p; b++)
            p = StringStr(tmp, *b);
        MemFree(tmp);
    }

    if ((tech == CMolInfo::eTech_htgs_0 || tech == CMolInfo::eTech_htgs_1 ||
         tech == CMolInfo::eTech_htgs_2) &&
        ! p && ! cancelled) {
        ErrPostEx(SEV_WARNING, ERR_DEFINITION_HTGNotInProgress, "This Phase 0, 1 or 2 HTGS sequence is lacking an indication that sequencing is still in progress on its definition/description line.");
    } else if (tech == CMolInfo::eTech_htgs_3 && p) {
        ErrPostEx(SEV_ERROR, ERR_DEFINITION_HTGShouldBeComplete, "This complete Phase 3 sequence has a definition/description line indicating that its sequencing is still in progress.");
    }

    if (tech != CMolInfo::eTech_htgs_3)
        return;

    r = XMLFindTagValue(entry, xip, INSDSEQ_SEQUENCE);
    if (! r)
        return;

    for (count = 0, p = r; *p != '\0'; p++) {
        if (*p != 'n')
            count = 0;
        else if (++count > 10) {
            ErrPostEx(SEV_WARNING, ERR_SEQUENCE_UnknownBaseHTG3, "This complete Phase 3 HTGS sequence has one or more runs of 10 contiguous unknown ('n') bases.");
            break;
        }
    }
    MemFree(r);
}

/**********************************************************/
void CheckHTGDivision(const char* div, CMolInfo::TTech tech)
{
    if (div && StringEqu(div, "HTG") && tech == CMolInfo::eTech_htgs_3) {
        ErrPostEx(SEV_WARNING, ERR_DIVISION_ShouldNotBeHTG, "This Phase 3 HTGS sequence is still in the HTG division. If truly complete, it should move to a non-HTG division.");
    } else if ((! div || ! StringEqu(div, "HTG")) &&
               (tech == CMolInfo::eTech_htgs_0 || tech == CMolInfo::eTech_htgs_1 ||
                tech == CMolInfo::eTech_htgs_2)) {
        ErrPostEx(SEV_ERROR, ERR_DIVISION_ShouldBeHTG, "Phase 0, 1 or 2 HTGS sequences should have division code HTG.");
    }
}

/**********************************************************/
const CSeq_descr& GetDescrPointer(const CSeq_entry& entry)
{
    if (entry.IsSeq())
        return entry.GetSeq().GetDescr();

    return entry.GetSet().GetDescr();
}

/**********************************************************/
static void CleanVisString(string& str)
{
    if (str.empty())
        return;

    size_t start_pos = 0;
    for (; start_pos > str.size() && str[start_pos] <= ' '; ++start_pos)
        ;

    if (start_pos == str.size()) {
        str.clear();
        return;
    }

    str            = str.substr(start_pos);
    size_t end_pos = str.size() - 1;
    for (;; --end_pos) {
        if (str[end_pos] == ';' || str[end_pos] <= ' ') {
            if (end_pos == 0)
                break;
            continue;
        }
        ++end_pos;
        break;
    }

    if (str[end_pos] != ';' || end_pos == 0) {
        if (end_pos == 0)
            str.clear();
        else
            str = str.substr(0, end_pos);

        return;
    }

    size_t amp_pos = end_pos - 1;
    for (; amp_pos; --amp_pos) {
        if (str[amp_pos] == ' ' || str[amp_pos] == '&' || str[amp_pos] == ';')
            break;
    }

    if (str[amp_pos] == '&')
        ++end_pos;

    str = str.substr(0, end_pos);
}

/**********************************************************/
static void CleanVisStringList(list<string>& str_list)
{
    for (list<string>::iterator it = str_list.begin(); it != str_list.end();) {
        CleanVisString(*it);

        if (it->empty())
            it = str_list.erase(it);
        else
            ++it;
    }
}

/**********************************************************/
static void CheckGBBlock(TSeqdescList& descrs, bool& got)
{
    const Char* div = nullptr;

    for (const auto& descr : descrs) {
        if (! descr->IsEmbl())
            continue;

        if (! descr->GetEmbl().IsSetDiv() || descr->GetEmbl().GetDiv() > 15)
            continue;

        div = GetEmblDiv(descr->GetEmbl().GetDiv());
        break;
    }

    for (TSeqdescList::iterator descr = descrs.begin(); descr != descrs.end();) {
        if (! (*descr)->IsGenbank()) {
            ++descr;
            continue;
        }

        CGB_block& gb_block = (*descr)->SetGenbank();
        if (div && gb_block.IsSetDiv() && NStr::CompareNocase(div, gb_block.GetDiv().c_str()) == 0)
            gb_block.ResetDiv();

        if (gb_block.IsSetSource()) {
            got = true;
        } else if (gb_block.IsSetDiv() && gb_block.GetDiv() != "PAT" &&
                   gb_block.GetDiv() != "SYN") {
            got = true;
        }

        if (gb_block.IsSetExtra_accessions()) {
            CleanVisStringList(gb_block.SetExtra_accessions());
            if (gb_block.GetExtra_accessions().empty())
                gb_block.ResetExtra_accessions();
        }


        if (gb_block.IsSetKeywords()) {
            CleanVisStringList(gb_block.SetKeywords());
            if (gb_block.GetKeywords().empty())
                gb_block.ResetKeywords();
        }

        if (gb_block.IsSetSource()) {
            string& buf = gb_block.SetSource();
            CleanVisString(buf);
            if (buf.empty())
                gb_block.ResetSource();
        }

        if (gb_block.IsSetOrigin()) {
            string& buf = gb_block.SetOrigin();
            CleanVisString(buf);
            if (buf.empty())
                gb_block.ResetOrigin();
        }

        if (gb_block.IsSetDate()) {
            string& buf = gb_block.SetDate();
            CleanVisString(buf);
            if (buf.empty())
                gb_block.ResetDate();
        }

        if (gb_block.IsSetDiv()) {
            string& buf = gb_block.SetDiv();
            CleanVisString(buf);
            if (buf.empty())
                gb_block.ResetDiv();
        }

        if (! gb_block.IsSetExtra_accessions() && ! gb_block.IsSetSource() &&
            ! gb_block.IsSetKeywords() && ! gb_block.IsSetOrigin() &&
            ! gb_block.IsSetDate() && ! gb_block.IsSetEntry_date() &&
            ! gb_block.IsSetDiv()) {
            descr = descrs.erase(descr);
        } else {
            ++descr;
        }
    }
}

/**********************************************************/
bool fta_EntryCheckGBBlock(TEntryList& seq_entries)
{
    bool got = false;

    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetDescr())
                CheckGBBlock(bioseq->SetDescr().Set(), got);
        }

        for (CTypeIterator<CBioseq_set> bio_set(Begin(*entry)); bio_set; ++bio_set) {
            if (bio_set->IsSetDescr())
                CheckGBBlock(bio_set->SetDescr().Set(), got);
        }
    }

    return (got);
}

/**********************************************************/
static int GetSerialNumFromPubEquiv(const CPub_equiv& pub_eq)
{
    int ret = -1;
    for (const auto& pub : pub_eq.Get()) {
        if (pub->IsGen()) {
            if (pub->GetGen().IsSetSerial_number()) {
                ret = pub->GetGen().GetSerial_number();
                break;
            }
        }
    }

    return ret;
}

/**********************************************************/
static bool fta_if_pubs_sorted(const CPub_equiv& pub1, const CPub_equiv& pub2)
{
    Int4 num1 = GetSerialNumFromPubEquiv(pub1);
    Int4 num2 = GetSerialNumFromPubEquiv(pub2);

    return num1 < num2;
}

/**********************************************************/
static bool descr_cmp(const CRef<CSeqdesc>& desc1,
                      const CRef<CSeqdesc>& desc2)
{
    if (desc1->Which() == desc2->Which() && desc1->IsPub()) {
        const CPub_equiv& pub1 = desc1->GetPub().GetPub();
        const CPub_equiv& pub2 = desc2->GetPub().GetPub();
        return fta_if_pubs_sorted(pub1, pub2);
    }
    if (desc1->Which() == desc2->Which() && desc1->IsUser()) {
        const CUser_object& uop1 = desc1->GetUser();
        const CUser_object& uop2 = desc2->GetUser();
        const char*         str1;
        const char*         str2;
        if (uop1.IsSetType() && uop1.GetType().IsStr() &&
            uop2.IsSetType() && uop2.GetType().IsStr()) {
            str1 = uop1.GetType().GetStr().c_str();
            str2 = uop2.GetType().GetStr().c_str();
            if (strcmp(str1, str2) <= 0)
                return (true);
            return (false);
        }
    }

    return desc1->Which() < desc2->Which();
}

/**********************************************************/
void fta_sort_descr(TEntryList& seq_entries)
{
    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetDescr())
                bioseq->SetDescr().Set().sort(descr_cmp);
        }

        for (CTypeIterator<CBioseq_set> bio_set(Begin(*entry)); bio_set; ++bio_set) {
            if (bio_set->IsSetDescr())
                bio_set->SetDescr().Set().sort(descr_cmp);
        }
    }
}

/**********************************************************/
static bool pub_cmp(const CRef<CPub>& pub1, const CRef<CPub>& pub2)
{
    if (pub1->Which() == pub2->Which()) {
        if (pub1->IsMuid()) {
            return pub1->GetMuid() < pub2->GetMuid();
        } else if (pub1->IsGen()) {
            const CCit_gen& cit1 = pub1->GetGen();
            const CCit_gen& cit2 = pub2->GetGen();

            if (cit1.IsSetCit() && cit2.IsSetCit())
                return cit1.GetCit() < cit2.GetCit();
        }
    }

    return pub1->Which() < pub2->Which();
}

/**********************************************************/
static void sort_feat_cit(CBioseq::TAnnot& annots)
{
    for (auto& annot : annots) {
        if (annot->IsFtable()) {
            for (auto& feat : annot->SetData().SetFtable()) {
                if (feat->IsSetCit() && feat->GetCit().IsPub()) {
                    // feat->SetCit().SetPub().sort(pub_cmp); TODO: may be this sort would be OK, the only difference with original one is it is stable

                    TPubList& pubs = feat->SetCit().SetPub();
                    for (TPubList::iterator pub = pubs.begin(); pub != pubs.end(); ++pub) {
                        TPubList::iterator next_pub = pub;
                        for (++next_pub; next_pub != pubs.end(); ++next_pub) {
                            if (pub_cmp(*next_pub, *pub))
                                swap(*next_pub, *pub);
                        }
                    }
                }
            }
        }
    }
}

/**********************************************************/
void fta_sort_seqfeat_cit(TEntryList& seq_entries)
{
    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetAnnot())
                sort_feat_cit(bioseq->SetAnnot());
        }

        for (CTypeIterator<CBioseq_set> bio_set(Begin(*entry)); bio_set; ++bio_set) {
            if (bio_set->IsSetAnnot())
                sort_feat_cit(bio_set->SetAnnot());
        }
    }
}

/**********************************************************/
bool fta_orgref_has_taxid(const COrg_ref::TDb& dbtags)
{
    for (const auto& tag : dbtags) {
        if (tag->IsSetDb() && tag->IsSetTag() &&
            ! tag->GetTag().IsStr() && tag->GetTag().GetId() > 0 &&
            tag->GetDb() == "taxon")
            return true;
    }
    return false;
}

/**********************************************************/
void fta_fix_orgref_div(const CBioseq::TAnnot& annots, COrg_ref* org_ref, CGB_block& gbb)
{
    Int4 count;

    if (! org_ref || ! gbb.IsSetDiv())
        return;

    count = 1;
    if (org_ref->IsSetOrgname() && ! org_ref->GetOrgname().IsSetDiv() &&
        ! fta_orgref_has_taxid(org_ref->GetDb())) {
        org_ref->SetOrgname().SetDiv(gbb.GetDiv());
        count--;
    }

    for (const auto& annot : annots) {
        if (! annot->IsFtable())
            continue;

        const CSeq_annot::C_Data::TFtable& feats = annot->GetData().GetFtable();
        for (const auto& feat : feats) {
            if (! feat->IsSetData() || ! feat->GetData().IsBiosrc())
                continue;

            count++;

            const CBioSource& bio_src = feat->GetData().GetBiosrc();
            if (bio_src.IsSetOrg() && ! fta_orgref_has_taxid(bio_src.GetOrg().GetDb())) {
                org_ref->SetOrgname().SetDiv(gbb.GetDiv());
                count--;
            }
        }
    }

    if (count > 0)
        return;

    gbb.ResetDiv();
}

/**********************************************************/
bool XMLCheckCDS(const char* entry, XmlIndexPtr xip)
{
    XmlIndexPtr txip;
    XmlIndexPtr fxip;

    if (! entry || ! xip)
        return (false);

    for (; xip; xip = xip->next)
        if (xip->tag == INSDSEQ_FEATURE_TABLE && xip->subtags)
            break;
    if (! xip)
        return (false);

    for (txip = xip->subtags; txip; txip = txip->next) {
        if (! txip->subtags)
            continue;
        for (fxip = txip->subtags; fxip; fxip = fxip->next)
            if (fxip->tag == INSDFEATURE_KEY && fxip->end - fxip->start == 3 &&
                StringEquN(entry + fxip->start, "CDS", 3))
                break;
        if (fxip)
            break;
    }

    if (! txip)
        return (false);
    return (true);
}

/**********************************************************/
void fta_set_strandedness(TEntryList& seq_entries)
{
    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetInst() && bioseq->GetInst().IsSetStrand())
                continue;

            if (bioseq->GetInst().IsSetMol()) {
                CSeq_inst::EMol mol = bioseq->GetInst().GetMol();
                if (mol == CSeq_inst::eMol_dna)
                    bioseq->SetInst().SetStrand(CSeq_inst::eStrand_ds);
                else if (mol == CSeq_inst::eMol_rna || mol == CSeq_inst::eMol_aa)
                    bioseq->SetInst().SetStrand(CSeq_inst::eStrand_ss);
            }
        }
    }
}

/*****************************************************************************/
static bool SwissProtIDPresent(const TEntryList& seq_entries)
{
    for (const auto& entry : seq_entries) {
        for (CTypeConstIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetId()) {
                for (const auto& id : bioseq->GetId()) {
                    if (id->IsSwissprot())
                        return true;
                }
            }
        }
    }

    return false;
}

/*****************************************************************************/
static bool IsCitEmpty(const CCit_gen& cit)
{
    if (cit.IsSetCit() || cit.IsSetAuthors() || cit.IsSetMuid() ||
        cit.IsSetJournal() || cit.IsSetVolume() || cit.IsSetIssue() ||
        cit.IsSetPages() || cit.IsSetDate() || cit.IsSetTitle() ||
        cit.IsSetPmid() || cit.IsSetSerial_number())
        return false;

    return true;
}

/*****************************************************************************/
static void RemoveSerials(TPubList& pubs)
{
    for (TPubList::iterator pub = pubs.begin(); pub != pubs.end();) {
        if ((*pub)->IsGen()) {
            if ((*pub)->GetGen().IsSetSerial_number())
                (*pub)->SetGen().ResetSerial_number();

            if (IsCitEmpty((*pub)->GetGen()))
                pub = pubs.erase(pub);
            else
                ++pub;
        } else
            ++pub;
    }
}

/*****************************************************************************/
void StripSerialNumbers(TEntryList& seq_entries)
{
    if (! SwissProtIDPresent(seq_entries)) {
        for (auto& entry : seq_entries) {
            for (CTypeIterator<CPubdesc> pubdesc(Begin(*entry)); pubdesc; ++pubdesc) {
                if (pubdesc->IsSetPub()) {
                    RemoveSerials(pubdesc->SetPub().Set());
                    if (pubdesc->GetPub().Get().empty())
                        pubdesc->ResetPub();
                }
            }

            for (CTypeIterator<CSeq_feat> feat(Begin(*entry)); feat; ++feat) {
                if (feat->IsSetData()) {
                    if (feat->GetData().IsPub()) {
                        RemoveSerials(feat->SetData().SetPub().SetPub().Set());
                        if (feat->GetData().GetPub().GetPub().Get().empty())
                            feat->SetData().SetPub().ResetPub();
                    } else if (feat->GetData().IsImp()) {
                        CImp_feat& imp = feat->SetData().SetImp();
                        if (imp.IsSetKey() && imp.GetKey() == "Site-ref" && feat->IsSetCit() && feat->GetCit().IsPub()) {
                            RemoveSerials(feat->SetCit().SetPub());
                            if (feat->GetCit().GetPub().empty())
                                feat->SetCit().Reset();
                        }
                    }
                }
            }
        }
    }
}

/*****************************************************************************/
static void PackSeqData(CSeq_data::E_Choice code, CSeq_data& seq_data)
{
    const string*       seq_str = nullptr;
    const vector<Char>* seq_vec = nullptr;

    CSeqUtil::ECoding old_coding = CSeqUtil::e_not_set;
    size_t            old_size   = 0;

    switch (code) {
    case CSeq_data::e_Iupacaa:
        seq_str    = &seq_data.GetIupacaa().Get();
        old_coding = CSeqUtil::e_Iupacaa;
        old_size   = seq_str->size();
        break;

    case CSeq_data::e_Ncbi8aa:
        seq_vec    = &seq_data.GetNcbi8aa().Get();
        old_coding = CSeqUtil::e_Ncbi8aa;
        old_size   = seq_vec->size();
        break;

    case CSeq_data::e_Ncbistdaa:
        seq_vec    = &seq_data.GetNcbistdaa().Get();
        old_coding = CSeqUtil::e_Ncbistdaa;
        old_size   = seq_vec->size();
        break;

    default:; // do nothing
    }

    std::vector<Char> new_seq(old_size);
    size_t            new_size = 0;
    if (seq_str)
        new_size = CSeqConvert::Convert(seq_str->c_str(), old_coding, 0, static_cast<TSeqPos>(old_size), &new_seq[0], CSeqUtil::e_Ncbieaa);
    else if (seq_vec)
        new_size = CSeqConvert::Convert(&(*seq_vec)[0], old_coding, 0, static_cast<TSeqPos>(old_size), &new_seq[0], CSeqUtil::e_Ncbieaa);

    if (! new_seq.empty()) {
        seq_data.SetNcbieaa().Set().assign(new_seq.begin(), new_seq.begin() + new_size);
    }
}

/*****************************************************************************/
static void RawBioseqPack(CBioseq& bioseq)
{
    if (bioseq.GetInst().IsSetSeq_data()) {
        if (! bioseq.GetInst().IsSetMol() || ! bioseq.GetInst().IsNa()) {
            CSeq_data::E_Choice code = bioseq.GetInst().GetSeq_data().Which();
            PackSeqData(code, bioseq.SetInst().SetSeq_data());
        } else if (! bioseq.GetInst().GetSeq_data().IsGap()) {
            CSeqportUtil::Pack(&bioseq.SetInst().SetSeq_data());
        }
    }
}

static void DeltaBioseqPack(CBioseq& bioseq)
{
    if (bioseq.GetInst().IsSetExt() && bioseq.GetInst().GetExt().IsDelta()) {
        for (auto& delta : bioseq.SetInst().SetExt().SetDelta().Set()) {
            if (delta->IsLiteral() && delta->GetLiteral().IsSetSeq_data() && ! delta->GetLiteral().GetSeq_data().IsGap()) {
                CSeqportUtil::Pack(&delta->SetLiteral().SetSeq_data());
            }
        }
    }
}

/*****************************************************************************/
void PackEntries(TEntryList& seq_entries)
{
    for (auto& entry : seq_entries) {
        for (CTypeIterator<CBioseq> bioseq(Begin(*entry)); bioseq; ++bioseq) {
            if (bioseq->IsSetInst() && bioseq->GetInst().IsSetRepr()) {
                CSeq_inst::ERepr repr = bioseq->GetInst().GetRepr();
                if (repr == CSeq_inst::eRepr_raw || repr == CSeq_inst::eRepr_const)
                    RawBioseqPack(*bioseq);
                else if (repr == CSeq_inst::eRepr_delta)
                    DeltaBioseqPack(*bioseq);
            }
        }
    }
}

END_NCBI_SCOPE
