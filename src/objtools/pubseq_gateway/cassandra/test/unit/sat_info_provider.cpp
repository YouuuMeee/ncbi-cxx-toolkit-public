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
* Author:  Dmitrii Saprykin, NCBI
*
* File Description:
*   Unit test suite for CSatInfoSchemaProvider
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include <corelib/ncbireg.hpp>

#include <objtools/pubseq_gateway/impl/cassandra/blob_storage.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/cass_driver.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/cass_factory.hpp>

BEGIN_IDBLOB_SCOPE

bool operator==(const SSatInfoEntry& a, const SSatInfoEntry& b) {
    return a.keyspace == b.keyspace
        && a.connection == b.connection
        && a.schema_type == b.schema_type
        && a.sat == b.sat;
}

END_IDBLOB_SCOPE


namespace {

USING_NCBI_SCOPE;
USING_IDBLOB_SCOPE;

class CSatInfoProviderTest
    : public testing::Test
{
 public:
    CSatInfoProviderTest() = default;

 protected:
    static void SetUpTestCase() {
        m_Registry.Set(m_ConfigSection, "service", string(m_TestClusterName), IRegistry::fPersistent);
        m_RegistryPtr = make_shared<CCompoundRegistry>();
        m_RegistryPtr->Add(m_Registry);
        m_Factory = CCassConnectionFactory::s_Create();
        m_Factory->LoadConfig(m_RegistryPtr.get(), m_ConfigSection);
        m_Connection = m_Factory->CreateInstance();
        m_Connection->Connect();
    }

    static void TearDownTestCase() {
        m_Connection->Close();
        m_Connection = nullptr;
        m_Factory = nullptr;
    }

    static const char* m_TestClusterName;
    static const char * m_ConfigSection;
    static shared_ptr<CCassConnectionFactory> m_Factory;
    static shared_ptr<CCassConnection> m_Connection;
    static CNcbiRegistry m_Registry;
    static shared_ptr<CCompoundRegistry> m_RegistryPtr;

    string m_KeyspaceName{"sat_info3"};
};

const char* CSatInfoProviderTest::m_TestClusterName = "ID_CASS_TEST";
const char* CSatInfoProviderTest::m_ConfigSection = "TEST";
shared_ptr<CCassConnectionFactory> CSatInfoProviderTest::m_Factory(nullptr);
shared_ptr<CCassConnection> CSatInfoProviderTest::m_Connection(nullptr);
CNcbiRegistry CSatInfoProviderTest::m_Registry;
shared_ptr<CCompoundRegistry> CSatInfoProviderTest::m_RegistryPtr(nullptr);

TEST_F(CSatInfoProviderTest, Smoke) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(false));
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(false));
    EXPECT_EQ("", provider.GetRefreshErrorMessage());
}

TEST_F(CSatInfoProviderTest, DomainDoesNotExist) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "DomainDoesNotExist", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoSat2KeyspaceEmpty, provider.RefreshSchema(false));
    EXPECT_FALSE(provider.GetRefreshErrorMessage().empty());
}

TEST_F(CSatInfoProviderTest, EmptySatInfoName) {
    CSatInfoSchemaProvider provider(
        "", "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoKeyspaceUndefined, provider.RefreshSchema(false));
    EXPECT_EQ("mapping_keyspace is not specified", provider.GetRefreshErrorMessage());
}

TEST_F(CSatInfoProviderTest, Basic) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(true));
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUnchanged, provider.RefreshSchema(false));
    auto sat4 = provider.GetBlobKeyspace(4);
    auto sat5 = provider.GetBlobKeyspace(5);
    auto sat23 = provider.GetBlobKeyspace(23);
    auto sat24 = provider.GetBlobKeyspace(24);
    auto sat52 = provider.GetBlobKeyspace(52);
    auto sat1000 = provider.GetBlobKeyspace(1000);
    ASSERT_TRUE(sat4.has_value());
    ASSERT_TRUE(sat5.has_value());
    ASSERT_TRUE(sat23.has_value());
    ASSERT_TRUE(sat24.has_value());
    ASSERT_FALSE(sat1000.has_value());

    ASSERT_NE(nullptr, provider.GetResolverKeyspace().connection);
    ASSERT_NE(nullptr, sat4.value().connection);
    ASSERT_NE(nullptr, sat5.value().connection);
    ASSERT_NE(nullptr, sat23.value().connection);
    ASSERT_NE(nullptr, sat24.value().connection);
    ASSERT_NE(nullptr, sat52.value().connection);

    ASSERT_EQ("idmain2", provider.GetResolverKeyspace().keyspace);
    ASSERT_EQ("satncbi_extended", sat4.value().keyspace);
    ASSERT_EQ("satprot_v2", sat5.value().keyspace);
    ASSERT_EQ("satddbj_wgs", sat23.value().keyspace);
    ASSERT_EQ("satold03", sat24.value().keyspace);
    ASSERT_EQ("nannotg3", sat52.value().keyspace);

    EXPECT_EQ(sat4.value().connection.get(), sat5.value().connection.get());
    EXPECT_EQ(sat4.value().connection.get(), sat24.value().connection.get());
    EXPECT_NE(sat4.value().connection.get(), sat23.value().connection.get());

    EXPECT_EQ("DC1", sat4.value().connection->GetDatacenterName());
    EXPECT_EQ("BETHESDA", sat23.value().connection->GetDatacenterName());

    vector<SSatInfoEntry> na_keyspaces = {sat52.value()};
    EXPECT_EQ(na_keyspaces, provider.GetNAKeyspaces());

    // Reload from other domain
    provider.SetDomain("PSG_CASS_UNIT2");
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(false));
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(true));

    auto sat23_reloaded = provider.GetBlobKeyspace(23);
    ASSERT_TRUE(sat23_reloaded.has_value());
    // Connection should be inherited from previous schema version (no Cassandra reconnect)
    EXPECT_EQ(sat23_reloaded.value().connection.get(), sat23.value().connection.get());
    EXPECT_EQ(230, provider.GetMaxBlobKeyspaceSat());

    // Reload from other domain
    provider.SetDomain("PSG_CASS_UNIT2.1");
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(false));
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eSatInfoUpdated, provider.RefreshSchema(true));

    auto sat230 = provider.GetBlobKeyspace(230);
    ASSERT_TRUE(sat230.has_value());
    // Connection should be inherited from previous schema version (no Cassandra reconnect)
    EXPECT_EQ(sat230.value().connection.get(), sat23.value().connection.get());
}

