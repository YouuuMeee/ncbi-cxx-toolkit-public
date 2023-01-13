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
* Authors:  Sergiy Gotvyanskyy
*
* File Description:
*
*
*/

#include <ncbi_pch.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seq_inst.hpp>

#include <serial/objistr.hpp>
#include <corelib/ncbifile.hpp>

#include <objtools/edit/huge_asn_reader.hpp>
#include <objtools/readers/objhook_lambdas.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)


CHugeAsnReader::~CHugeAsnReader()
{
}

CHugeAsnReader::CHugeAsnReader()
{
}

CHugeAsnReader::CHugeAsnReader(CHugeFile* file, ILineErrorListener * pMessageListener)
{
    Open(file, pMessageListener);
}

void CHugeAsnReader::x_ResetIndex()
{
    m_max_local_id = 0;
    m_submit_block.Reset();
    m_bioseq_list.clear();
    m_bioseq_index.clear();
    m_bioseq_set_list.clear();
}

void CHugeAsnReader::Open(CHugeFile* file, ILineErrorListener * pMessageListener)
{
    x_ResetIndex();

    m_file = file;
    mp_MessageListener = pMessageListener;
}

bool CHugeAsnReader::IsMultiSequence() const
{
    return m_FlattenedSets.size()>1;
    //return m_bioseq_index.size()>1;
}

const CHugeAsnReader::TBioseqSetInfo* CHugeAsnReader::FindTopObject(CConstRef<CSeq_id> seqid) const
{
    auto it = m_FlattenedIndex.lower_bound(seqid);
    if (it == m_FlattenedIndex.end())
        return nullptr;
    if (it->first->CompareOrdered(*seqid) == 0)
        return &*it->second;
    if (it->first->Compare(*seqid) != CSeq_id::E_SIC::e_YES)
        return nullptr;

    return &*it->second;
}

const CHugeAsnReader::TBioseqInfo* CHugeAsnReader::FindBioseq(CConstRef<CSeq_id> seqid) const
{
    auto it = m_bioseq_index.lower_bound(seqid);
    if (it == m_bioseq_index.end())
        return nullptr;
    if (it->first->CompareOrdered(*seqid) == 0)
        return &*it->second;
    if (it->first->Compare(*seqid) != CSeq_id::E_SIC::e_YES)
        return nullptr;

    return &*it->second;
}



static CConstRef<CSeqdesc> s_GetDescriptor(const CSeq_descr& descr, CSeqdesc::E_Choice choice)
{
    if (descr.IsSet()) {
        for (auto pDesc : descr.Get()) {
            if (pDesc && (pDesc->Which() == choice)) {
                return pDesc;
            }
        }
    }

    return {};
}


CConstRef<CSeqdesc> CHugeAsnReader::GetClosestDescriptor(const TBioseqInfo& info, CSeqdesc::E_Choice choice) const
{
    CConstRef<CSeqdesc> result;

    if (info.m_descr) {
        result = s_GetDescriptor(*info.m_descr, choice);
        if (result) {
            return result;
        }
    }

    auto parentSet = info.m_parent_set;
    while (parentSet != end(m_bioseq_set_list)) {
        if (parentSet->m_descr) {
            result = s_GetDescriptor(*parentSet->m_descr, choice);
            if (result) {
                return result;
            }
        }
        parentSet = parentSet->m_parent_set;
    }

    return result;
}


CConstRef<CSeqdesc> CHugeAsnReader::GetClosestDescriptor(const CSeq_id& id, CSeqdesc::E_Choice choice) const
{
    CConstRef<CSeq_id> pId(&id);
    const auto* pInfo = FindBioseq(pId);
    if (!pInfo) {
        return {};
    }
    return GetClosestDescriptor(*pInfo, choice);
}


CRef<CSeq_entry> CHugeAsnReader::LoadSeqEntry(CConstRef<CSeq_id> seqid) const
{
    auto info = FindTopObject(seqid);
    if (info)
        return LoadSeqEntry(*info);
    else
        return {};
}

