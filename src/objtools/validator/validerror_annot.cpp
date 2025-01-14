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
 * Author:  Jonathan Kans, Clifford Clausen, Aaron Ucko......
 *
 * File Description:
 *   validation of seq_annot
 *   .......
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <objtools/validator/validerror_annot.hpp>
#include <objtools/validator/utilities.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Annot_descr.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(validator)


CValidError_annot::CValidError_annot(CValidError_imp& imp) :
    CValidError_base(imp),
    m_GraphValidator(imp),
    m_AlignValidator(imp),
    m_FeatValidator(imp)
{
}


CValidError_annot::~CValidError_annot()
{
}


void CValidError_annot::ValidateSeqAnnot(const CSeq_annot_Handle& annot)
{
    if (annot.IsAlign()) {
        if (annot.Seq_annot_IsSetDesc()) {
            for (const CRef<CAnnotdesc>& iter : annot.Seq_annot_GetDesc().Get()) {
                if (iter->IsUser()) {
                    const CObject_id& oid = iter->GetUser().GetType();
                    if (oid.IsStr()) {
                        if (oid.GetStr() == "Blast Type") {
                            PostErr(eDiag_Error, eErr_SEQ_ALIGN_BlastAligns,
                                "Record contains BLAST alignments", *annot.GetCompleteSeq_annot()); // !!!
                            break;
                        }
                    }
                }
            }
        }
    } else if (annot.IsIds()) {
        PostErr(eDiag_Error, eErr_SEQ_ANNOT_AnnotIDs,
                "Record should not contain Seq-annot.data.ids", *annot.GetCompleteSeq_annot());
    } else if (annot.IsLocs()) {
        PostErr(eDiag_Error, eErr_SEQ_ANNOT_AnnotLOCs,
                "Record contains Seq-annot.data.locs", *annot.GetCompleteSeq_annot());
    }
}


void CValidError_annot::ValidateSeqAnnot(const CSeq_annot& annot)
{
    if (annot.IsAlign()) {
        if (annot.IsSetDesc()) {
            for (const CRef<CAnnotdesc>& iter : annot.GetDesc().Get()) {
                if (iter->IsUser()) {
                    const CObject_id& oid = iter->GetUser().GetType();
                    if (oid.IsStr()) {
                        if (oid.GetStr() == "Blast Type") {
                            PostErr(eDiag_Error, eErr_SEQ_ALIGN_BlastAligns,
                                "Record contains BLAST alignments", annot); // !!!
                            break;
                        }
                    }
                }
            }
        }
        if (m_Imp.IsValidateAlignments()) {
            int order = 1;
            for (const auto& align : annot.GetData().GetAlign()) {
                m_AlignValidator.ValidateSeqAlign(*align, order++);
            }
        }
    } else if (annot.IsIds()) {
        PostErr(eDiag_Error, eErr_SEQ_ANNOT_AnnotIDs,
                "Record contains Seq-annot.data.ids", annot);
    } else if (annot.IsLocs()) {
        PostErr(eDiag_Error, eErr_SEQ_ANNOT_AnnotLOCs,
                "Record contains Seq-annot.data.locs", annot);
    } else if (annot.IsGraph()) {
        for (const auto& graph : annot.GetData().GetGraph()) {
            m_GraphValidator.ValidateSeqGraph(*graph);
        }
    } else if (annot.IsFtable()) {
        CSeq_entry_Handle appropriate_parent;
        if (m_Imp.ShouldSubdivide() && m_Scope) {
            CSeq_annot_Handle ah = m_Scope->GetSeq_annotHandle(annot);
            if (ah) {
                CSeq_entry_Handle seh = ah.GetParentEntry();
                if (seh) {
                    appropriate_parent = GetAppropriateXrefParent(seh);
                }
            }
        }
        if (appropriate_parent) {
            CRef<CScope> tmp_scope(new CScope(*(CObjectManager::GetInstance())));
            tmp_scope->AddDefaults();
            CSeq_entry_Handle this_seh = tmp_scope->AddTopLevelSeqEntry(*(appropriate_parent.GetCompleteSeq_entry()));
            m_FeatValidator.SetScope(*tmp_scope);
            m_FeatValidator.SetTSE(this_seh);
            for (const auto& feat : annot.GetData().GetFtable()) {
                m_FeatValidator.ValidateSeqFeat(*feat);
            }
            m_FeatValidator.SetScope(*m_Scope);
            m_FeatValidator.SetTSE(m_Imp.GetTSEH());
        } else {
            m_FeatValidator.SetScope(*m_Scope);
            m_FeatValidator.SetTSE(m_Imp.GetTSEH());
            for (const auto& feat : annot.GetData().GetFtable()) {
                m_FeatValidator.ValidateSeqFeat(*feat);
            }
        }
    }
}


