#ifndef CASSBLOBOP__HPP
#define CASSBLOBOP__HPP

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
 * Authors: Dmitri Dmitrienko
 *
 * File Description:
 *
 *  BigData application layer
 *
 */

#include <corelib/request_status.hpp>
#include <corelib/ncbidiag.hpp>

#include <atomic>
#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "cass_driver.hpp"
#include "cass_exception.hpp"
#include "Key.hpp"
#include "IdCassScope.hpp"
#include "cass_util.hpp"
#include "blob_record.hpp"
#include "nannot/record.hpp"
#include "id2_split/record.hpp"
#include "acc_ver_hist/record.hpp"

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;

class CCassBlobWaiter;
class CCassBlobTaskLoadBlob;

enum ECassTristate {
    eUnknown,
    eTrue,
    eFalse
};

using TBlobChunkCallback = function<void(const unsigned char * data, unsigned int size, int chunk_no)>;
using TPropsCallback     = function<void(const SBlobStat& stat, bool isFound)>;
using TDataErrorCallback = function<void(
    CRequestStatus::ECode status,
    int code,
    EDiagSev severity,
    const string & message
)>;
using TDataReadyCallback = void(*)(void*);

class CCassBlobWaiter
{
 public:
    CCassBlobWaiter(const CCassBlobWaiter&) = delete;
    CCassBlobWaiter& operator=(const CCassBlobWaiter&) = delete;
    CCassBlobWaiter(CCassBlobWaiter&&) = default;
    CCassBlobWaiter& operator=(CCassBlobWaiter&&) = default;

    /// Use constructor without op_timeout_ms parameter
    /*NCBI_DEPRECATED*/ CCassBlobWaiter(
        unsigned int /*op_timeout_ms deprecated*/,
        shared_ptr<CCassConnection> conn,
        const string & keyspace,
        int32_t key,
        bool async,
        unsigned int max_retries,
        TDataErrorCallback error_cb
    )
      : m_ErrorCb(move(error_cb))
      , m_Conn(conn)
      , m_Async(async)
      , m_Keyspace(keyspace)
      , m_Key(key)
    {
        if (max_retries > numeric_limits<int>::max()) {
            m_MaxRetries = 0;
        } else {
            m_MaxRetries = static_cast<int>(max_retries);
        }
    }

    CCassBlobWaiter(
        shared_ptr<CCassConnection> conn,
        const string & keyspace,
        int32_t key,
        bool async,
        TDataErrorCallback error_cb
    )
      : m_ErrorCb(move(error_cb))
      , m_Conn(move(conn))
      , m_Async(async)
      , m_Keyspace(keyspace)
      , m_Key(key)
    {}

    CCassBlobWaiter(
        shared_ptr<CCassConnection> conn,
        const string & keyspace,
        bool async,
        TDataErrorCallback error_cb
    )
      : m_ErrorCb(move(error_cb))
      , m_Conn(move(conn))
      , m_Async(async)
      , m_Keyspace(keyspace)
    {}

    virtual ~CCassBlobWaiter()
    {
        CloseAll();
    }

    bool Cancelled() const
    {
        return m_Cancelled;
    }

    virtual void Cancel(void)
    {
        if (m_State != eDone) {
            m_Cancelled = true;
            m_State = eError;
        }
    }

    bool Wait(void)
    {
        while (m_State != eDone && m_State != eError && !m_Cancelled) {
            try {
                Wait1();
            } catch (const CCassandraException& e) {
                // We will not re-throw here as CassandraException is not fatal
                Error(CRequestStatus::e500_InternalServerError, e.GetErrCode(), eDiag_Error, e.what());
            } catch (const exception& e) {
                // See ID-6241 There is a requirement to catch all exceptions and continue here
                Error(CRequestStatus::e500_InternalServerError, CCassandraException::eUnknown, eDiag_Error, e.what());
            } catch (...) {
                // See ID-6241 There is a requirement to catch all exceptions and continue here
                Error(CRequestStatus::e500_InternalServerError, CCassandraException::eUnknown, eDiag_Error, "Unknown exception");
            }
            if (m_Async) {
                break;
            }
        }
        return (m_State == eDone || m_State == eError || m_Cancelled);
    }

