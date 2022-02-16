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
 * Author:
 *
 * File Description:
 *
 * ===========================================================================
 */

#ifndef _PUBMED_UPDATER_HPP_
#define _PUBMED_UPDATER_HPP_

#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CPub;
class CTitle_msg;
class CTitle_msg_list;

BEGIN_SCOPE(edit)

using EPubmedError = int;

class NCBI_XOBJEDIT_EXPORT IPubmedUpdater
{
public:
    virtual ~IPubmedUpdater() {}
    virtual bool Init() { return true; }
    virtual void Fini() {}
    virtual TEntrezId CitMatch(const CPub&) = 0;
    virtual CRef<CPub> GetPub(TEntrezId pmid, EPubmedError* = nullptr) = 0;
    virtual CRef<CTitle_msg_list> GetTitle(const CTitle_msg&) = 0;
};

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif // _PUBMED_UPDATER_HPP_
