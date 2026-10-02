/*
* CFavorites.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmMain;

class CFavorites
{
public:
    static void Import(TfmMain* Form, const String& FileName, bool AddToFavorites);
    static bool Export(const String& FileName, bool ExportPasswords);
    static void SaveNow();

    static bool IsChanged();
    static void SetChanged(bool Changed);
};
