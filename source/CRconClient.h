/*
* CRconClient.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

enum class TRconStartResult
{
    Started,
    AlreadyActive,
    Failed
};

class CRconClient
{
public:
    static TRconStartResult Start(const AnsiString& Host, int Port, const AnsiString& Password);
    static bool IsActive();
};
