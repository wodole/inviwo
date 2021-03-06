/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/webbrowser/processors/webbrowserprocessor.h>
#include <modules/webbrowser/webbrowserapp.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>

#include <modules/opengl/shader/shadermanager.h>

// Autogenerated
#include <modules/webbrowser/shader_resources.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include <warn/pop>

namespace inviwo {

WebBrowserModule::WebBrowserModule(InviwoApplication* app)
    : InviwoModule(app, "WebBrowser")
    // Call 60 times per second
    , doChromiumWork_(Timer::Milliseconds(1000 / 60), []() { CefDoMessageLoopWork(); }) {

    if (!app->getSystemSettings().enablePickingProperty_) {
		LogInfo("Enabling picking system setting since it is required for interaction (View->Settings->System settings->Enable picking).");
		app->getSystemSettings().enablePickingProperty_.set(true);
    }
    // CEF initialization

    CefMainArgs args;
    CefSettings settings;
#ifdef WIN32
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();
#endif

    // checkout detailed settings options
    // http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html nearly all
    // the settings can be set via args too.
    settings.multi_threaded_message_loop = false;  // not supported, except windows
    // We want to use off-screen rendering
    settings.windowless_rendering_enabled = true;
    // CefString(&settings.cache_path).FromASCII("");
    // CefString(&settings.log_file).FromASCII("");
    // settings.log_severity = LOGSEVERITY_DEFAULT;
    auto locale = app->getUILocale().name();
    if (locale == "C") {
        // Crash when default locale "C" is used. Reproduce with GLFWMinimum application
        locale = std::locale("en_US").name();
    }
    // Specify the path for the sub-process executable.
    auto exeExtension = filesystem::getFileExtension(filesystem::getExecutablePath());
    // Assume that inviwo_web_helper is next to the main executable
    auto exeDirectory = filesystem::getFileDirectory(filesystem::getExecutablePath());
    auto subProcessExecutable = exeDirectory + "/inviwo_web_helper." + exeExtension;
#ifdef DARWIN  // Mac specific

    // CEF framework and inviwo_web_helper are in the bin/build_config directory during development
    // and in the exe.app/Contents/Frameworks directory if installed. Search in
    // exe.app/Contents/Frameworks directory first
    auto cefParentDir = filesystem::getCanonicalPath(exeDirectory + std::string("/.."));
    if (!filesystem::fileExists(cefParentDir + std::string("/Frameworks/inviwo_web_helper.app"))) {
        // Assume that we are in development: exe.app/Contents/MacOS/../../..;
        cefParentDir = filesystem::getCanonicalPath(exeDirectory + std::string("/../../.."));
    }
    CefString(&settings.framework_dir_path)
        .FromASCII((cefParentDir + "/Frameworks/Chromium Embedded Framework.framework").c_str());
    // Crashes if not set and non-default locale is used
    CefString(&settings.locales_dir_path)
        .FromASCII((cefParentDir +
                    std::string("/Frameworks/Chromium Embedded Framework.framework/Resources"))
                       .c_str());
    CefString(&settings.resources_dir_path)
        .FromASCII((cefParentDir +
                    std::string("/Frameworks/Chromium Embedded Framework.framework/Resources"))
                       .c_str());
    // Locale returns "en_US.UFT8" but "en.UTF8" is needed by CEF
    auto startErasePos = locale.find('_');
    if (startErasePos != std::string::npos) {
        locale.erase(startErasePos, locale.find('.') - startErasePos);
    }

    // Web helper executable should be located in Frameworks dir of bundle when installed
    // and bin dir during development, see OS_MACOSX part in CMakeLists.txt
    if (!filesystem::fileExists(subProcessExecutable)) {
        subProcessExecutable =
            cefParentDir +
            std::string("/Frameworks/inviwo_web_helper.app/Contents/MacOS/inviwo_web_helper");
        if (!filesystem::fileExists(subProcessExecutable)) {
            // We are not using an installed version. Assume that it is in bin/configuration
            // directory
            subProcessExecutable =
                cefParentDir +
                std::string("/inviwo_web_helper.app/Contents/MacOS/inviwo_web_helper");
        }
    }

#endif

    CefString(&settings.locale).FromASCII(locale.c_str());

    if (!filesystem::fileExists(subProcessExecutable)) {
        throw ModuleInitException("Could not find web helper executable:" + subProcessExecutable);
    }
    CefString(&settings.browser_subprocess_path).FromASCII(subProcessExecutable.c_str());
    
    // Optional implementation of the CefApp interface.
    CefRefPtr<WebBrowserApp> browserApp(new WebBrowserApp);

    bool result = CefInitialize(args, settings, browserApp, nullptr);

    if (!result) {
        throw ModuleInitException("Failed to initialize Chromium Embedded Framework");
    }

    // Add a directory to the search path of the Shadermanager
    webbrowser::addShaderResources(ShaderManager::getPtr(), {getPath(ModulePath::GLSL)});
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<WebBrowserProcessor>();

    doChromiumWork_.start();
}

WebBrowserModule::~WebBrowserModule() {
    // Stop message pumping and make sure that app has finished processing before CefShutdown
    doChromiumWork_.stop();
    app_->waitForPool();
    CefShutdown();
}
}  // namespace inviwo
