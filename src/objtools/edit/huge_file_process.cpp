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
* Author:  Sergiy Gotvyanskyy
* File Description:
*   Utility class for processing ASN.1 files using Huge Files approach
*
*/
#include <ncbi_pch.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objtools/edit/huge_file.hpp>
#include <objtools/edit/huge_asn_reader.hpp>
#include <objtools/edit/huge_file_process.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)

class CHugeFileProcessImpl
{
public:
    CHugeFileProcessImpl(const string& file_name)
    : m_file{new CHugeFile},
      m_reader{new CHugeAsnReader}
    {
        static set<TTypeInfo> supported_types =
        {
            CBioseq_set::GetTypeInfo(),
            CBioseq::GetTypeInfo(),
            CSeq_entry::GetTypeInfo(),
            CSeq_submit::GetTypeInfo(),
        };
        m_file->m_supported_types = &supported_types;

        m_file->Open(file_name);
        m_reader->Open(m_file.get(), nullptr);
    }

    CRef<CSeq_entry> GetNextEntry()
    {
        if (m_current == m_flattened.end())
        {
            m_flattened.clear();
            m_current = m_flattened.end();
            return {};
        }
        else
        {
            return m_reader->LoadSeqEntry(*m_current++);
        }
    }

    void FlattenGenbankSet()
    {
        m_flattened.clear();


        for (auto& rec: m_reader->GetBioseqs())
        {
            auto parent = rec.m_parent_set;
            
            if (auto _class = parent->m_class; 
                    x_ShouldSplitSet(_class))
            { // create fake bioseq_set
                m_flattened.push_back({rec.m_pos, m_reader->GetBiosets().end(), objects::CBioseq_set::eClass_not_set, {} });
                continue;
            }
         
            auto grandParent = parent->m_parent_set;
            while (!x_ShouldSplitSet(grandParent->m_class)) {
                parent = grandParent;
                grandParent = grandParent->m_parent_set;
            }  
            if (m_flattened.empty() || (m_flattened.back().m_pos != parent->m_pos)) {
                m_flattened.push_back(*parent);
            }
        }


        if (m_reader->GetBiosets().size()>1)
        {
            auto top = next(m_reader->GetBiosets().begin());
            if (m_flattened.size() == 1) {
                // exposing the whole top entry
                if (m_reader->GetSubmitBlock().NotEmpty()
                    || (top->m_class != CBioseq_set::eClass_genbank)
                    || top->m_descr.NotEmpty())
                {
                    m_flattened.clear();
                    m_flattened.push_back(*top);
                }
            }
            else { // m_flattened.size() > 1)
                m_top_entry = Ref(new CSeq_entry());
                m_top_entry->SetSet().SetClass() = top->m_class;
                if (top->m_descr) {
                    m_top_entry->SetSet().SetDescr().Assign(*top->m_descr);
                }
            }
        }


        m_current = m_flattened.begin();
    }

private:

    static bool x_ShouldSplitSet(CBioseq_set::EClass setClass) {
        return setClass == CBioseq_set::eClass_not_set ||
               setClass == CBioseq_set::eClass_genbank;
    }

    unique_ptr<CHugeFile> m_file;
    CHugeAsnReader::TBioseqSetIndex m_flattened;
    CHugeAsnReader::TBioseqSetIndex::iterator m_current;
public:
    CRef<CSeq_entry> m_top_entry;
    unique_ptr<CHugeAsnReader> m_reader;
};

CHugeFileProcess::CHugeFileProcess(const string& file_name)
    :m_Impl{new CHugeFileProcessImpl(file_name)}
{
}

CHugeFileProcess::~CHugeFileProcess(void)
{
}

bool CHugeFileProcess::Read(THandler handler, CRef<CSeq_id> seqid)
{
    if (!m_Impl->m_reader->GetNextBlob()) {
        return false;
    }   

    do
    {
        m_Impl->FlattenGenbankSet();
        CRef<CSeq_entry> entry;

        do
        {
            entry.Reset();

            if (seqid.Empty())
                entry = m_Impl->GetNextEntry();
            else
            {
                auto seq = m_Impl->m_reader->LoadBioseq(seqid);
                if (seq.NotEmpty())
                {
                    entry = Ref(new CSeq_entry);
                    entry->SetSeq(*seq);
                }
            }

            if (entry)
            {
                if (m_Impl->m_top_entry) {
                    auto pNewEntry = Ref(new CSeq_entry());
                    pNewEntry->Assign(*m_Impl->m_top_entry);
                    pNewEntry->SetSet().SetSeq_set().push_back(entry);
                    entry = pNewEntry;
                }

                handler(m_Impl->m_reader->GetSubmitBlock(), entry);
            }
        }
        while ( entry && seqid.Empty());
    } while (m_Impl->m_reader->GetNextBlob());

    return true;
}

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE
