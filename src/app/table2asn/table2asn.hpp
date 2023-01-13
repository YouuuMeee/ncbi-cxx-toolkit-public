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
* Authors:  Jonathan Kans, Clifford Clausen,
*           Aaron Ucko, Sergiy Gotvyanskyy
*
* File Description:
*   Converter of various files into ASN.1 format, main application function
*
*/

#include <objtools/readers/message_listener.hpp>
#include <misc/data_loaders_util/data_loaders_util.hpp>
#include <util/format_guess.hpp>

#include "table2asn_context.hpp"

BEGIN_NCBI_SCOPE
namespace objects {
    struct TAsyncToken;
};

using namespace objects;

class CSerialObject;

class CTable2AsnLogger;
class CMultiReader;
class CTable2AsnValidator;
class CMask;
class CTable2AsnStructuredCommentsReader;
//class CHugeAsnReader;
class CFeatureTableReader;

/*
struct TAsyncToken
{
    CRef<CSeq_entry>  entry;
    CRef<CSeq_submit> submit;
    CRef<CSeq_entry>  top_entry;
    CRef<CScope>      scope;
    CSeq_entry_Handle seh;
};
*/

class CTbl2AsnApp : public CNcbiApplication
{
public:
    CTbl2AsnApp();

    void Init() override;
    int Run() override;
    int DryRun() override
    {
        return Run();
    }

private:

    static const CDataLoadersUtil::TLoaders default_loaders = CDataLoadersUtil::fGenbank | CDataLoadersUtil::fVDB | CDataLoadersUtil::fGenbankOffByDefault | CDataLoadersUtil::fSRA;
    void Setup(const CArgs& args);

    void ProcessOneFile(bool isAlignment);
    void ProcessOneFile(CNcbiOstream* output);
    void ProcessHugeFile(CNcbiOstream* output);
    void ProcessOneEntry(CFormatGuess::EFormat inputFormat, CRef<CSerialObject> obj, CRef<CSerialObject>& result);
    void ProcessSingleEntry(CFormatGuess::EFormat inputFormat, TAsyncToken& token);
    void MakeFlatFile(CSeq_entry_Handle seh, CRef<CSeq_submit> submit, std::ostream& ostream);
    void ProcessTopEntry(CFormatGuess::EFormat inputFormat, bool need_update_date, CRef<CSeq_submit>& submit, CRef<CSeq_entry>& entry);
    bool ProcessOneDirectory(const CDir& directory, const CMask& mask, bool recurse);
    void ProcessAlignmentFile(CNcbiOstream* output);
    void ReportUnusedSourceQuals();
    void ProcessSecretFiles1Phase(bool readModsFromTitle, CSeq_entry& result);
    void ProcessSecretFiles2Phase(CSeq_entry& result) const;
    void ProcessCMTFiles(CSeq_entry& result) const;
    void LoadPEPFile(const string& pathname);
    void LoadRNAFile(const string& pathname);
    void LoadPRTFile(const string& pathname);
    void LoadDSCFile(const string& pathname);
    void LoadAdditionalFiles();
    void LoadCMTFile(const string& pathname, unique_ptr<CTable2AsnStructuredCommentsReader>& comments);
    void LoadAnnots(const string& pathname, list<CRef<CSeq_annot>>& annots);
    void AddAnnots(CScope& scope);

    void xProcessHugeEntries();

    void x_SetAlnArgs(CArgDescriptions& arg_desc);

    struct TAdditionalFiles
    {
        unique_ptr<CTable2AsnStructuredCommentsReader> m_struct_comments;
        list<CRef<CSeq_annot>> m_Annots;
        CRef<CSeq_entry> m_replacement_proteins;
        CRef<CSeq_entry> m_possible_proteins;
        CRef<CSeq_descr> m_descriptors;
        unique_ptr<CMemorySrcFileMap> mp_src_qual_map;
        unique_ptr<CFeatureTableReader> m_feature_table_reader;
    };

    TAdditionalFiles m_global_files;
    unique_ptr<TAdditionalFiles> m_secret_files;

    unique_ptr<CMultiReader> m_reader;
    CRef<CTable2AsnValidator> m_validator;
    CRef<CTable2AsnLogger> m_logger;
    CTable2AsnContext    m_context;

    static const Int8 TBL2ASN_MAX_ALLOWED_FASTA_SIZE = INT8_C(0x7FFFFFFF);
};

END_NCBI_SCOPE
