#ifndef _VALIDATOR_CONTEXT_HPP_
#define _VALIDATOR_CONTEXT_HPP_

/*  $Id$
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
 * Author:  Justin Foley
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <atomic>
#include <mutex>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CTaxon3_reply;
class COrg_ref;

BEGIN_SCOPE(validator)

struct NCBI_VALIDATOR_EXPORT SValidatorContext
{
    bool        PreprocessHugeFile{false};
    bool        PostprocessHugeFile{false};
    bool        IsPatent{false};
    bool        IsPDB{false};
    bool        IsRefSeq{false};
    string      GenbankSetId;
    atomic_bool CheckECNumFileStatus{true};
    bool        NoBioSource{false};
    bool        NoPubsFound{false};
    bool        NoCitSubsFound{false};
    once_flag   DescriptorsOnceFlag;
    once_flag   SubmitBlockOnceFlag;

    using taxupdate_func_t = function<CRef<CTaxon3_reply>(const vector<CRef<COrg_ref>>& list)>;
    taxupdate_func_t m_taxon_update;
};

END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif
