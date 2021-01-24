/* gstcef
 * Copyright (C) <2018> Mathieu Duponchelle <mathieu@centricular.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <include/cef_app.h>
#include <glib.h>
#include "include/cef_command_line.h"
#include "include/base/cef_logging.h"
#include <include//wrapper/cef_message_router.h>

int main(int argc, char * argv[])
{
  CefSettings settings;

#ifdef G_OS_WIN32
  HINSTANCE hInstance = GetModuleHandle(NULL);
  CefMainArgs args(hInstance);
#else
  CefMainArgs args(argc, argv);
#endif

    // Parse command-line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(argc, argv);

  if (!command_line->HasSwitch("type"))
    LOG(ERROR) << " BrowserProcess";

  const std::string& process_type = command_line->GetSwitchValue("type");
  if (process_type == "renderer")
    LOG(ERROR) <<  " RendererProcess";
#if defined(OS_LINUX)
  else if (process_type == "zygote")
    LOG(ERROR) << " ZygoteProcess";
#endif
  else
   LOG(ERROR) << " OtherProcess";

  // Implementation of CefApp for the renderer process.
  class RendererApp : public CefApp, public CefRenderProcessHandler {
    public:
      RendererApp() { LOG(ERROR) << "RendererApp on renderer process"; }

      // CefApp methods:
      CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
        LOG(ERROR) << "GetRenderProcessHandler ";
        return this;
      }

      // CefRenderProcessHandler methods:
      void OnWebKitInitialized() OVERRIDE {
          LOG(ERROR) << "OnWebKitInitialized ";
          // Create the renderer-side router for query handling.
          CefMessageRouterConfig config;
          config.js_query_function = "cefQuery";
          config.js_cancel_function = "cefQueryCancel";
          message_router_ = CefMessageRouterRendererSide::Create(config);
      }

      void OnContextCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) OVERRIDE {
          message_router_->OnContextCreated(browser, frame, context);
          LOG(ERROR) << "OnContextCreated ";
      }

      void OnContextReleased(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) OVERRIDE {
          LOG(ERROR) << "OnContextReleased ";
          message_router_->OnContextReleased(browser, frame, context);
      }

      bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) OVERRIDE {
          LOG(ERROR) << "OnProcessMessageReceived ";
          return message_router_->OnProcessMessageReceived(browser, frame,
            source_process, message);
      }

    private:
      // Handles the renderer side of query routing.
      CefRefPtr<CefMessageRouterRendererSide> message_router_;

      IMPLEMENT_REFCOUNTING(RendererApp);
      DISALLOW_COPY_AND_ASSIGN(RendererApp);
    };
  
  CefRefPtr<CefApp> app = nullptr;
  if(process_type == "renderer" || process_type == "zygote")
    app = new RendererApp(); 
  return CefExecuteProcess(args, app.get(), nullptr);
} 	
