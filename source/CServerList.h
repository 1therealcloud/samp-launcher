/*
* CServerList.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#include <string>

class TfmMain;

class CServerList
{
public:
    static bool Add(TfmMain* Form, const String& Server);
    static bool Remove(int Index);

    static std::string EndpointKey(const AnsiString& Address, int Port);
    static bool IsValidEndpoint(const String& Address, int Port);

    static void RebuildLookup();
    static int FindByPacket(const AnsiString& Address, WORD Port, WORD Tag);

    static void MarkOrderDirty();
    static bool IsOrderDirty();
    static void Sort();

    static void MarkDirty();
    static bool ConsumeDirty();

private:
    static std::string ServerKey(const AnsiString& Address, WORD Port, WORD Tag);
};
