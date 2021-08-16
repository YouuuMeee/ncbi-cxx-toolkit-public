#ifndef AUTOGENERATEDCLEANUP__HPP
#define AUTOGENERATEDCLEANUP__HPP

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
 */
/// This file was generated by application DATATOOL
///
/// ATTENTION:
///   Don't edit or commit this file into SVN as this file will
///   be overridden (by DATATOOL) without warning!

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_bond.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/PDB_seq_id.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <objects/seqloc/Seq_loc_equiv.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_loc_mix.hpp>
#include <objects/seqloc/Packed_seqint.hpp>
#include <objects/seqloc/Packed_seqpnt.hpp>
#include <objects/seqalign/Dense_diag.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Packed_seg.hpp>
#include <objects/seqalign/Sparse_seg.hpp>
#include <objects/seqalign/Sparse_align.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/seqalign/Spliced_exon.hpp>
#include <objects/seqalign/Std_seg.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/biblio/Cit_art.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Cit_book.hpp>
#include <objects/biblio/Imprint.hpp>
#include <objects/biblio/PubStatusDateSet.hpp>
#include <objects/biblio/PubStatusDate.hpp>
#include <objects/biblio/Cit_jour.hpp>
#include <objects/biblio/Cit_proc.hpp>
#include <objects/biblio/Meeting.hpp>
#include <objects/biblio/Title.hpp>
#include <objects/medline/Medline_entry.hpp>
#include <objects/biblio/Cit_pat.hpp>
#include <objects/biblio/Patent_priority.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/biblio/Cit_let.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/MultiOrgName.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Clone_seq_set.hpp>
#include <objects/seqfeat/Clone_seq.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seq/Numbering.hpp>
#include <objects/seq/Num_ref.hpp>
#include <objects/seqfeat/Prot_ref.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/RNA_qual_set.hpp>
#include <objects/seqfeat/RNA_qual.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Txinit.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Variation_inst.hpp>
#include <objects/seqfeat/Delta_item.hpp>
#include <objects/seqfeat/SeqFeatSupport.hpp>
#include <objects/seqfeat/InferenceSupport.hpp>
#include <objects/seqfeat/EvidenceBasis.hpp>
#include <objects/seqfeat/ModelEvidenceSupport.hpp>
#include <objects/seqfeat/ModelEvidenceItem.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqres/Seq_graph.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/Scaled_int_multi_data.hpp>
#include <objects/seqtable/Scaled_real_multi_data.hpp>
#include <objects/seqtable/SeqTable_single_data.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Align_def.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqblock/EMBL_block.hpp>
#include <objects/seqblock/GB_block.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqblock/PDB_block.hpp>
#include <objects/seqblock/PDB_replace.hpp>
#include <objects/seqblock/PIR_block.hpp>
#include <objects/seqblock/SP_block.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Map_ext.hpp>
#include <objects/seq/Ref_ext.hpp>
#include <objects/seq/Seg_ext.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seq/Seq_hist_rec.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>

#include "newcleanupp.hpp"


BEGIN_SCOPE(ncbi)
BEGIN_SCOPE(objects)

class CAutogeneratedCleanup {
public:
  CAutogeneratedCleanup(
    CScope & scope,
    CNewCleanup_imp & newCleanup ) :
    m_Scope(scope),
    m_NewCleanup(newCleanup),
    m_LastArg_BasicCleanupBioseq(NULL),
    m_pCurrentSeqFeat(NULL),
    m_pCurrentBioSource(nullptr),
    m_LastArg_x_BasicCleanupBioseq_inst_inst(NULL),
    m_Dummy(0)
  {}

