/*
* CMasterServer.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmMain;

class CMasterServer
{
public:
    static void Refresh(TfmMain* Form);
    static void Apply(TfmMain* Form, TStringList* ServerList);
    static void Tick(TfmMain* Form);
    static void CheckQueryBatchComplete(TfmMain* Form);
    static void Cancel(TfmMain* Form);
    static void Shutdown(TfmMain* Form);

    static bool IsQueryBatchActive();
    static void OnServerQueryProgress(TfmMain* Form, int ServerIndex);

private:
    static bool Download(TStringList* ServerList, String& ErrorMessage);
    static void FinishQueryBatch(TfmMain* Form);
    static void CloseProgress();
};
