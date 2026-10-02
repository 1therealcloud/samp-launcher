/*
* CDnsResolver.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmMain;

class CDnsResolver
{
public:
    static String GetCached(const String& HostName);
    static bool HasCached(const String& HostName);
    static void Cache(const String& HostName, const String& Address);
    static void Clear();

    static bool QueueServerQuery(HWND Window, const AnsiString& Host, const String& Server, bool Ping, bool Info,
                                 bool Players, bool Rules);
    static bool QueueConnect(HWND Window, const AnsiString& Host, const String& Server, const String& Port,
                             const String& Password);

    static void HandleMessage(TfmMain* Form, TMessage& Message);
    static void Shutdown();
};