    bool HasError() const
    {
        return !m_LastError.empty();
    }

    string LastError() const
    {
        return m_LastError;
    }

    void ClearError()
    {
        m_LastError.clear();
    }

    string GetKeySpace() const
    {
        return m_Keyspace;
    }

    void SetKeySpace(string const & keyspace) {
        if (m_State != eInit) {
            NCBI_THROW(CCassandraException, eSeqFailed, "CCassBlobWaiter: Cannot change keyspace for started task");
        }
        m_Keyspace = keyspace;
    }

    /// Use GetKey()
    NCBI_DEPRECATED int32_t key() const
    {
        return m_Key;
    }

    int32_t GetKey() const
    {
        return m_Key;
    }

    void SetErrorCB(TDataErrorCallback error_cb)
    {
        m_ErrorCb = std::move(error_cb);
    }

    /// Set connection point parameters.
    ///
    /// @param value
    ///   Max number of query retries operation allows.
    ///   < 0 means not configured. Will use value provided by CCassConnection
    ///   0 means no limit in auto-restart count,
    ///   1 means no 2nd start -> no re-starts at all
    ///   n > 1 means n-1 restart allowed
    void SetMaxRetries(int value)
    {
        m_MaxRetries = value < 0 ? -1 : value;
    }

    int GetMaxRetries() const
    {
        return m_MaxRetries < 0 ? m_Conn->GetMaxRetries() : m_MaxRetries;
    }

    void SetDataReadyCB3(shared_ptr<CCassDataCallbackReceiver> datareadycb3)
    {
        m_DataReadyCb3 = datareadycb3;
    }

 protected:
    enum EBlobWaiterState {
        eInit = 0,
        eDone = 10000,
        eError = -1
    };
    struct SQueryRec {
        shared_ptr<CCassQuery> query;
        unsigned int restart_count;
    };

    void CloseAll(void)
    {
        for (auto & it : m_QueryArr) {
            it.query->Close();
            it.restart_count = 0;
        }
    }

    void SetupQueryCB3(shared_ptr<CCassQuery>& query)
    {
        auto DataReadyCb3 = m_DataReadyCb3.lock();
        if (DataReadyCb3) {
            query->SetOnData3(DataReadyCb3);
        } else if (IsDataReadyCallbackExpired()) {
            char msg[1024];
            snprintf(msg, sizeof(msg), "Failed to setup data ready callback (expired)");
            Error(CRequestStatus::e502_BadGateway, CCassandraException::eUnknown, eDiag_Error, msg);
        }
    }

    // Returns true for expired non empty weak pointers
    bool IsDataReadyCallbackExpired() const
    {
        using wt = weak_ptr<CCassDataCallbackReceiver>;
        return m_DataReadyCb3.owner_before(wt{}) || wt{}.owner_before(m_DataReadyCb3);
    }

    void Error(CRequestStatus::ECode  status,
               int  code,
               EDiagSev  severity,
               const string &  message)
    {
        assert(m_ErrorCb != nullptr);
        m_State = eError;
        m_LastError = message;
        m_ErrorCb(status, code, severity, message);
    }

    bool CanRestart(shared_ptr<CCassQuery> query, unsigned int restart_count) const
    {
        int max_retries = GetMaxRetries();
        bool is_out_of_retries = (max_retries > 0) &&
                                 (restart_count >= static_cast<unsigned int>(max_retries) - 1);
        ERR_POST(Info << "CanRestartQ?" <<
                 " out_of_retries=" << is_out_of_retries <<
                 ", time=" << gettime() / 1000L <<
                 ", timeout=" << query->Timeout() << "ms");
        return !is_out_of_retries && !m_Cancelled;
    }

    bool CanRestart(SQueryRec& it) const
    {
        return CanRestart(it.query, it.restart_count);
    }

