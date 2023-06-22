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
 */

#include <ncbi_pch.hpp>

#include "http_connection.hpp"
#include "http_reply.hpp"
#include "pubseq_gateway.hpp"



static void IncrementBackloggedCounter(void)
{
    auto *  app = CPubseqGatewayApp::GetInstance();
    app->GetCounters().Increment(nullptr, CPSGSCounters::ePSGS_BackloggedRequests);
}


static void IncrementTooManyRequestsCounter(void)
{
    auto *  app = CPubseqGatewayApp::GetInstance();
    app->GetCounters().Increment(nullptr, CPSGSCounters::ePSGS_TooManyRequests);
}


static void NotifyRequestFinished(size_t  request_id)
{
    auto *  app = CPubseqGatewayApp::GetInstance();
    app->NotifyRequestFinished(request_id);
}



CHttpConnection::~CHttpConnection()
{
    // Note: this should not happened that there are records in the pending
    // and backlog lists. The reason is that the CHttpConnection instances
    // are reused and each time the are recycled the ResetForReuse() is
    // called. So the check below is rather a sanity check.
    while (!m_BacklogRequests.empty()) {
        backlog_list_iterator_t     it = m_BacklogRequests.begin();
        x_UnregisterBacklog(it);
    }

    while (!m_RunningRequests.empty()) {
        auto it = m_RunningRequests.begin();
        (*it)->GetHttpReply()->CancelPending();
        x_UnregisterRunning(it);
    }
}


void CHttpConnection::SetupMaintainTimer(uv_loop_t *  tcp_worker_loop)
{
    uv_timer_init(tcp_worker_loop, &m_ScheduledMaintainTimer);
    m_ScheduledMaintainTimer.data = (void *)(this);
}


void CHttpConnection::CleanupToStop(void)
{
    uv_timer_stop(&m_ScheduledMaintainTimer);
    uv_close(reinterpret_cast<uv_handle_t*>(&m_ScheduledMaintainTimer), nullptr);
}


void CHttpConnection::ResetForReuse(void)
{
    while (!m_BacklogRequests.empty()) {
        backlog_list_iterator_t     it = m_BacklogRequests.begin();
        x_UnregisterBacklog(it);
    }

    while (!m_RunningRequests.empty()) {
        auto it = m_RunningRequests.begin();
        (*it)->GetHttpReply()->CancelPending();
        x_UnregisterRunning(it);
    }

    m_IsClosed = false;
}


void CHttpConnection::PeekAsync(bool  chk_data_ready)
{
    for (auto &  it: m_RunningRequests) {
        if (!chk_data_ready ||
            it->GetHttpReply()->CheckResetDataTriggered()) {
            it->GetHttpReply()->PeekPending();
        }
    }

    x_MaintainFinished();
    x_MaintainBacklog();
}


void CHttpConnection::DoScheduledMaintain(void)
{
    x_MaintainFinished();
    x_MaintainBacklog();
}


void MaintanTimerCB(uv_timer_t *  handle)
{
    CHttpConnection *       http_connection = (CHttpConnection *)(handle->data);
    http_connection->DoScheduledMaintain();
}

void CHttpConnection::ScheduleMaintain(void)
{
    // Send one time async event to initiate maintain
    if (uv_is_active((uv_handle_t*)(&m_ScheduledMaintainTimer))) {
        return; // has already been activated
    }

    m_ScheduledMaintainTimer.data = (void *)(this);
    uv_timer_start(&m_ScheduledMaintainTimer, MaintanTimerCB, 0, 0);
}


void CHttpConnection::x_RegisterPending(shared_ptr<CPSGS_Request>  request,
                                        shared_ptr<CPSGS_Reply>  reply,
                                        list<string>  processor_names)
{
    if (m_RunningRequests.size() < m_HttpMaxRunning) {
        x_Start(request, reply, move(processor_names));
    } else if (m_BacklogRequests.size() < m_HttpMaxBacklog) {
        m_BacklogRequests.push_back(
                SBacklogAttributes{request, reply,
                                   move(processor_names),
                                   psg_clock_t::now()});
        IncrementBackloggedCounter();
    } else {
        IncrementTooManyRequestsCounter();

        reply->SetContentType(ePSGS_PSGMime);
        reply->PrepareReplyMessage("Too many pending requests",
                                   CRequestStatus::e503_ServiceUnavailable,
                                   ePSGS_TooManyRequests, eDiag_Error);
        reply->PrepareReplyCompletion(CRequestStatus::e503_ServiceUnavailable,
                                      psg_clock_t::now());
        reply->Flush(CPSGS_Reply::ePSGS_SendAndFinish);
        reply->SetCompleted();
    }
}