  void BasicCleanupSeqEntry( CSeq_entry & arg0 );
  void BasicCleanupSeqSubmit( CSeq_submit & arg0 );
  void BasicCleanupSeqAnnot( CSeq_annot & arg0 );
  void BasicCleanupBioseq( CBioseq & arg0 );
  void BasicCleanupBioseqSet( CBioseq_set & arg0 );
  void BasicCleanupSeqFeat( CSeq_feat & arg0_raw );
  void BasicCleanupSeqdesc(CSeqdesc & arg0) { x_BasicCleanupBioseq_descr_descr_E(arg0); }

private:
  void x_BasicCleanupDate(CDate& date);
  void x_BasicCleanupSeqId(CSeq_id &id);
  void x_BasicCleanupSeqPoint(CSeq_point & seq_point);
  void x_BasicCleanupSeqBond(CSeq_bond & seq_bond);
  void x_BasicCleanupSeqLocEquiv( CSeq_loc_equiv & arg0 );
  void x_BasicCleanupSeqInt(CSeq_interval& seq_int);
  void x_BasicCleanupSeqLocMix( CSeq_loc_mix & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_interval_ >
  void x_BasicCleanupSeqInts( Tcontainer_ncbi_cref_cseq_interval_ & arg0 );
  void x_BasicCleanupPackedSeqInt(CPacked_seqint & arg0);
  void x_BasicCleanupPackedPoint(CPacked_seqpnt & arg0);
  void x_BasicCleanupSeqLoc(CSeq_loc& seq_loc);
  template<typename TSeqLocContainer>
  void x_BasicCleanupSeqLocs(TSeqLocContainer& locs);
  template<typename TSeqIdContainer>
  void x_BasicCleanupSeqIds(TSeqIdContainer& ids);
  void x_BasicCleanupDenseDiag( CDense_diag & arg0 );
  template<typename TDenseDiagContainer>
  void x_BasicCleanupDenseDiags(TDenseDiagContainer& dense_diags);
  void x_BasicCleanupDenseg(CDense_seg & denseg);
  void x_BasicCleanupSeqAlignSet( CSeq_align_set & arg0 );
  void x_BasicCleanupPackedSeg( CPacked_seg & arg0 );
  void x_BasicCleanupSparseAlign( CSparse_align & arg0 );
  void x_BasicCleaupSparseSeg( CSparse_seg & arg0 );
  void x_BasicCleanupSplicedExon( CSpliced_exon & arg0 );
  void x_BasicCleanupSplicedSeg( CSpliced_seg & arg0 );
  void x_BasicCleanupStdSeg( CStd_seg & arg0 );
  void x_BasicCleanupAlignSegs( CSeq_align::C_Segs & arg0 );
  void x_BasicCleanupSeqAlign( CSeq_align & arg0 );
  template<typename TSeqAlignContainer>
  void x_BasicCleanupSeqAligns(TSeqAlignContainer& aligns);
  void x_BasicCleanupAffilStd( CAffil::C_Std & arg0 );
  void x_BasicCleanupAffil(CAffil & arg0);
  void x_BasicCleanupAuthor(CAuthor & arg0);
  void x_BasicCleanupAuthListNames( CAuth_list::C_Names & arg0 );
  void x_BasicCleanupAuthList( CAuth_list & arg0, bool fix_initials=false);
  void x_BasicCleanupPubStatusDate(CPubStatusDate & arg0);
  void x_BasicCleanupPubStatusDateSet(CPubStatusDateSet & arg0);
  void x_BasicCleanupImprint(CImprint & arg0);
  void x_BasicCleanupCitBook(CCit_book & arg0);
  void x_BasicCleanupCitJournal( CCit_jour & arg0 );
  void x_BasicCleanupMeeting( CMeeting & arg0 );
  void x_BasicCleanupCitProc( CCit_proc & arg0 );
  void x_BasicCleanupCitArtFrom( CCit_art::C_From & arg0 );
  void x_BasicCleanupTitle( CTitle & arg0 );
  void x_BasicCleanupCitArt( CCit_art & arg0 );
  void x_BasicCleanupMedlineEntry(CMedline_entry & arg0);
  void x_BasicCleanupPatentPriority(CPatent_priority & arg0);
  void x_BasicCleanupCitPat( CCit_pat & arg0 );
  void x_BasicCleanupCitGen( CCit_gen & arg0 );
  void x_BasicCleanupCitLet( CCit_let & arg0 );
  void x_BasicCleanupSeqFeat_cit_ETC( CPub_set & arg0 );
  void  x_BasicCleanupSeqFeat_comment( std::string & arg0 );
  template< typename Tcontainer_ncbi_cref_cdbtag_ >
void x_BasicCleanupSeqFeat_dbxref_ETC( Tcontainer_ncbi_cref_cdbtag_ & arg0 );
  template< typename Tcontainer_std_string_ >
void x_BasicCleanupOrgRefMod( Tcontainer_std_string_ & arg0 );
  template< typename Tcontainer_ncbi_cref_corgmod_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_biosrc_org_org_orgname_orgname_mod_ETC( Tcontainer_ncbi_cref_corgmod_ & arg0 );
  void x_BasicCleanupOrgNameName_hybrid_hybrid_E( COrgName & arg0 );
  template< typename Tcontainer_ncbi_cref_corgname_ >
void x_BasicCleanupOrgNameName_hybrid_hybrid( Tcontainer_ncbi_cref_corgname_ & arg0 );
  void x_BasicCleanupOrgNameName_hybrid( CMultiOrgName & arg0 );
  void x_BasicCleanupOrgNameName( COrgName::C_Name & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_biosrc_org_org_orgname_E1798_ETC( COrgName & arg0 );
  void x_BasicCleanupSeqFeatDataOrgName( COrgName & arg0, bool cleanup_parent_biosource);
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_biosrc_pcr_primers_ETC( CPCRReactionSet & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_somatic_origin_E_source_source_ETC( CSubSource & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_somatic_origin_E_source_ETC( CSubSource & arg0 );
  template< typename Tcontainer_ncbi_cref_csubsource_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_biosrc_subtype_ETC( Tcontainer_ncbi_cref_csubsource_ & arg0 );
  void x_BasicCleanupSeqFeat_data_data_biosrc_biosrc( CBioSource & arg0 );
  void x_BasicCleanupSeqFeat_data_data_biosrc( CBioSource & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_cdregion_cdregion_code_break_E_E_ETC( CCode_break & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_cdregion_cdregion_code_break_E_ETC( CCode_break & arg0 );
  template< typename Tcontainer_ncbi_cref_ccode_break_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_cdregion_cdregion_code_break_ETC( Tcontainer_ncbi_cref_ccode_break_ & arg0 );
  void x_BasicCleanupSeqFeat_data_data_cdregion_cdregion( CCdregion & arg0 );
  void x_BasicCleanupSeqFeat_data_data_cdregion( CCdregion & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_clone_clone_seq_clone_seq_E_E_ETC( CClone_seq & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_clone_clone_seq_clone_seq_E_ETC( CClone_seq & arg0 );
  template< typename Tcontainer_ncbi_cref_cclone_seq_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_clone_clone_seq_clone_seq_ETC( Tcontainer_ncbi_cref_cclone_seq_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_clone_clone_seq_ETC( CClone_seq_set & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_clone_ETC( CClone_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_clone_ETC( CClone_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_gene_E_E_desc_ETC( std::string & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_gene_E_E_locus_ETC( std::string & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_gene_E_E_syn_E_ETC( std::string & arg0 );
  template< typename Tcontainer_std_string_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_gene_E_E_syn_ETC( Tcontainer_std_string_ & arg0 );
  void x_BasicCleanupGeneRef( CGene_ref & arg0, bool cleanup_parent_feat);
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_imp_imp_ETC( CImp_feat & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_imp_ETC( CImp_feat & arg0 );
  void x_BasicCleanupNumRef( CNum_ref & arg0 );
  void x_BasicCleanupNumbering( CNumbering & arg0 );
  void x_BasicCleanupOrgRef( COrg_ref & arg0, bool inBioSource);
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_activity_E_ETC( std::string & arg0 );
  template< typename Tcontainer_std_string_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_activity_ETC( Tcontainer_std_string_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_desc_ETC( std::string & arg0 );
  template< typename Tcontainer_std_string_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_ec_ETC( Tcontainer_std_string_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_name_E_ETC( std::string & arg0 );
  template< typename Tcontainer_std_string_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_E_E_name_ETC( Tcontainer_std_string_ & arg0 );
  void x_BasicCleanupProtRef( CProt_ref & arg0, bool cleanup_parent_feat);
  void x_BasicCleanupPub( CPub & arg0 );
  void x_BasicCleanupPubEquiv( CPub_equiv & arg0 );
  template< typename Tcontainer_ncbi_cref_cpub_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_pub_pub_pub_pub1778_ETC( Tcontainer_ncbi_cref_cpub_ & arg0 );
  void x_BasicCleanupSeqFeat_data_data_pub_ETC( CPubdesc & arg0 );
  void x_BasicCleanupSeqFeat_data_data_region_ETC( std::string & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_product_ETC( std::string & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_quals_quals_E_E_ETC( CRNA_qual & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_quals_quals_E_ETC( CRNA_qual & arg0 );
  template< typename Tcontainer_ncbi_cref_crna_qual_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_quals_quals_ETC( Tcontainer_ncbi_cref_crna_qual_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_quals_ETC( CRNA_qual_set & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_gen_ETC( CRNA_gen & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_gen_ETC( CRNA_gen & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_name_ETC( std::string & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_tRNA_tRNA_ETC( CTrna_ext & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_tRNA_ETC( CTrna_ext & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_rna_ext_ETC( CRNA_ref::C_Ext & arg0 );
  void x_BasicCleanupSeqFeat_data_data_rna_rna( CRNA_ref & arg0 );
  void x_BasicCleanupSeqFeat_data_data_rna( CRNA_ref & arg0 );
  void x_BasicCleanupSeqFeat_data_data_site( CSeqFeatData::ESite & arg0 );
  template< typename Tcontainer_ncbi_cref_cprot_ref_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_protein_ETC( Tcontainer_ncbi_cref_cprot_ref_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_txorg_ETC( COrg_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_txinit_ETC( CTxinit & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_txinit_ETC( CTxinit & arg0 );
  void x_BasicCleanupSeqFeat_ext_ext1769_ETC( CUser_object & arg0 );
  void x_BasicCleanupSeqFeat_ext_ETC( CUser_object & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_consequence_E_variation( CVariation_ref & arg0 );
  template< typename Tcvariation_ref_container_ncbi_cref_c_e_consequence_c_e_consequence >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_consequence_E( Tcvariation_ref_container_ncbi_cref_c_e_consequence_c_e_consequence & arg0 );
  template< typename Tcontainer_ncbi_cref_c_e_consequence_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_consequence( Tcontainer_ncbi_cref_c_e_consequence_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_instance_delta_E_E_seq_ETC( CDelta_item::C_Seq & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_instance_delta_E_E_ETC( CDelta_item & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_instance_delta_E_ETC( CDelta_item & arg0 );
  template< typename Tcontainer_ncbi_cref_cdelta_item_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_instance_delta_ETC( Tcontainer_ncbi_cref_cdelta_item_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_instance_ETC( CVariation_inst & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_instance_ETC( CVariation_inst & arg0 );
  template< typename Tcvariation_ref_container_ncbi_cref_c_e_somatic_origin_c_e_somatic_origin >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_somatic_origin_E_ETC( Tcvariation_ref_container_ncbi_cref_c_e_somatic_origin_c_e_somatic_origin & arg0 );
  template< typename Tcontainer_ncbi_cref_c_e_somatic_origin_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_somatic_origin_ETC( Tcontainer_ncbi_cref_c_e_somatic_origin_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_E1804_ETC( CVariation_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_set_variations_E( CVariation_ref & arg0 );
  template< typename Tcontainer_ncbi_cref_cvariation_ref_ >
void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_set_variations( Tcontainer_ncbi_cref_cvariation_ref_ & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data_set( CVariation_ref::C_Data::C_Set & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_data( CVariation_ref::C_Data & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation1805_ETC( CVariation_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_ETC( CVariation_ref & arg0 );
  void x_BasicCleanupSeqFeat_data( CSeqFeatData & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_variation_variation_location_location_equiv_ETC( CSeq_loc_equiv & arg0 );
  void x_BasicCleanupSeqFeat_location_location1767_ETC( CSeq_loc & arg0 );
  void x_BasicCleanupSeqFeat_location_ETC( CSeq_loc & arg0 );
  template< typename Tcontainer_ncbi_cref_cgb_qual_ >
void x_BasicCleanupSeqFeat_qual_ETC( Tcontainer_ncbi_cref_cgb_qual_ & arg0 );
  void x_BasicCleanupSeqFeat_support_support_inference_E_E_basis_basis_ETC( CEvidenceBasis & arg0 );
  void x_BasicCleanupSeqFeat_support_support_inference_E_E_basis_ETC( CEvidenceBasis & arg0 );
  void x_BasicCleanupSeqFeat_support_support_inference_E_E_ETC( CInferenceSupport & arg0 );
  void x_BasicCleanupSeqFeat_support_support_inference_E_ETC( CInferenceSupport & arg0 );
  template< typename Tcontainer_ncbi_cref_cinferencesupport_ >
void x_BasicCleanupSeqFeat_support_support_inference_ETC( Tcontainer_ncbi_cref_cinferencesupport_ & arg0 );
  void x_BasicCleanupSeqFeat_support_support_model_evidence_E_E_protein_E_ETC( CModelEvidenceItem & arg0 );
  template< typename Tcontainer_ncbi_cref_cmodelevidenceitem_ >
void x_BasicCleanupSeqFeat_support_support_model_evidence_E_E_protein_ETC( Tcontainer_ncbi_cref_cmodelevidenceitem_ & arg0 );
  void _BasicCleanupSeqFeat_support_support_model_evidence_E_E_ETC( CModelEvidenceSupport & arg0 );
  void x_BasicCleanupModelEvidenceSupport( CModelEvidenceSupport & arg0 );
  template< typename Tcontainer_ncbi_cref_cmodelevidencesupport_ >
void x_BasicCleanupSeqFeat_support_support_model_evidence_ETC( Tcontainer_ncbi_cref_cmodelevidencesupport_ & arg0 );
  void x_BasicCleanupSeqFeatSupport( CSeqFeatSupport & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_biosrc_org( COrg_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_biosrc_ETC( CBioSource & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_cdregion_ETC( CCdregion & arg0 );
  void x_BasicCleanupSeqFeatXrefPub( CPubdesc & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_rna_ETC( CRNA_ref & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_data_ETC( CSeqFeatData & arg0 );
  void x_BasicCleanupSeqFeat_xref_E_E_data_ETC( CSeqFeatData & arg0 );
  void x_BasicCleanupSeqFeatXref( CSeqFeatXref & arg0 );
  template< typename Tcontainer_ncbi_cref_cseqfeatxref_ >
void x_BasicCleanupSeqFeat_xref_ETC( Tcontainer_ncbi_cref_cseqfeatxref_ & arg0 );
  template<typename TSeqFeatContainer>
  void x_BasicCleanupSeqFeats(TSeqFeatContainer& feats);
  void x_BasicCleanupSeqGraph( CSeq_graph & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_graph_ >
void x_BasicCleanupSeqAnnotGraph( Tcontainer_ncbi_cref_cseq_graph_ & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_int_scaled_int_scaled_data( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_int_scaled_int_scaled( CScaled_int_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_int_scaled( CScaled_int_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_real_scaled_real_scaled_data( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_real_scaled_real_scaled( CScaled_real_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_real_scaled( CScaled_real_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_int_delta1713_ETC( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data_int_delta( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_data1712_ETC( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_data_ETC( CSeqTable_multi_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_sparse_other_sparse_other_ETC( CSeqTable_single_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_E_sparse_other_ETC( CSeqTable_single_data & arg0 );
  void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_E_ETC( CSeqTable_column & arg0 );
  template< typename Tcontainer_ncbi_cref_cseqtable_column_ >
void x_BasicCleanupSeqAnnotData_seq_table_seq_table_columns_ETC( Tcontainer_ncbi_cref_cseqtable_column_ & arg0 );
  void x_BasicCleanupSeqTable( CSeq_table & arg0 );
  void x_BasicCleanupBioseqSet_annot_E_E_desc_desc_E_E_align_ETC( CAlign_def & arg0 );
  void x_BasicCleanupBioseqSet_annot_E_E_desc_desc_E_ETC( CAnnotdesc & arg0 );
  template< typename Tcontainer_ncbi_cref_cannotdesc_ >
void x_BasicCleanupBioseqSet_annot_E_E_desc_desc_ETC( Tcontainer_ncbi_cref_cannotdesc_ & arg0 );
  void x_BasicCleanupBioseqSet_annot_E_E_desc_ETC( CAnnot_descr & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_annot_ >
void x_BasicCleanupBioseq_annot( Tcontainer_ncbi_cref_cseq_annot_ & arg0 );
  void x_BasicCleanupDescComment( std::string & arg0 );
  void x_BasicCleanupEMBLBlock( CEMBL_block & arg0 );
  void x_BasicCleanupGBBlock( CGB_block & arg0 );
  void x_BasicCleanupMolInfo( CMolInfo & arg0 );
  void x_BasicCleanupPDBBlock( CPDB_block & arg0 );
  void x_BasicCleanupPIRBlock( CPIR_block & arg0 );
  void x_BasicCleanupDescRegion( std::string & arg0 );
  void x_BasicCleanupSPBlock( CSP_block & arg0 );
  void x_BasicCleanupDescTitle( std::string & arg0 );
  void x_BasicCleanupBioseq_descr_descr_E( CSeqdesc & arg0 );
  void x_BasicCleanupBioseq_descr( CSeq_descr & arg0 );
  void x_BasicCleanupDeltaExt( CDelta_ext & arg0 );
  void x_BasicCleanupBioseq_inst_inst_ext_ext_map( CMap_ext & arg0 );
  void x_BasicCleanupBioseqSet_seq_set_E_E_seq_seq_inst_inst_ext_ext_seg_ETC( CSeg_ext & arg0 );
  void x_BasicCleanupSeqExt( CSeq_ext & arg0 );
  void x_BasicCleanupSeqHistDeleted( CSeq_hist::C_Deleted & arg0 );
  void x_BasicCleanupSeqHistRec( CSeq_hist_rec & arg0 );
  void x_BasicCleanupSeqHist( CSeq_hist & arg0 );
  void x_BasicCleanupBioseq_inst( CSeq_inst & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_annot_ >
void x_BasicCleanupSeqAnnots( Tcontainer_ncbi_cref_cseq_annot_ & arg0 );
  void x_BasicCleanupBioseqSetDesc( CSeqdesc & arg0 );
  void x_BasicCleanupBioseqSet_descr_ETC( CSeq_descr & arg0 );
  void x_BasicCleanupSeqEntry_set( CBioseq_set & arg0 );
  void x_BasicCleanupSeqSubmit_data_annots_E( CSeq_annot & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_annot_ >
void x_BasicCleanupSeqSubmit_data_annots( Tcontainer_ncbi_cref_cseq_annot_ & arg0 );
  void x_BasicCleanupSeqSubmit_data( CSeq_submit::C_Data & arg0 );
  void x_BasicCleanupCitSub(CCit_sub & arg0, bool in_submit_block=false);
  void x_BasicCleanupContactInfo( CContact_info & arg0 );
  void x_BasicCleanupSeqSubmit_sub( CSubmit_block & arg0 );
  void x_BasicCleanupSeqAnnotData( CSeq_annot::C_Data & arg0 );
  void x_BasicCleanupBioseqSet_seq_set_E( CSeq_entry & arg0 );
  template< typename Tcontainer_ncbi_cref_cseq_entry_ >
void x_BasicCleanupBioseqSet_seq_set( Tcontainer_ncbi_cref_cseq_entry_ & arg0 );

  CScope & m_Scope;
  CNewCleanup_imp & m_NewCleanup;

  CBioseq* m_LastArg_BasicCleanupBioseq;
  CSeq_feat* m_pCurrentSeqFeat;
  CBioSource* m_pCurrentBioSource;
  CSeq_inst* m_LastArg_x_BasicCleanupBioseq_inst_inst;

  int m_Dummy;
}; // end of CAutogeneratedCleanup

END_SCOPE(objects)
END_SCOPE(ncbi)

#endif /* AUTOGENERATEDCLEANUP__HPP */
