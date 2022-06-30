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
 * Authors: Sergey Satskiy
 *
 * File Description: PSG processor interface
 *
 */

#include <ncbi_pch.hpp>

#include "ipsgs_processor.hpp"
#include "pubseq_gateway.hpp"
#include "insdc_utils.hpp"


string
IPSGS_Processor::StatusToString(IPSGS_Processor::EPSGS_Status  status)
{
    switch (status) {
        case IPSGS_Processor::ePSGS_InProgress:
            return "ePSGS_InProgress";
        case IPSGS_Processor::ePSGS_Done:
            return "ePSGS_Done";
        case IPSGS_Processor::ePSGS_NotFound:
            return "ePSGS_NotFound";
        case IPSGS_Processor::ePSGS_Error:
            return "ePSGS_Error";
        case IPSGS_Processor::ePSGS_Canceled:
            return "ePSGS_Canceled";
        case IPSGS_Processor::ePSGS_Timeout:
            return "ePSGS_Timeout";
        default:
            break;
    }
    return "unknown (" + to_string(status) + ")";
}


string
IPSGS_Processor::StatusToProgressMessage(IPSGS_Processor::EPSGS_Status  status)
{
    switch (status) {
        case IPSGS_Processor::ePSGS_InProgress:
            // Note: must not really be called while a processor in this status
            return "inprogress";
        case IPSGS_Processor::ePSGS_Done:
            return "done";
        case IPSGS_Processor::ePSGS_NotFound:
            return "not_found";
        case IPSGS_Processor::ePSGS_Error:
            return "error";
        case IPSGS_Processor::ePSGS_Canceled:
            return "canceled";
        case IPSGS_Processor::ePSGS_Timeout:
            return "timeout";
        default:
            break;
    }
    return "unknown (" + to_string(status) + ")";
}


IPSGS_Processor::EPSGS_StartProcessing
IPSGS_Processor::SignalStartProcessing(void)
{
    return CPubseqGatewayApp::GetInstance()->SignalStartProcessing(this);
}


void IPSGS_Processor::SignalFinishProcessing(void)
{
    if (!m_FinishSignalled) {
        CPubseqGatewayApp::GetInstance()->SignalFinishProcessing(
                                this, CPSGS_Dispatcher::ePSGS_Processor);
        m_FinishSignalled = true;
    }
}


void IPSGS_Processor::PostponeInvoke(CPSGS_UvLoopBinder::TProcessorCB  cb,
                                     void *  user_data)
{
    auto *  app = CPubseqGatewayApp::GetInstance();
    app->GetUvLoopBinder(GetUVThreadId()).PostponeInvoke(cb, user_data,
                                                         m_Request->GetRequestId());
}


bool IPSGS_Processor::GetEffectiveSeqIdType(
    const CSeq_id& parsed_seq_id,
    int request_seq_id_type,
    int16_t& eff_seq_id_type,
    bool need_trace)
{
    auto    parsed_seq_id_type = parsed_seq_id.Which();
    bool    parsed_seq_id_type_found = (parsed_seq_id_type !=
                                        CSeq_id_Base::e_not_set);

    if (!parsed_seq_id_type_found && request_seq_id_type < 0) {
        eff_seq_id_type = -1;
        return true;
    }

    if (!parsed_seq_id_type_found) {
        eff_seq_id_type = request_seq_id_type;
        return true;
    }

    if (request_seq_id_type < 0) {
        eff_seq_id_type = parsed_seq_id_type;
        return true;
    }

    // Both found
    if (parsed_seq_id_type == request_seq_id_type) {
        eff_seq_id_type = request_seq_id_type;
        return true;
    }

    // The parsed and url explicit seq_id_type do not match
    if (IsINSDCSeqIdType(parsed_seq_id_type) &&
        IsINSDCSeqIdType(request_seq_id_type)) {
        if (need_trace) {
            m_Reply->SendTrace(
                "Seq id type mismatch. Parsed CSeq_id reports seq_id_type as " +
                to_string(parsed_seq_id_type) + " while the URL reports " +
                to_string(request_seq_id_type) + ". They both belong to INSDC types so "
                "CSeq_id provided type " + to_string(parsed_seq_id_type) +
                " is taken as an effective one",
                m_Request->GetStartTimestamp());
        }
        eff_seq_id_type = parsed_seq_id_type;
        return true;
    }

    return false;
}