void
CHttpConnection::x_Start(shared_ptr<CPSGS_Request>  request,
                         shared_ptr<CPSGS_Reply>  reply,
                         list<string>  processor_names)
{
    auto    http_reply = reply->GetHttpReply();
    if (!http_reply->IsPostponed())
        NCBI_THROW(CPubseqGatewayException, eRequestNotPostponed,
                   "Request has not been postponed");
    if (IsClosed())
        NCBI_THROW(CPubseqGatewayException, eConnectionClosed,
                   "Request handling can not be started after connection was closed");

    // Try to instantiate the processors in accordance to the pre-dispatched
    // names.
    auto *  app = CPubseqGatewayApp::GetInstance();
    list<shared_ptr<IPSGS_Processor>>   processors =
            app->GetProcessorDispatcher()->DispatchRequest(request, reply, processor_names);

    if (processors.empty()) {
        // No processors were actually instantiated
        // The reply is completed by the dispatcher with all necessary messages.
        return;
    }

    request->SetConcurrentProcessorCount(processors.size());
    for (auto & processor : processors) {
        reply->GetHttpReply()->AssignPendingReq(
                move(unique_ptr<CPendingOperation>(
                    new CPendingOperation(request, reply, processor))));
    }

    // Add the reply to the list of running replies
    m_RunningRequests.push_back(reply);

    // To avoid a possibility to have cancel->start in progress messages in the
    // reply in case of multiple processors due to the first one may do things
    // synchronously and call Cancel() for the other processors right away: the
    // sending of the start message will be done for all of them before the
    // actual Start() call
    for (auto req: http_reply->GetPendingReqs())
        req->SendProcessorStartMessage();

    // Start the request timer
    app->GetProcessorDispatcher()->StartRequestTimer(reply->GetRequestId());

    // Now start the processors
    for (auto req: http_reply->GetPendingReqs())
        req->Start();
}


void CHttpConnection::Postpone(shared_ptr<CPSGS_Request>  request,
                               shared_ptr<CPSGS_Reply>  reply,
                               list<string>  processor_names)
{
    auto    http_reply = reply->GetHttpReply();
    switch (http_reply->GetState()) {
        case CHttpReply::eReplyInitialized:
            if (http_reply->IsPostponed())
                NCBI_THROW(CPubseqGatewayException,
                           eRequestAlreadyPostponed,
                           "Request has already been postponed");
            break;
        case CHttpReply::eReplyStarted:
            NCBI_THROW(CPubseqGatewayException, eRequestCannotBePostponed,
                       "Request that has already started "
                       "can't be postponed");
            break;
        default:
            NCBI_THROW(CPubseqGatewayException, eRequestAlreadyFinished,
                       "Request has already been finished");
            break;
    }

    http_reply->SetPostponed();
    x_RegisterPending(request, reply, move(processor_names));
}


void CHttpConnection::x_CancelAll(void)
{
    x_CancelBacklog();
    for (auto &  it: m_RunningRequests) {
        if (!it->IsFinished()) {
            auto    http_reply = it->GetHttpReply();
            http_reply->CancelPending();
            http_reply->PeekPending();
        }
    }
    x_MaintainFinished();
}


void CHttpConnection::x_UnregisterRunning(running_list_iterator_t &  it)
{
    size_t request_id = (*it)->GetRequestId();

    // Note: without this call there will be memory leaks.
    // The infrastructure holds a shared_ptr to the reply, the pending
    // operation instance also holds a shared_ptr to the very same reply
    // and the reply holds a shared_ptr to the pending operation instance.
    // All together it forms a loop which needs to be broken for a correct
    // memory management.
    // The call below resets a shared_ptr to the pending operation. It is
    // safe to do it here because this point is reached only when all
    // activity on processing a request is over.
    (*it)->GetHttpReply()->ResetPendingRequest();

    m_RunningRequests.erase(it);
    NotifyRequestFinished(request_id);
}


void CHttpConnection::x_UnregisterBacklog(backlog_list_iterator_t &  it)
{
    // The backlogged requests have not started processing yet. They do not
    // have any created processors. So there is no need to notify a dispatcher
    // that the request is finished.
    // They do not have assigned pending requests either. So, just remove the
    // record from the list.
    m_BacklogRequests.erase(it);
}


void CHttpConnection::x_CancelBacklog(void)
{
    while (!m_BacklogRequests.empty()) {
        auto it = m_BacklogRequests.begin();
        shared_ptr<CPSGS_Reply>  reply = it->m_Reply;

        reply->GetHttpReply()->CancelPending();
        x_UnregisterBacklog(it);
    }
}


void CHttpConnection::x_MaintainFinished(void)
{
    running_list_iterator_t     it = m_RunningRequests.begin();
    while (it != m_RunningRequests.end()) {
        if ((*it)->IsCompleted()) {
            auto    next = it;
            ++next;
            x_UnregisterRunning(it);
            it = next;
        } else {
            ++it;
        }
    }
}


void CHttpConnection::x_MaintainBacklog(void)
{
    while (m_RunningRequests.size() < m_HttpMaxRunning &&
           !m_BacklogRequests.empty()) {
        auto &  backlog_front = m_BacklogRequests.front();

        shared_ptr<CPSGS_Request>  request = backlog_front.m_Request;
        shared_ptr<CPSGS_Reply>    reply = backlog_front.m_Reply;
        list<string>               processor_names = backlog_front.m_PreliminaryDispatchedProcessors;
        psg_time_point_t           backlog_start = backlog_front.m_BacklogStart;

        m_BacklogRequests.pop_front();

        auto *      app = CPubseqGatewayApp::GetInstance();
        uint64_t    mks = app->GetTiming().Register(nullptr, eBacklog,
                                                    eOpStatusFound,
                                                    backlog_start);
        if (mks == 0) {
            // The timing was disabled
            auto            now = psg_clock_t::now();
            mks = chrono::duration_cast<chrono::microseconds>(now - backlog_start).count();
        }
        request->SetBacklogTime(mks);

        auto    context = request->GetRequestContext();
        if (context.NotNull()) {
            CRequestContextResetter     context_resetter;
            request->SetRequestContext();

            GetDiagContext().Extra().Print("backlog_time_mks", mks);
        }

        x_Start(request, reply, move(processor_names));
    }
}

