/* ftamed.c
 *
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
 * File Name:  ftamed.cpp
 *
 * Author:
 *
 * File Description:
 * -----------------
 *      Pubmed lookup and post-processing utilities.
 *
 */
#include <ncbi_pch.hpp>

#include <objects/mla/Title_msg.hpp>
#include <objects/mla/Title_msg_list.hpp>
#include <objects/mla/mla_client.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/pub/Pub.hpp>
#include <objtools/edit/pub_fix.hpp>

#include "ftaerr.hpp"
#include "ftamed.h"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* this_module = "medarch";
#ifdef THIS_MODULE
#undef THIS_MODULE
#endif
#define THIS_MODULE this_module

IMessageListener::EPostResult
CPubFixMessageListener::PostMessage(const IMessage& message)
{
    static const map<EDiagSev, ErrSev> sSeverityMap
               = {{eDiag_Trace, SEV_NONE},
                  {eDiag_Info, SEV_INFO},
                  {eDiag_Warning, SEV_WARNING},
                  {eDiag_Error, SEV_ERROR},
                  {eDiag_Critical, SEV_REJECT},
                  {eDiag_Fatal, SEV_FATAL}};

    ErrPostEx(sSeverityMap.at(message.GetSeverity()),
            message.GetCode(),      // fix_pub::EFixPubErrorCategory
            message.GetSubCode(),   // fix_pub::EFixPubReferenceError
            message.GetText().c_str());

    return eHandled;
}

/**********************************************************/
class CMLAUpdater : public edit::IPubmedUpdater
{
    CMLAClient m_mla;

public:
    bool Init() override
    {
        CMla_back resp;
        try {
            m_mla.AskInit(&resp);
        } catch (...) {
            return false;
        }
        return resp.IsInit();
    }

    void Fini() override
    {
        try {
            m_mla.AskFini();
        } catch (...) {
        }
    }

    TEntrezId GetPmId(const CPub& pub) override
    {
        return ENTREZ_ID_FROM(int, m_mla.AskCitmatchpmid(pub));
    }

    CRef<CPub> GetPub(TEntrezId pmid) override
    {
        return m_mla.AskGetpubpmid(CPubMedId(pmid));
    }

    CRef<CTitle_msg_list> GetTitle(const CTitle_msg& msg) override
    {
        return m_mla.AskGettitle(msg);
    }
};

static CMLAUpdater mlaupdater;

/**********************************************************/
edit::IPubmedUpdater* GetPubmedClient()
{
    return &mlaupdater;
}

/**********************************************************/
CRef<CCit_art> FetchPubPmId(TEntrezId pmid)
{
    return edit::CPubFix::FetchPubPmId(pmid, GetPubmedClient());
}

END_NCBI_SCOPE
