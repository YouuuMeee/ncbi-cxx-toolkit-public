#ifndef SPLIT_INFO_CACHE__HPP
#define SPLIT_INFO_CACHE__HPP

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
 * Authors: Sergey Satskiy
 *
 * File Description:
 *
 */


#include <chrono>
#include <atomic>
#include <map>
#include <vector>
using namespace std;

#include "cass_blob_id.hpp"
#include "split_info_utils.hpp"
#include <objects/seqsplit/seqsplit__.hpp>

USING_NCBI_SCOPE;

using namespace ncbi::objects;

struct SSplitInfoCacheItem
{
    public:
        SSplitInfoCacheItem(CRef<CID2S_Split_Info> blob) :
            m_LastTouch(std::chrono::steady_clock::now()),
            m_SplitInfoBlob(blob)
        {}

        SSplitInfoCacheItem()
        {}

        ~SSplitInfoCacheItem()
        {}

    public:
        // For the future - if the cache needs to be purged
        std::chrono::time_point<std::chrono::steady_clock>  m_LastTouch;
        CRef<CID2S_Split_Info>                              m_SplitInfoBlob;
};



class CSplitInfoCache
{
    public:
        CSplitInfoCache() :
            m_Lock(false)
        {}

        ~CSplitInfoCache()
        {}

    public:
        pair<bool, CRef<CID2S_Split_Info>>
                                GetBlob(const SCass_BlobId &  info_blob_id);
        void                    AddBlob(const SCass_BlobId &  info_blob_id,
                                        CRef<CID2S_Split_Info>);

        size_t Size(void)
        {
            size_t size = 0;
            while (m_Lock.exchange(true)) {}    // acquire top level lock
            size = m_Cache.size();
            m_Lock = false;                     // release top level lock
            return size;
        }

    private:
        map<SCass_BlobId, SSplitInfoCacheItem>      m_Cache;
        atomic<bool>                                m_Lock;
};


#endif
