/*
* findsort.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#include <cstddef>
#include <stdexcept>
#include <cstring>

bool SortArray(void* customArray, int lowIndex, int itemSize, int L, int R, System::Classes::TListSortCompare compareItems);

bool FindInArray(void* sortedArray, int lowIndex, int itemSize, int itemCount, void* item, System::Classes::TListSortCompare compareItems, int& index);