    bool CheckReady(shared_ptr<CCassQuery> qry, unsigned int restart_counter, bool& need_repeat)
    {
        need_repeat = false;
        try {
            if (m_Async && qry->WaitAsync(0) == ar_wait) {
                return false;
            }
            return true;
        } catch (const CCassandraException& e) {
            if (
                (e.GetErrCode() == CCassandraException::eQueryTimeout
                    || e.GetErrCode() == CCassandraException::eQueryFailedRestartable)
                && CanRestart(qry, restart_counter))
            {
                need_repeat = true;
            } else {
                Error(CRequestStatus::e502_BadGateway, e.GetErrCode(), eDiag_Error, e.what());
            }
        }

        return false;
    }

    bool CheckReady(SQueryRec& it)
    {
        bool need_restart = false;
        bool rv = CheckReady(it.query, it.restart_count, need_restart);
        if (!rv && need_restart) {
            try {
                ++it.restart_count;
                ERR_POST(Warning << "CCassBlobWaiter query retry: " + to_string(it.restart_count));
                it.query->Restart();
            } catch (const exception& ex) {
                ERR_POST(NCBI_NS_NCBI::Error << "Failed to restart query (p2): " << ex.what());
                throw;
            }
        }
        return rv;
    }

    CassConsistency GetQueryConsistency(void)
    {
        return CASS_CONSISTENCY_LOCAL_QUORUM;
    }

    bool CheckMaxActive(void);
    virtual void Wait1(void) = 0;

    TDataErrorCallback              m_ErrorCb;
    weak_ptr<CCassDataCallbackReceiver> m_DataReadyCb3;
    shared_ptr<CCassConnection>     m_Conn;
    atomic<int32_t>                 m_State{eInit};
    string                          m_LastError;
    bool                            m_Async;
    atomic_bool                     m_Cancelled{false};
    vector<SQueryRec>               m_QueryArr;

 private:
    string                          m_Keyspace;
    int32_t                         m_Key{0};
    int                             m_MaxRetries{-1};
};

class CCassBlobOp: public enable_shared_from_this<CCassBlobOp>
{
 public:
    enum EBlopOpFlag {
        eFlagOpOr,
        eFlagOpAnd,
        eFlagOpSet
    };

 public:
    CCassBlobOp(CCassBlobOp&&) = default;
    CCassBlobOp& operator=(CCassBlobOp&&) = default;
    CCassBlobOp(const CCassBlobOp&) = delete;
    CCassBlobOp& operator=(const CCassBlobOp&) = delete;

    explicit CCassBlobOp(shared_ptr<CCassConnection> conn)
        : m_Conn(conn)
    {
        m_Keyspace = m_Conn->Keyspace();
    }

    virtual ~CCassBlobOp()
    {
        m_Conn = nullptr;
    }

    NCBI_DEPRECATED void GetBlobChunkSize(unsigned int timeout_ms, int64_t * chunk_size);
    void GetBlobChunkSize(unsigned int timeout_ms, const string & keyspace, int64_t * chunk_size);
    void SetKeyspace(const string &  keyspace)
    {
        m_Keyspace = keyspace;
    }

    string GetKeyspace(void) const
    {
        return m_Keyspace;
    }

    NCBI_DEPRECATED void InsertBlobExtended(unsigned int  op_timeout_ms, unsigned int  max_retries,
                         CBlobRecord *  blob_rslt, TDataErrorCallback  error_cb,
                         unique_ptr<CCassBlobWaiter> &  waiter);
    NCBI_DEPRECATED void InsertNAnnot(unsigned int  op_timeout_ms,
                     int32_t  key, unsigned int  max_retries,
                     CBlobRecord * blob, CNAnnotRecord * annot,
                     TDataErrorCallback error_cb,
                     unique_ptr<CCassBlobWaiter> & waiter);
    NCBI_DEPRECATED void DeleteNAnnot(unsigned int op_timeout_ms,
                     unsigned int max_retries,
                     CNAnnotRecord * annot,
                     TDataErrorCallback error_cb,
                     unique_ptr<CCassBlobWaiter> & waiter);
    NCBI_DEPRECATED void FetchNAnnot(unsigned int op_timeout_ms,
        unsigned int max_retries,
        const string & accession,
        int16_t version,
        int16_t seq_id_type,
        const vector<string>& annot_names,
        TNAnnotConsumeCallback consume_callback,
        TDataErrorCallback error_cb,
        unique_ptr<CCassBlobWaiter> & waiter
    );
    NCBI_DEPRECATED void FetchNAnnot(unsigned int op_timeout_ms,
        unsigned int max_retries,
        const string & accession,
        int16_t version,
        int16_t seq_id_type,
        const vector<CTempString>& annot_names,
        TNAnnotConsumeCallback consume_callback,
        TDataErrorCallback error_cb,
        unique_ptr<CCassBlobWaiter> & waiter
    );
    NCBI_DEPRECATED void FetchNAnnot(unsigned int op_timeout_ms,
        unsigned int max_retries,
        const string & accession,
        int16_t version,
        int16_t seq_id_type,
        TNAnnotConsumeCallback consume_callback,
        TDataErrorCallback error_cb,
        unique_ptr<CCassBlobWaiter> & waiter
    );