CRef<CSeq_entry> CHugeAsnReader::LoadSeqEntry(const TBioseqSetInfo& info, eAddTopEntry add_top_entry) const
{
    auto entry = Ref(new CSeq_entry);
    auto obj_stream = MakeObjStream(info.m_pos);
    if (info.m_class == CBioseq_set::eClass_not_set)
    {
        obj_stream->Read(&entry->SetSeq(), CBioseq::GetTypeInfo(), CObjectIStream::eNoFileHeader);
    } else {
        obj_stream->Read(&entry->SetSet(), CBioseq_set::GetTypeInfo(), CObjectIStream::eNoFileHeader);
    }

    if (add_top_entry == eAddTopEntry::yes && m_top_entry) {
        auto pNewEntry = Ref(new CSeq_entry());
        pNewEntry->Assign(*m_top_entry);
        pNewEntry->SetSet().SetSeq_set().push_back(entry);
        return pNewEntry;
    }

    return entry;
}

CRef<CBioseq> CHugeAsnReader::LoadBioseq(CConstRef<CSeq_id> seqid) const
{
    auto it = m_bioseq_index.lower_bound(seqid);
    if (it == m_bioseq_index.end())
        return {};
    if (it->first->Compare(*seqid) != CSeq_id::E_SIC::e_YES)
        return {};

    auto obj_stream = MakeObjStream(it->second->m_pos);
    auto bioseq = Ref(new CBioseq);
    obj_stream->Read(bioseq, CBioseq::GetTypeInfo(), CObjectIStream::eNoFileHeader);
    return bioseq;
}

unique_ptr<CObjectIStream> CHugeAsnReader::MakeObjStream(TFileSize pos) const
{
    unique_ptr<CObjectIStream> str;

    if (m_file->m_memory)
        str.reset(CObjectIStream::CreateFromBuffer(
        m_file->m_serial_format, m_file->m_memory+pos, m_file->m_filesize-pos));
    else {
        std::unique_ptr<std::ifstream> stream{new std::ifstream(m_file->m_filename, ios::binary)};
        stream->seekg(pos);
        str.reset(CObjectIStream::Open(m_file->m_serial_format, *stream.release(), eTakeOwnership));
    }

    str->UseMemoryPool();

    return str;
}

bool CHugeAsnReader::GetNextBlob()
{
    if (m_streampos >= m_file->m_filesize)
        return false;

    x_IndexNextAsn1();
    return true;
}