EPSGS_SeqIdParsingResult IPSGS_Processor::ParseInputSeqId(
    CSeq_id& seq_id,
    const string& request_seq_id,
    int request_seq_id_type,
    string* err_msg)
{
    bool    need_trace = m_Request->NeedTrace();

    try {
        seq_id.Set(request_seq_id);
        if (need_trace)
            m_Reply->SendTrace("Parsing CSeq_id('" + request_seq_id +
                             "') succeeded", m_Request->GetStartTimestamp());

        if (request_seq_id_type <= 0) {
            if (need_trace)
                m_Reply->SendTrace("Parsing CSeq_id finished OK (#1)",
                                   m_Request->GetStartTimestamp());
            return ePSGS_ParsedOK;
        }

        // Check the parsed type with the given
        int16_t     eff_seq_id_type;
        if (GetEffectiveSeqIdType(seq_id, request_seq_id_type, eff_seq_id_type, false)) {
            if (need_trace)
                m_Reply->SendTrace("Parsing CSeq_id finished OK (#2)",
                                   m_Request->GetStartTimestamp());
            return ePSGS_ParsedOK;
        }

        // seq_id_type from URL and from CSeq_id differ
        CSeq_id_Base::E_Choice  seq_id_type = seq_id.Which();

        if (need_trace)
            m_Reply->SendTrace("CSeq_id provided type " + to_string(seq_id_type) +
                               " and URL provided seq_id_type " +
                               to_string(request_seq_id_type) + " mismatch",
                               m_Request->GetStartTimestamp());

        if (IsINSDCSeqIdType(request_seq_id_type) &&
            IsINSDCSeqIdType(seq_id_type)) {
            // Both seq_id_types belong to INSDC
            if (need_trace) {
                m_Reply->SendTrace("Both types belong to INSDC types.\n"
                                   "Parsing CSeq_id finished OK (#3)",
                                   m_Request->GetStartTimestamp());
            }
            return ePSGS_ParsedOK;
        }

        // Type mismatch: form the error message in case of resolution problems
        if (err_msg) {
            *err_msg = "Seq_id '" + request_seq_id +
                      "' possible type mismatch: the URL provides " +
                      to_string(request_seq_id_type) +
                      " while the CSeq_Id detects it as " +
                      to_string(static_cast<int>(seq_id_type));
        }
    } catch (...) {
        if (need_trace)
            m_Reply->SendTrace("Parsing CSeq_id('" + request_seq_id +
                               "') failed (exception)",
                               m_Request->GetStartTimestamp());
    }

    // Second variation of Set()
    if (request_seq_id_type > 0) {
        try {
            seq_id.Set(CSeq_id::eFasta_AsTypeAndContent,
                       (CSeq_id_Base::E_Choice)(request_seq_id_type),
                       request_seq_id);
            if (need_trace) {
                m_Reply->SendTrace("Parsing CSeq_id(eFasta_AsTypeAndContent, " +
                                   to_string(request_seq_id_type) +
                                   ", '" + request_seq_id + "') succeeded.\n"
                                   "Parsing CSeq_id finished OK (#4)",
                                   m_Request->GetStartTimestamp());
            }
            return ePSGS_ParsedOK;
        } catch (...) {
            if (need_trace)
                m_Reply->SendTrace("Parsing CSeq_id(eFasta_AsTypeAndContent, " +
                                   to_string(request_seq_id_type) +
                                   ", '" + request_seq_id + "') failed (exception)",
                                   m_Request->GetStartTimestamp());
        }
    }

    if (need_trace) {
        m_Reply->SendTrace("Parsing CSeq_id finished FAILED",
                           m_Request->GetStartTimestamp());
    }

    return ePSGS_ParseFailed;
}