TEST_F(CSatInfoProviderTest, NotLoaded) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    ASSERT_EQ(nullopt, provider.GetBlobKeyspace(23));
    ASSERT_EQ(nullptr, provider.GetResolverKeyspace().connection);
    ASSERT_EQ("", provider.GetResolverKeyspace().keyspace);
    ASSERT_TRUE(provider.GetNAKeyspaces().empty());
    EXPECT_EQ(-1, provider.GetMaxBlobKeyspaceSat());
}

TEST_F(CSatInfoProviderTest, ServiceResolutionErrors) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT3", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eLbsmServiceNotResolved, provider.RefreshSchema(true));
    EXPECT_FALSE(provider.GetRefreshErrorMessage().empty());

    provider.SetDomain("PSG_CASS_UNIT4");
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eLbsmServiceNotResolved, provider.RefreshSchema(true));
    EXPECT_FALSE(provider.GetRefreshErrorMessage().empty());

    provider.SetDomain("PSG_CASS_UNIT5");
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eLbsmServiceNotResolved, provider.RefreshSchema(true));
    EXPECT_FALSE(provider.GetRefreshErrorMessage().empty());
}

TEST_F(CSatInfoProviderTest, ResolverKeyspaceDuplicated) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT6", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eResolverKeyspaceDuplicated, provider.RefreshSchema(true));
}

TEST_F(CSatInfoProviderTest, ResolverKeyspaceUndefined) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT7", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eResolverKeyspaceUndefined, provider.RefreshSchema(true));
}

TEST_F(CSatInfoProviderTest, BlobKeyspacesUndefined) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT8", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshSchemaResult::eBlobKeyspacesEmpty, provider.RefreshSchema(true));
}

TEST_F(CSatInfoProviderTest, EmptyMessages) {
    {
        CSatInfoSchemaProvider provider(
            m_KeyspaceName, "PSG_CASS_UNIT7", m_Connection,
            m_RegistryPtr, m_ConfigSection
        );
        EXPECT_EQ("", provider.GetMessage("ANY"));
        EXPECT_EQ(ESatInfoRefreshMessagesResult::eSatInfoMessagesEmpty, provider.RefreshMessages(false));
    }
    {
        CSatInfoSchemaProvider provider(
            "", "PSG_CASS_UNIT7", m_Connection,
            m_RegistryPtr, m_ConfigSection
        );
        EXPECT_EQ(ESatInfoRefreshMessagesResult::eSatInfoKeyspaceUndefined, provider.RefreshMessages(false));
    }
}

TEST_F(CSatInfoProviderTest, BasicMessages) {
    CSatInfoSchemaProvider provider(
        m_KeyspaceName, "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_EQ(ESatInfoRefreshMessagesResult::eMessagesUpdated, provider.RefreshMessages(false));
    EXPECT_EQ(ESatInfoRefreshMessagesResult::eMessagesUpdated, provider.RefreshMessages(true));
    EXPECT_EQ(ESatInfoRefreshMessagesResult::eMessagesUnchanged, provider.RefreshMessages(true));
    EXPECT_EQ("MESSAGE1_TEXT", provider.GetMessage("MESSAGE1"));
    EXPECT_EQ("", provider.GetMessage("MESSAGE2"));
}

TEST_F(CSatInfoProviderTest, CassException) {
    CSatInfoSchemaProvider provider(
        "NON_EXISTENT_KEYSPACE", "PSG_CASS_UNIT1", m_Connection,
        m_RegistryPtr, m_ConfigSection
    );
    EXPECT_THROW(provider.RefreshMessages(true), CCassandraException);
    EXPECT_THROW(provider.RefreshSchema(true), CCassandraException);
}


}  // namespace