void CHugeAsnReader::x_SetHooks(CObjectIStream& objStream, CHugeAsnReader::TContext& context)
{
    CObjectTypeInfo bioseq_info = CType<CBioseq>();
    CObjectTypeInfo bioseq_set_info = CType<CBioseq_set>();
    CObjectTypeInfo seqinst_info = CType<CSeq_inst>();

    auto bioseq_id_mi = bioseq_info.FindMember("id");
    auto bioseqset_class_mi = bioseq_set_info.FindMember("class");
    auto bioseqset_descr_mi = bioseq_set_info.FindMember("descr");
    auto seqinst_len_mi = seqinst_info.FindMember("length");
    auto seqinst_mol_mi = seqinst_info.FindMember("mol");
    auto seqinst_repr_mi = seqinst_info.FindMember("repr");
    auto bioseq_descr_mi = bioseq_info.FindMember("descr");


    SetLocalSkipHook(bioseq_id_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        in.ReadObject(&context.bioseq_stack.back().m_ids, (*member).GetTypeInfo());
    });

    SetLocalSkipHook(bioseqset_class_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        CBioseq_set::TClass _class;
        in.ReadObject(&_class, (*member).GetTypeInfo());
        context.bioseq_set_stack.back()->m_class = _class;
    });

    SetLocalSkipHook(bioseqset_descr_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        auto descr = Ref(new CSeq_descr);
        in.ReadObject(&descr, (*member).GetTypeInfo());
        context.bioseq_set_stack.back()->m_descr = descr;
    });

    SetLocalSkipHook(bioseq_descr_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        auto descr = Ref(new CSeq_descr);
        in.ReadObject(&descr, (*member).GetTypeInfo());
        context.bioseq_stack.back().m_descr = descr;
    });

    SetLocalSkipHook(seqinst_len_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        in.ReadObject(&context.bioseq_stack.back().m_length, (*member).GetTypeInfo());
    });

    SetLocalSkipHook(seqinst_mol_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        in.ReadObject(&context.bioseq_stack.back().m_mol, (*member).GetTypeInfo());
    });


    SetLocalSkipHook(seqinst_repr_mi, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfoMI& member)
    {
        in.ReadObject(&context.bioseq_stack.back().m_repr, (*member).GetTypeInfo());
    });


    SetLocalSkipHook(CType<CFeat_id>(), objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfo& type)
    {
        auto id = Ref(new CFeat_id);
        type.GetTypeInfo()->DefaultReadData(in, id);
        if (id->IsLocal() && id->GetLocal().IsId())
        {
            m_max_local_id = std::max(m_max_local_id, id->GetLocal().GetId());
        }
    });


    SetLocalSkipHook(bioseq_info, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfo& type)
    {
        auto pos = in.GetStreamPos() + m_streampos;

        context.bioseq_stack.push_back({});
        auto parent = context.bioseq_set_stack.back();

        type.GetTypeInfo()->DefaultSkipData(in);

        auto& bioseqinfo = context.bioseq_stack.back();
        m_bioseq_list.push_back({pos, parent, bioseqinfo.m_length, bioseqinfo.m_descr, bioseqinfo.m_ids, bioseqinfo.m_mol, bioseqinfo.m_repr});
        context.bioseq_stack.pop_back();
    });

    SetLocalSkipHook(bioseq_set_info, objStream,
        [this, &context](CObjectIStream& in, const CObjectTypeInfo& type)
    {
        auto pos = in.GetStreamPos() + m_streampos;

        auto parent = context.bioseq_set_stack.back();
        m_bioseq_set_list.push_back({pos, parent});

        auto last = --(m_bioseq_set_list.end());

        context.bioseq_set_stack.push_back(last);
        type.GetTypeInfo()->DefaultSkipData(in);
        context.bioseq_set_stack.pop_back();
    });

    SetLocalSkipHook(CType<CSubmit_block>(), objStream,
        [this](CObjectIStream& in, const CObjectTypeInfo& type)
    {
        auto submit_block = Ref(new CSubmit_block);
        in.Read(submit_block, CSubmit_block::GetTypeInfo(), CObjectIStream::eNoFileHeader);
        m_submit_block = submit_block;
    });

}

void CHugeAsnReader::x_IndexNextAsn1()
{
    x_ResetIndex();
    auto object_type = m_file->RecognizeContent(m_streampos);

    auto obj_stream = MakeObjStream(m_streampos);

    TContext context;
    x_SetHooks(*obj_stream, context);


    // Ensure there is at least on bioseq_set_info object exists
    obj_stream->SkipFileHeader(object_type);
    m_bioseq_set_list.push_back({ 0, m_bioseq_set_list.end() });
    context.bioseq_set_stack.push_back(m_bioseq_set_list.begin());
    obj_stream->Skip(object_type, CObjectIStream::eNoFileHeader);
    obj_stream->EndOfData(); // force to SkipWhiteSpace
    m_streampos += obj_stream->GetStreamPos();
}

CRef<CObject> CHugeAsnReader::ReadAny()
{
    if (m_streampos >= m_file->m_filesize)
        return {};

    x_ResetIndex();
    auto object_type = m_file->RecognizeContent(m_streampos);
    if (object_type == nullptr || !object_type->IsCObject())
        return {};

    auto obj_stream = MakeObjStream(m_streampos);

    auto obj_info = obj_stream->Read(object_type);
    CRef<CObject> serial(static_cast<CObject*>(obj_info.GetObjectPtr()));
    obj_stream->EndOfData(); // force to SkipWhiteSpace
    m_streampos += obj_stream->GetStreamPos();

    return serial;
}

