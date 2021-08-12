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
 * Author:  Aleksey Grichenko
 *
 * File Description:
 *   Plain example of a FastCGI MT application.
 *
 *   USAGE:  fcgi_mt_sample.fcgi?message=Some+Message
 *
 *   NOTE: needs HTML template file "cgi_sample.html" in curr. dir to run,
 *
 */

#include <ncbi_pch.hpp>
#include <cgi/fcgiapp_mt.hpp>
#include <cgi/cgictx.hpp>
#include <html/html.hpp>
#include <html/page.hpp>

// To get CGI client API (in-house only, optional)
// #include <connect/ext/ncbi_localnet.h>

USING_NCBI_SCOPE;


/////////////////////////////////////////////////////////////////////////////
//  CFastCgiMTSampleApplication::
//

class CFastCgiMTSampleApplication : public CFastCgiApplicationMT
{
public:
    void Init(void) override;
    CCgiRequestProcessor* CreateRequestProcessor(void) override;

private:
    // These 2 functions just demonstrate the use of cmd-line argument parsing
    // mechanism in CGI application -- for the processing of both cmd-line
    // arguments and HTTP entries
    void x_SetupArgs(void);
};


/////////////////////////////////////////////////////////////////////////////
//  CFastCgiMTSampleRequestProcessor::
//

class CFastCgiMTSampleRequestProcessor : public CCgiRequestProcessorMT
{
public:
    CFastCgiMTSampleRequestProcessor(CFastCgiMTSampleApplication& app)
        : CCgiRequestProcessorMT(app) {}
    ~CFastCgiMTSampleRequestProcessor(void) override {}

    int ProcessRequest(CCgiContext& context) override;

private:
    void x_LookAtArgs(void);
};


void CFastCgiMTSampleApplication::Init()
{
    // Standard CGI framework initialization
    CCgiApplication::Init();

    // Describe possible cmd-line and HTTP entries
    // (optional)
    x_SetupArgs();
}


CCgiRequestProcessor* CFastCgiMTSampleApplication::CreateRequestProcessor(void)
{
    return new CFastCgiMTSampleRequestProcessor(*this);
}


void CFastCgiMTSampleApplication::x_SetupArgs()
{
    // Disregard the case of CGI arguments
    SetRequestFlags(CCgiRequest::fCaseInsensitiveArgs);

    // Create CGI argument descriptions class
    //  (For CGI applications only keys can be used)
    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "FastCGI MT sample application");
        
    arg_desc->AddOptionalKey("message",
                             "message",
                             "Message passed to CGI application",
                             CArgDescriptions::eString,
                             CArgDescriptions::fAllowMultiple);

    arg_desc->AddDefaultKey("delay",
                            "delay",
                            "Delay before sending reply, seconds",
                            CArgDescriptions::eInteger, "0");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


int CFastCgiMTSampleRequestProcessor::ProcessRequest(CCgiContext& ctx)
{
    // Parse, verify, and look at cmd-line and CGI parameters via "CArgs"
    // (optional)
    x_LookAtArgs();

    int delay = GetArgs()["delay"].AsInteger();
    if ( delay ) SleepSec(delay);

    // Given "CGI context", get access to its "HTTP request" and
    // "HTTP response" sub-objects
    const CCgiRequest& request  = ctx.GetRequest();
    CCgiResponse&      response = ctx.GetResponse();

    /*
    // To get CGI client API (in-house only, optional)
    const char* const* client_tracking_env = request.GetClientTrackingEnv();
    unsigned int client_ip = NcbiGetCgiClientIP(eCgiClientIP_TryAll, client_tracking_env);
    int is_local_client = NcbiIsLocalCgiClient(client_tracking_env);
    */
   
    // Try to retrieve the message ('message=...') from the HTTP request.
    // NOTE:  the case sensitivity was turned off in Init().
    bool is_message = false;
    string message = request.GetEntry("Message", &is_message);
    if ( is_message ) {
        message = "'" + message + "'";
    } else {
        message = "<NONE>";
    }

    // NOTE:  While this sample uses the CHTML* classes for generating HTML,
    // you are encouraged to use XML/XSLT and the NCBI port of XmlWrapp.
    // For more info:
    //   http://ncbi.github.io/cxx-toolkit/pages/ch_xmlwrapp
    //   http://www.ncbi.nlm.nih.gov/IEB/ToolBox/CPP_DOC/doxyhtml/namespacexml.html

    // Create a HTML page (using template HTML file "cgi_sample.html")
    unique_ptr<CHTMLPage> page;
    try {
        // Find page template
        string html_path = "cgi_sample.html";
        if ( !CFile(html_path).Exists() ) {
            html_path = CDirEntry::ConcatPath(CDirEntry(GetApp().GetProgramExecutablePath()).GetDir(), html_path);
        }
        page.reset(new CHTMLPage("Sample CGI", html_path));
    } catch (exception& e) {
        ERR_POST("Failed to create Sample CGI HTML page: " << e.what());
        return 2;
    }

    // Register substitution for the template parameters <@MESSAGE@> and
    // <@SELF_URL@>
    try {
        CHTMLPlainText* text = new CHTMLPlainText(message);
        _TRACE("foo");
        page->AddTagMap("MESSAGE", text);

        CHTMLPlainText* self_url = new CHTMLPlainText(ctx.GetSelfURL());
        page->AddTagMap("SELF_URL", self_url);
    }
    catch (exception& e) {
        ERR_POST("Failed to populate Sample CGI HTML page: " << e.what());
        return 3;
    }

    // Compose and flush the resultant HTML page
    try {
        _TRACE("stream status: " << ctx.GetStreamStatus());
        response.WriteHeader();
        if (request.GetRequestMethod() != CCgiRequest::eMethod_HEAD) {
            page->Print(response.out(), CNCBINode::eHTML);
        }
    } catch (CCgiHeadException&) {
        throw;
    } catch (exception& e) {
        ERR_POST("Failed to compose/send Sample CGI HTML page: " << e.what());
        return 4;
    }

    return 0;
}


void CFastCgiMTSampleRequestProcessor::x_LookAtArgs()
{
    // You can catch CArgException& here to process argument errors,
    // or you can handle it in OnException()
    const CArgs& args = GetArgs();

    // "args" now contains both command line arguments and the arguments 
    // extracted from the HTTP request

    if ( args["message"] ) {
        // get the first "message" argument only...
        const string& m = args["message"].AsString();
        (void) m.c_str(); // just get rid of compiler warning about unused variable

        // ...or get the whole list of "message" arguments
        const auto& values = args["message"].GetStringList();  // const CArgValue::TStringArray& 

        for (const auto& v : values) {
            // do something with each message 'v' (string)
            (void) v.c_str(); // just get rid of compiler warning about unused variable
        } 
    } else {
        // no "message" argument is present
    }
}


/////////////////////////////////////////////////////////////////////////////
//  MAIN
//

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    return CFastCgiMTSampleApplication().AppMain(argc, argv);
}
