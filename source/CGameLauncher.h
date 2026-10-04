/*
* CGameLauncher.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#include "globals.h"

class TfmMain;

class CGameLauncher
{
public:
    static TGameLaunchResult Connect(TfmMain* Form, const String& Server, const String& Port, const String& Password);
    static TGameLaunchResult Debug(TfmMain* Form, const String& DebugScript);
    static void HandleComplete(TfmMain* Form, TMessage& Message);
    static void Shutdown(TfmMain* Form);

    static void ResumeDnsConnect(TfmMain* Form, const String& Server, const String& Port, const String& Password);
    static void CancelDnsConnect();

private:
    static String GetAbsolutePath(const String& Path);
    static bool HasUnsafeCommandLineCharacters(const String& Value);
};
