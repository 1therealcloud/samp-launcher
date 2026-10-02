/*
* CServerQuery.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmMain;

class CServerQuery
{
public:
    static bool Initialize(HWND NotifyWindow);
    static void Shutdown();

    static bool IsReady();
    static void Query(TfmMain* Form, const String& Server, bool Ping, bool Info, bool Players, bool Rules);
    static void Parse(TfmMain* Form, const String& SourceIP, WORD SourcePort, char* Buffer, int DataLength);

    static void HandleRecv(TfmMain* Form, TMessage& Message);
    static void HandleBatchReady(TfmMain* Form, TMessage& Message);

    static void Enqueue(const String& Query);
    static void ClearQueue();
    static int PendingCount();
    static void ProcessQueue(TfmMain* Form, int MaxPerTick = 24);
    static void ClearNetwork();
};
