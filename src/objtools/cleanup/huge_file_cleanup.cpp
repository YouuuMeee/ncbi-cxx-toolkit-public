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
* File Description:
*
*/
#include <ncbi_pch.hpp>

#include <objtools/edit/huge_file_process.hpp>
#include <objtools/cleanup/huge_file_cleanup.hpp>
#include <objtools/cleanup/cleanup_change.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include "newcleanupp.hpp"
#include "influenza_set.hpp" 

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
USING_SCOPE(edit);


CCleanupHugeAsnReader::CCleanupHugeAsnReader(TOptions options) 
: m_CleanupOptions(options) {}


const CCleanupChangeCore& CCleanupHugeAsnReader::GetChanges() const
{
    return m_Changes;
}


void CCleanupHugeAsnReader::FlattenGenbankSet() 
{
    auto& biosets = m_bioseq_set_list;
    if (biosets.size()>1) {
        auto firstTrueBioset = next(biosets.begin());
        if (firstTrueBioset->m_class == CBioseq_set::eClass_not_set) {
            if (x_LooksLikeNucProtSet()) {
                firstTrueBioset->m_class = CBioseq_set::eClass_nuc_prot; 
            }
            else {
                firstTrueBioset->m_class = CBioseq_set::eClass_genbank;
            }
            m_Changes.SetChanged(CCleanupChange::eChangeBioseqSetClass);
        }
    }
    TParent::FlattenGenbankSet();
    x_CleanupTopLevelDescriptors();

    if (m_CleanupOptions & eEnableSmallGenomeSets) {
        x_CreateSmallGenomeSets();
        set<string> fluLabels;
        auto it = m_top_ids.begin();
        while (it != m_top_ids.end()) {
            if (auto mit = m_IdToFluLabel.find(*it);
                mit != m_IdToFluLabel.end()) {
                if (!fluLabels.insert(mit->second).second) {
                    it = m_top_ids.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}


bool CCleanupHugeAsnReader::x_LooksLikeNucProtSet() const
{
    int numNucs{0};
    int numProts{0};
    for (const auto& bioseq : GetBioseqs()) {
        if (CSeq_inst::IsNa(bioseq.m_mol)) {
            ++numNucs;
            if (numNucs>1) {
                return false;
            }
        }
        else if (CSeq_inst::IsAa(bioseq.m_mol)) {
            ++numProts;
        }
    }
    if (numProts==0) {
        return false;
    }
    
    _ASSERT(m_bioseq_set_list.size()>1);
    // check for invalid subsets
    auto it = next(m_bioseq_set_list.begin(),2);
    while (it != m_bioseq_set_list.end()) {
        if (it->m_class != CBioseq_set::eClass_segset &&
            it->m_class != CBioseq_set::eClass_parts) {
            return false;
        }
        ++it;
    }
    return true;
}


void CCleanupHugeAsnReader::x_CleanupTopLevelDescriptors() 
{

    m_TopLevelBiosources.clear();
    m_pTopLevelMolInfo.Reset();


    if (!m_top_entry || 
        !m_top_entry->IsSetDescr() ||
        !m_top_entry->GetDescr().IsSet()) {
        return;
    }

    CCleanup cleanup;
    m_Changes += *cleanup.BasicCleanup(m_top_entry->SetDescr());
    
    if (!(m_CleanupOptions & eExtendedCleanup)) {
        return;
    }

    auto& descriptors = m_top_entry->SetDescr().Set();
    auto it = descriptors.begin();
    while (it != descriptors.end()) {
        if (it->Empty()) {
            it = descriptors.erase(it);
        }
        else if ((*it)->IsSource()) {
            m_TopLevelBiosources.push_back(*it);
            it = descriptors.erase(it);
            m_Changes.SetChanged(CCleanupChange::eAddDescriptor);
            m_Changes.SetChanged(CCleanupChange::eRemoveDescriptor);
        }
        else if ((*it)->IsMolinfo()) {
            if (!m_pTopLevelMolInfo) {
                m_pTopLevelMolInfo.Reset(&(**it));
            }
            it = descriptors.erase(it);
            m_Changes.SetChanged(CCleanupChange::eRemoveDescriptor);
        } else {
            ++it;
        }
    }

    if (!(m_CleanupOptions & eNoNcbiUserObjects)) {
        CNewCleanup_imp::AddNcbiCleanupObject(m_top_entry->SetDescr());
        m_Changes.SetChanged(CCleanupChange::eAddNcbiCleanupObject);
    }


    if (descriptors.empty()) {
        m_top_entry->SetSet().ResetDescr();
    } 
    else if (CCleanup::NormalizeDescriptorOrder(m_top_entry->SetDescr())) { 
        m_Changes.SetChanged(CCleanupChange::eMoveDescriptor);       
    }
}


void CCleanupHugeAsnReader::x_AddTopLevelDescriptors(CSeq_entry& entry) const
{
    if (!(m_CleanupOptions & eExtendedCleanup) ||
        (m_TopLevelBiosources.empty() && m_pTopLevelMolInfo.Empty())) {
        return;
    }

    bool addMolInfo = false;
    if (m_pTopLevelMolInfo &&
        entry.IsSetDescr() &&
        entry.GetDescr().IsSet()) {
        const auto& descriptors = entry.GetDescr().Get();
        auto it = find_if(descriptors.begin(), descriptors.end(),
                [](const CRef<CSeqdesc>& pDesc) {
                    return (pDesc && pDesc->IsMolinfo());
                });
        if (it == descriptors.end()) {
            addMolInfo = true;
        }
    }

    for (auto pSource : m_TopLevelBiosources) {
        entry.SetDescr().Set().push_back(pSource);
    }

    if (addMolInfo) {
        entry.SetDescr().Set().push_back(m_pTopLevelMolInfo);
        m_Changes.SetChanged(CCleanupChange::eAddDescriptor);
    }
}


CRef<CSeq_entry> CCleanupHugeAsnReader::LoadSeqEntry(
        const TBioseqSetInfo& info,
        eAddTopEntry add_top_entry) const
{
    if (m_CleanupOptions & eEnableSmallGenomeSets) {
        auto it = m_SetPosToFluLabel.find(info.m_pos);
        if (it != m_SetPosToFluLabel.end()) {
            auto pSmallGenomeEntry = Ref(new CSeq_entry());
            pSmallGenomeEntry->SetSet().SetClass() = CBioseq_set::eClass_small_genome_set;
            for (const auto& setInfo : m_FluLabelToSetInfo.at(it->second)) {
                auto pSubEntry = TParent::LoadSeqEntry(setInfo, eAddTopEntry::no);
                pSmallGenomeEntry->SetSet().SetSeq_set().push_back(pSubEntry);
            }
            if (add_top_entry == eAddTopEntry::yes) {
                x_AddTopLevelDescriptors(*pSmallGenomeEntry);
            }
            return pSmallGenomeEntry;
        }
    }
    
    auto pEntry = TParent::LoadSeqEntry(info, eAddTopEntry::no);
    if (add_top_entry == eAddTopEntry::yes) {
        x_AddTopLevelDescriptors(*pEntry);
    }
    return pEntry;
}


static string s_GetInfluenzaKey(const CSeq_descr& descr)
{
    if (descr.IsSet()) {
        for (const auto& pDesc : descr.Get()) {
            if (pDesc->IsSource()) {
                const auto& source = pDesc->GetSource();
                if (source.IsSetOrg()) {
                    auto key = CInfluenzaSet::GetKey(source.GetOrg());
                    if (!NStr::IsBlank(key)) {
                        return key;
                    }
                }
            }
        }
    }
    return "";    
}

template<typename TMap>
static void s_RemoveEntriesWithKey(const string& key, TMap& mapToKey)
{
    if (mapToKey.empty()) {
        return;
    }

    auto it = mapToKey.begin();
    while (it != mapToKey.end()) {
        if (it->second == key) {
            it = mapToKey.erase(it);
        } else {
            ++it;
        }
    }
}

static bool s_CheckForSegments(CConstRef<CSeq_descr> seqDescrs, 
        CConstRef<CSeq_descr> setDescrs,
        const string& key,
        set<size_t>& segments)
{
    if (NStr::IsBlank(key)) {
        return false;
    }
    auto fluType = CInfluenzaSet::GetInfluenzaType(key);
    if (fluType == CInfluenzaSet::eNotInfluenza) {
        return false;
    }

    auto numRequired = CInfluenzaSet::GetNumRequired(fluType);  

    if (seqDescrs && seqDescrs->IsSet()) {
        for (auto pDesc : seqDescrs->Get()) {
            if (pDesc->IsSource()) {
                return g_FindSegs(pDesc->GetSource(), numRequired, segments);
            }
        }
    }

    if (setDescrs && setDescrs->IsSet()) {
        for (auto pDesc : setDescrs->Get()) {
            if (pDesc->IsSource()) {
                return g_FindSegs(pDesc->GetSource(), numRequired, segments);
            }
        }
    }
    return false;
}

void CCleanupHugeAsnReader::x_CreateSmallGenomeSets()
{
    map<string, set<size_t>> keyToSegs;
    CConstRef<CSeq_descr> pNpSetDescr;
    for (const auto& bioseqInfo : GetBioseqs()) {
        if (!CSeq_inst::IsNa(bioseqInfo.m_mol)) {
            continue;
        }
        auto parent = bioseqInfo.m_parent_set;
        if (parent->m_class == CBioseq_set::eClass_nuc_prot) {
            pNpSetDescr = parent->m_descr;
            parent = parent->m_parent_set;
        }
        if (!IsHugeSet(parent->m_class)) {
            continue;
        }
        string key;
        if (bioseqInfo.m_descr) {
            key = s_GetInfluenzaKey(*(bioseqInfo.m_descr));
            if (NStr::IsBlank(key) && pNpSetDescr) {
                key = s_GetInfluenzaKey(*pNpSetDescr);
            }
        }
        if (!NStr::IsBlank(key)) {
            bool makeSmallGenomeSet = 
                s_CheckForSegments(bioseqInfo.m_descr, pNpSetDescr, key, keyToSegs[key]);

            if (makeSmallGenomeSet) {
                const auto& setInfo = *FindTopObject(bioseqInfo.m_ids.front());
                m_FluLabelToSetInfo[key].push_back(setInfo);
                m_SetPosToFluLabel[setInfo.m_pos] = key;
                for (auto pId : bioseqInfo.m_ids) {
                    m_IdToFluLabel[pId] = key;
                }
            }
        }
    }

    // Prune if there are missing segments
    for (auto entry : keyToSegs) {
        const auto& key = entry.first;
        if (auto it = m_FluLabelToSetInfo.find(key); it != m_FluLabelToSetInfo.end()) {
            auto fluType = CInfluenzaSet::GetInfluenzaType(key);
            auto numRequired = CInfluenzaSet::GetNumRequired(fluType);
            if (entry.second.size() != numRequired) {
                m_FluLabelToSetInfo.erase(it);
                s_RemoveEntriesWithKey(key, m_SetPosToFluLabel);
                s_RemoveEntriesWithKey(key, m_IdToFluLabel);
            } 
        }
    }
}


string CCleanupHugeAsnReader::x_GetFluLabel(const CConstRef<CSeq_id>& pId) const
{
    auto it = m_IdToFluLabel.lower_bound(pId);
    if (it != m_IdToFluLabel.end()) {
        if (it->first->CompareOrdered(*pId) == 0 ||
            it->first->Compare(*pId) == CSeq_id::E_SIC::e_YES) {
            return it->second;
        }
    }
    return  "";
}


END_SCOPE(objects)
END_NCBI_SCOPE
