/*
* findsort.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "findsort.h"

struct TStackFrame {
    int Ls;
    int Rs;
};

bool SortArray(void* customArray, int lowIndex, int itemSize, int L, int R, System::Classes::TListSortCompare compareItems) {
    if (itemSize < 1 || R < L || compareItems == nullptr) {
        return false;
    }

    if (R == L) {
        return true;
    }

    unsigned char* P = new unsigned char[itemSize];
    unsigned char* T = new unsigned char[itemSize];

    try {
        unsigned char* base = static_cast<unsigned char*>(customArray) - (lowIndex * itemSize);
        
        TStackFrame stack[65];
        int level = 1;

        stack[level].Ls = L;
        stack[level].Rs = R;

        do {
            L = stack[level].Ls;
            R = stack[level].Rs;
            --level;

            do {
                int I = L;
                int J = R;

                int midIndex = L + (R - L) / 2;
                std::memcpy(P, base + (midIndex * itemSize), itemSize);

                do {
                    unsigned char* Ip = base + (I * itemSize);
                    unsigned char* Jp = base + (J * itemSize);

                    while (compareItems(Ip, P) < 0) {
                        ++I;
                        Ip += itemSize;
                    }

                    while (compareItems(Jp, P) > 0) {
                        --J;
                        Jp -= itemSize;
                    }

                    if (I <= J) {
                        std::memcpy(T, Ip, itemSize);
                        std::memcpy(Ip, Jp, itemSize);
                        std::memcpy(Jp, T, itemSize);

                        ++I;
                        --J;
                    }
                } while (I <= J);

                if (I < R) {
                    ++level;
                    if (level > 64) {
                        throw Exception("Stack Overflow in Quick Sort");
                    }
                    stack[level].Ls = I;
                    stack[level].Rs = R;
                }

                R = J;
            } while (L < R);

        } while (level > 0);

        delete[] P;
        delete[] T;
        return true;
    }
    catch (...) {
        delete[] P;
        delete[] T;
        throw;
    }
}

bool FindInArray(void* sortedArray, int lowIndex, int itemSize, int itemCount, void* item, System::Classes::TListSortCompare compareItems, int& index) {
    bool result = false;
    int L = lowIndex;
    int H = lowIndex + itemCount - 1;

    while (L <= H) {
        int I = (L + H) / 2;
        void* It = static_cast<unsigned char*>(sortedArray) + (I - lowIndex) * itemSize;
        
        int C = compareItems(It, item);
        if (C < 0) {
            L = I + 1;
        } else {
            H = I - 1;
            if (C == 0) {
                result = true;
            }
        }
    }

    index = L;
    return result;
}
