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
* Author:  Sema Kachalo, NCBI
*
* File Description:
*   Sample unit tests file for the mainstream test developing.
*
* This file represents basic most common usage of Ncbi.Test framework (which
* is based on Boost.Test framework. For more advanced techniques look into
* another sample - unit_test_alt_sample.cpp.
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include "../discrepancy_core.hpp"
#include <corelib/test_boost.hpp>
#include <objtools/unit_test_util/unit_test_util.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(NDiscrepancy);
USING_SCOPE(unit_test_util);

// Needed under windows for some reason.
#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{
    void throw_exception( std::exception const & e ) {
        throw e;
    }
}
#endif


BOOST_AUTO_TEST_CASE(FLATFILE_FIND)
{
    UnitTest_FLATFILE_FIND();
}

BOOST_AUTO_TEST_CASE(Test_Autofix)
{
    auto tests = NDiscrepancy::GetDiscrepancyTests(NDiscrepancy::eAll);
    BOOST_CHECK(!tests.empty());
}

BOOST_AUTO_TEST_CASE(Test_Autofix1)
{
    auto tests = NDiscrepancy::GetDiscrepancyTests(NDiscrepancy::eAutofix);
    BOOST_CHECK_EQUAL(tests.size(), 39);
    BOOST_CHECK(!tests.empty());
}

BOOST_AUTO_TEST_CASE(Test_Autofix2)
{
    vector<string> tests = NDiscrepancy::GetDiscrepancyNames(NDiscrepancy::eAutofix);
    BOOST_CHECK_EQUAL(tests.size(), 39);
    BOOST_CHECK(!tests.empty());
    //for (auto name: tests) std::cerr << name << "\n";
}


BOOST_AUTO_TEST_CASE(Test_DiscrepancySet_Format)
{
    string inputString = "[n] [is] [(][does][)] [has] [* this gets deleted *]";
    string outputString = "2 are do have ";
    BOOST_CHECK_EQUAL(CDiscrepancySet::Format(inputString, 2), outputString);
}


BOOST_AUTO_TEST_CASE(Test_NamesAndDescriptions)
{
    BOOST_CHECK_EQUAL(GetDiscrepancyCaseName(eTestNames::ALL_SEQS_CIRCULAR), "ALL_SEQS_CIRCULAR");
    BOOST_CHECK_EQUAL(GetDiscrepancyCaseName("DISC_ALL_SEQS_CIRCULAR"), eTestNames::ALL_SEQS_CIRCULAR);
    BOOST_CHECK_EQUAL(GetDiscrepancyDescr(eTestNames::ALL_SEQS_CIRCULAR), "All sequences circular");
}


BOOST_AUTO_TEST_CASE(Test_CDiscrepancyCore) 
{
    class CDiscrepancyMock : public CDiscrepancyCore
    {
    public:
        using CDiscrepancyCore::CDiscrepancyCore;
        [[nodiscard]] 
        CRef<CAutofixReport> Autofix(CDiscrepancyObject*, CDiscrepancyContext&) const override
        {
            return CRef<CAutofixReport>();
        }
        void Visit(CDiscrepancyContext& context) override {
           NCBI_THROW(CException, eUnknown, m_ExceptionMsg);
        }

        const string& GetExceptionMsg() const { return m_ExceptionMsg; }

        void Summarize() override { xSummarize(); }
    private:
        string m_ExceptionMsg { "CDiscrepancyMock::Visit() dummy exception" };
    };

    CDiscrepancyCaseProps props{nullptr, eTestTypes::STRING, eTestNames::notset, "TestProps", "Dummy props for testing purposes"};
    CDiscrepancyMock mock(&props);

    auto pScope = Ref(new CScope(*(CObjectManager::GetInstance())));
    CDiscrepancyContext context(*pScope);
    mock.Call(context);
    BOOST_CHECK(!mock.Empty());
    mock.Summarize();
    auto reportItems = mock.GetReport();
    BOOST_CHECK(reportItems.size() == 1 && reportItems[0].NotEmpty());
    const auto& item = *(reportItems[0]);
    BOOST_CHECK_EQUAL(item.GetTitle(), "TestProps"); 
    BOOST_CHECK(NStr::EndsWith(item.GetMsg(), mock.GetExceptionMsg()));

    // Should I be surprised that this exception only results in a non-fatal warning?
    BOOST_CHECK(!item.IsFatal());  
    BOOST_CHECK_EQUAL(item.GetSeverity(), CReportItem::eSeverity_warning);
}


BOOST_AUTO_TEST_CASE(Test_TextDescriptions)
{
    auto pEntry = BuildGoodNucProtSet();
    auto pId = Ref(new CSeq_id());
    pId->SetLocal().SetStr("nuc");

    auto pScope = Ref(new CScope(*(CObjectManager::GetInstance())));
    pScope->AddTopLevelSeqEntry(*pEntry);
    auto bsh = pScope->GetBioseqHandle(*pId);
    auto description = CDiscrepancyObject::GetTextObjectDescription(*(bsh.GetCompleteBioseq()), *pScope);
    BOOST_CHECK_EQUAL(description, "nuc");
        
    auto bssh = bsh.GetParentBioseq_set();
    description = CDiscrepancyObject::GetTextObjectDescription(bssh);
    BOOST_CHECK_EQUAL(description, "np|nuc");
}