void CValidError_annot::ValidateSeqAnnotContext(const CSeq_annot& annot, const CBioseq& seq)
{
    if (annot.IsGraph()) {
        for (const auto& graph : annot.GetData().GetGraph()) {
            m_GraphValidator.ValidateSeqGraphContext(*graph, seq);
        }
    } else if (annot.IsFtable()) {
        for (const auto& feat_it : annot.GetData().GetFtable()) {
            string label = seq.GetId().front()->AsFastaString();
            ReportLocationGI0(*feat_it, label);
            if (! feat_it->IsSetLocation() || IsLocationUnindexed(feat_it->GetLocation())) {
                m_Imp.PostErr(eDiag_Error, eErr_SEQ_FEAT_UnindexedFeature,
                    "Feature is not indexed on Bioseq " + label, *feat_it);
            } else {
                // check feature packaging
                // a feature packaged on a bioseq should have at least one location on the bioseq
                bool found = false;
                for (CSeq_loc_CI loc_it(feat_it->GetLocation()); loc_it; ++loc_it) {
                    const CSeq_id& id = loc_it.GetSeq_id();
                    if (seq.IsSetId()) {
                        for (const auto& id_it : seq.GetId()) {
                            if (id.Compare(*id_it) == CSeq_id::e_YES) {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (! found && seq.GetInst().GetRepr() == CSeq_inst::eRepr_seg) {
                        const CBioseq_Handle& part =
                            GetCache().GetBioseqHandleFromLocation(
                                m_Scope,
                                loc_it.GetEmbeddingSeq_loc(),
                                m_Imp.GetTSE_Handle());
                        if (part) {
                            CSeq_entry_Handle parent = part.GetParentEntry();
                            if (parent && parent.IsSeq()) {
                                parent = parent.GetParentEntry();
                                if (parent && parent.IsSet() && parent.GetSet().GetClass() == CBioseq_set::eClass_parts) {
                                    parent = parent.GetParentEntry();
                                    if (parent && parent.IsSet() && parent.GetSet().GetClass() == CBioseq_set::eClass_segset) {
                                        CBioseq_CI bi(parent);
                                        if (bi && bi->GetCompleteBioseq()->Equals(seq)) {
                                            found = true;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (! found && seq.GetInst().GetRepr() == CSeq_inst::eRepr_raw) {
                        CBioseq_Handle part = m_Scope->GetBioseqHandle(seq);
                        if (part) {
                            CSeq_entry_Handle parent = part.GetParentEntry();
                            if (parent && parent.IsSeq()) {
                                parent = parent.GetParentEntry();
                                if (parent && parent.IsSet() && parent.GetSet().GetClass() == CBioseq_set::eClass_parts) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (! found) {
                        if (m_Imp.IsSmallGenomeSet()) {
                            m_Imp.IncrementSmallGenomeSetMisplacedCount();
                            break;
                        }
                    }
                }
                if (! found) {
                    m_Imp.IncrementMisplacedFeatureCount();
                    break;
                }
            }
        }
    }
}

static bool x_IsEmblOrDdbjOnSet(const CBioseq_set_Handle& set)
{
    bool answer = false;
    for (CBioseq_CI b_ci(set); b_ci && ! answer; ++b_ci) {
        // actually looks only at the first seq-id
        const CBioseq* bioseq = b_ci->GetCompleteBioseq();
        if (bioseq->IsSetId()) {
            for (const auto& id_it : bioseq->GetId()) {
                switch (id_it->Which()) {
                case CSeq_id::e_Embl:
                case CSeq_id::e_Ddbj:
                case CSeq_id::e_Tpe:
                case CSeq_id::e_Tpd:
                    answer = true;
                    break;
                default:
                    break;
                }
            }
        }
    }

    return answer;
}


bool s_IsBioseqInSet(CBioseq_Handle bsh, const CBioseq_set& set)
{
    CBioseq_set_Handle parent = bsh.GetParentBioseq_set();
    while (parent) {
        if (parent.GetCompleteBioseq_set().GetPointer() == &set) {
            return true;
        }
        parent = parent.GetParentBioseq_set();
    }
    return false;
}


bool s_HasOneIntervalInSet(const CSeq_loc& loc, const CBioseq_set& set, CScope& scope, const CSeq_entry& tse)
{
    for (CSeq_loc_CI loc_it(loc); loc_it; ++loc_it) {
        const CSeq_id& id        = loc_it.GetSeq_id();
        CBioseq_Handle in_record = scope.GetBioseqHandleFromTSE(id, tse);
        if (! in_record)
            continue;
        if (s_IsBioseqInSet(in_record, set)) {
            return true;
        }
    }
    return false;
}


void CValidError_annot::ValidateSeqAnnotContext(const CSeq_annot& annot, const CBioseq_set& set)
{
    if (annot.IsGraph()) {
        for (const auto& graph : annot.GetData().GetGraph()) {
            m_GraphValidator.ValidateSeqGraphContext(*graph, set);
        }
    } else if (annot.IsFtable()) {
        // if a feature is packaged on a set, the bioseqs in the locations should be in the set
        CBioseq_set_Handle bssh     = m_Scope->GetBioseq_setHandle(set);
        bool is_embl_or_ddbj_on_set = x_IsEmblOrDdbjOnSet(bssh);

        for (const auto& feat_it : annot.GetData().GetFtable()) {
            ReportLocationGI0(*feat_it, "?");
            if (! feat_it->IsSetLocation() || IsLocationUnindexed(feat_it->GetLocation())) {
                m_Imp.PostErr(eDiag_Error, eErr_SEQ_FEAT_UnindexedFeature,
                    "Feature is not indexed on Bioseq ?", *feat_it);
            } else if (is_embl_or_ddbj_on_set) {
                // don't check packaging
            } else if (! set.IsSetClass() ||
                       (set.GetClass() != CBioseq_set::eClass_nuc_prot && set.GetClass() != CBioseq_set::eClass_gen_prod_set)) {
                m_Imp.IncrementMisplacedFeatureCount();
            } else if (feat_it->IsSetLocation() &&
                       ! s_HasOneIntervalInSet(feat_it->GetLocation(), set, *m_Scope, m_Imp.GetTSE())) {
                if (m_Imp.IsSmallGenomeSet()) {
                    m_Imp.IncrementSmallGenomeSetMisplacedCount();
                } else {
                    m_Imp.IncrementMisplacedFeatureCount();
                }
            }
        }
    }
}


// feature must have location on at least one sequence in this record
// feature location must not extend past end of sequence
bool CValidError_annot::IsLocationUnindexed(const CSeq_loc& loc)
{
    bool found_one = false;
    for (CSeq_loc_CI loc_it(loc); loc_it; ++loc_it) {
        const CSeq_id& id = loc_it.GetSeq_id();
#if 0
        CBioseq_Handle in_record = m_Scope->GetBioseqHandleFromTSE(id, m_Imp.GetTSE());
        if (in_record) {
            found_one = true;
            if (! loc_it.IsWhole() && loc_it.GetRange().GetFrom() > in_record.GetBioseqLength() - 1) {
                return true;
            }
        }
#else
        auto seq_len = m_Scope->GetSequenceLength(id, CScope::fDoNotRecalculate);
        if (seq_len != kInvalidSeqPos) {
            found_one = true;
            if (! loc_it.IsWhole() && loc_it.GetRange().GetFrom() > seq_len - 1) {
                return true;
            }
        }
#endif
    }

    return ! found_one;
}


void CValidError_annot::ReportLocationGI0(const CSeq_feat& f, const string& label)
{
    if (! f.IsSetLocation()) {
        return;
    }

    unsigned int zero_gi = 0;

    for (CSeq_loc_CI lit(f.GetLocation()); lit; ++lit) {
        if (lit.GetSeq_id().IsGi() && lit.GetSeq_id().GetGi() == ZERO_GI) {
            zero_gi++;
        }
    }

    if (zero_gi > 0) {
        PostErr(eDiag_Critical, eErr_SEQ_FEAT_FeatureLocationIsGi0,
            "Feature has " + NStr::UIntToString(zero_gi) + " gi|0 location" + (zero_gi > 1 ? "s" : "") + " on Bioseq " + label,
            f);
    }
}


END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE
