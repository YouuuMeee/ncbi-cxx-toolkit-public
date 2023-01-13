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
 * File Name: ftanet.h
 *
 * Author: Alexey Dobronadezhdin
 *
 * File Description:
 *
 */

#ifndef FTANET_H
#define FTANET_H
#include <objtools/flatfile/flatfile_parse_info.hpp>


BEGIN_NCBI_SCOPE

namespace objects
{
    class COrg_ref;
    class CSeq_id;
    class CCit_art;
};

CRef<objects::COrg_ref> fta_fix_orgref_byid(ParserPtr pp, TTaxId taxid, bool* drop, bool isoh);

void fta_find_pub_explore(ParserPtr pp, TEntryList& seq_entries);
void fta_fill_find_pub_option(ParserPtr pp, bool htag, bool rtag);
Int4 fta_is_con_div(ParserPtr pp, const objects::CSeq_id& id, const Char* acc);
void fta_fix_orgref(ParserPtr pp, objects::COrg_ref& org_ref, bool* drop, char* organelle);

END_NCBI_SCOPE

#endif // FTANET_H