static bool s_ShouldSplitSet(CBioseq_set::EClass setClass) {
    return setClass == CBioseq_set::eClass_not_set ||
           setClass == CBioseq_set::eClass_genbank;
}

void CHugeAsnReader::x_ThrowDuplicateId(
    const TBioseqSetInfo& existingInfo,
    const TBioseqSetInfo& newInfo,
    const CSeq_id& duplicateId)
{
    auto filename = m_file->m_filename;
    auto existingPos = existingInfo.m_pos;
    auto newPos = newInfo.m_pos;

    auto existingFilePos = NStr::UInt8ToString(existingPos);
    auto newFilePos = NStr::UInt8ToString(newPos);
    if (!filename.empty()) {
        existingFilePos = filename + ":" + existingFilePos;
        newFilePos = filename + ":" + newFilePos;
    }
    string msg = "duplicate Bioseq id " + objects::GetLabel(duplicateId) +
        " present in the set starting at " + existingFilePos;
    if (newPos != existingPos) {
        msg += " and the set starting at " + newFilePos;
    }
    NCBI_THROW(CHugeFileException, eDuplicateSeqIds, msg);
}

void CHugeAsnReader::FlattenGenbankSet()
{
    m_FlattenedSets.clear();
    m_top_ids.clear();
    m_FlattenedIndex.clear();

    for (auto it = m_bioseq_list.begin(); it!= m_bioseq_list.end(); ++it)
    {
        auto rec = *it;
        auto parent = rec.m_parent_set;

        if (auto _class = parent->m_class; s_ShouldSplitSet(_class))
        { // create fake bioseq_set
            m_FlattenedSets.push_back({rec.m_pos, m_FlattenedSets.cend(), objects::CBioseq_set::eClass_not_set});
            m_top_ids.push_back(rec.m_ids.front());
        } else {

            auto grandParent = parent->m_parent_set;
            while (!s_ShouldSplitSet(grandParent->m_class)) {
                parent = grandParent;
                grandParent = grandParent->m_parent_set;
            }
            if (m_FlattenedSets.empty() || (m_FlattenedSets.back().m_pos != parent->m_pos)) {
                m_FlattenedSets.push_back(*parent);
                m_top_ids.push_back(rec.m_ids.front());
            }
        }
        auto last = --m_FlattenedSets.end();
        for (auto id: rec.m_ids) {
            auto existingIndex = m_FlattenedIndex.find(id);
            if (existingIndex != m_FlattenedIndex.end()) {
                x_ThrowDuplicateId(*(existingIndex->second), *last, *id);
            }
            m_FlattenedIndex[id] = last;
            m_bioseq_index[id] = it;
        }
    }

    if (GetBiosets().size()>1)
    {
        auto top = next(GetBiosets().begin());
        if (m_FlattenedSets.size() == 1) {
            // exposing the whole top entry
            if (GetSubmitBlock().NotEmpty()
                || (top->m_class != CBioseq_set::eClass_genbank)
                || top->m_descr.NotEmpty())
            {
                m_FlattenedSets.clear();
                m_FlattenedSets.push_back(*top);
            }
        }
        else { // m_FlattenedSets.size() > 1)
            if (top->m_descr ||
                    (top->m_class != CBioseq_set::eClass_genbank &&
                     top->m_class != CBioseq_set::eClass_not_set)) {
                auto top_entry = Ref(new CSeq_entry());
                top_entry->SetSet().SetClass() = top->m_class;
                if (top->m_descr)
                    top_entry->SetSet().SetDescr().Assign(*top->m_descr);
                m_top_entry = top_entry;
            }
        }
    }

    m_Current = m_FlattenedSets.begin();
}

CConstRef<CSubmit_block> CHugeAsnReader::GetSubmitBlock() const
{
    return m_submit_block;
};

CRef<CSeq_entry> CHugeAsnReader::GetNextSeqEntry()
{
    if (m_Current == end(m_FlattenedSets)) {
        m_FlattenedSets.clear();
        m_Current = m_FlattenedSets.end();
        return {};
    }

    return LoadSeqEntry(*m_Current++, eAddTopEntry::no);
}

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE
