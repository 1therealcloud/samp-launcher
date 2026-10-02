/*
* CSettings.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class CSettings
{
public:
    static void Load(String& PlayerName);
    static void SavePlayerName(const String& PlayerName);

    static String GetGtaExecutable();
    static void SetGtaExecutable(const String& Path, bool Persist = true);

    static bool ReadBool(const String& Name, bool DefaultValue);
    static String ReadString(const String& Name, const String& DefaultValue = "");
    static void WriteBool(const String& Name, bool Value);
    static void WriteString(const String& Name, const String& Value);

    static String GetUserFilesPath();
    static String GetUserDataFileName();
    static bool EnsureUserFilesFolder();
};