    NCBI_DEPRECATED void FetchAccVerHistory(
        unsigned int op_timeout_ms,
        unsigned int max_retries,
        const string & accession,
        TAccVerHistConsumeCallback consume_callback,
        TDataErrorCallback error_cb,
        unique_ptr<CCassBlobWaiter> & waiter,
        int16_t version = 0,
        int16_t seq_id_type = 0
    );


    NCBI_DEPRECATED void DeleteBlobExtended(unsigned int  op_timeout_ms,
                         int32_t  key, unsigned int  max_retries,
                         TDataErrorCallback error_cb,
                         unique_ptr<CCassBlobWaiter> &  waiter);
    NCBI_DEPRECATED void DeleteExpiredBlobVersion(unsigned int op_timeout_ms,
                             int32_t key, CBlobRecord::TTimestamp last_modified,
                             CBlobRecord::TTimestamp expiration,
                             unsigned int  max_retries,
                             TDataErrorCallback error_cb,
                             unique_ptr<CCassBlobWaiter> & waiter);

    NCBI_DEPRECATED unique_ptr<CCassBlobTaskLoadBlob> GetBlobExtended(
        unsigned int op_timeout_ms,
        unsigned int max_retries,
        CBlobRecord::TSatKey sat_key,
        bool load_chunks,
        TDataErrorCallback error_cb
    );

    NCBI_DEPRECATED unique_ptr<CCassBlobTaskLoadBlob> GetBlobExtended(
        unsigned int op_timeout_ms,
        unsigned int max_retries,
        CBlobRecord::TSatKey sat_key,
        CBlobRecord::TTimestamp modified,
        bool load_chunks,
        TDataErrorCallback error_cb
    );

    NCBI_DEPRECATED unique_ptr<CCassBlobTaskLoadBlob> GetBlobExtended(
        unsigned int timeout_ms,
        unsigned int max_retries,
        unique_ptr<CBlobRecord> blob_record,
        bool load_chunks,
        TDataErrorCallback error_cb
    );

    void UpdateBlobFlagsExtended(
        unsigned int op_timeout_ms,
        CBlobRecord::TSatKey key,
        EBlobFlags flag,
        bool set_flag
    );

    void InsertID2Split(
        unsigned int op_timeout_ms,
        unsigned int max_retries,
        CBlobRecord * blob, CID2SplitRecord* id2_split,
        TDataErrorCallback error_cb,
        unique_ptr<CCassBlobWaiter> & waiter);

    // @deprecated Use GetSetting with explicit domain parameter
    NCBI_DEPRECATED bool GetSetting(unsigned int op_timeout_ms, const string & name, string & value);

    // @deprecated Use UpdateSetting with explicit domain parameter
    NCBI_DEPRECATED void UpdateSetting(unsigned int op_timeout_ms, const string & name, const string & value);

    bool GetSetting(unsigned int op_timeout_ms, const string & domain, const string & name, string & value);
    void UpdateSetting(unsigned int op_timeout_ms, const string & domain, const string & name, const string & value);

    shared_ptr<CCassConnection> GetConn(void)
    {
        if (!m_Conn) {
            NCBI_THROW(CCassandraException, eSeqFailed, "CCassBlobOp instance is not initialized with DB connection");
        }
        return m_Conn;
    }

 private:
    shared_ptr<CCassConnection> m_Conn;
    string m_Keyspace;
};

END_IDBLOB_SCOPE

#endif
