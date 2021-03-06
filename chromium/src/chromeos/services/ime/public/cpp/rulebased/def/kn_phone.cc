// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/services/ime/public/cpp/rulebased/def/kn_phone.h"

#include "base/cxx17_backports.h"

namespace kn_phone {

const char* kId = "kn_phone";
bool kIs102 = false;
const char* kTransforms[] = {u8"0",
                             u8"\u0ce6",
                             u8"1",
                             u8"\u0ce7",
                             u8"2",
                             u8"\u0ce8",
                             u8"3",
                             u8"\u0ce9",
                             u8"4",
                             u8"\u0cea",
                             u8"5",
                             u8"\u0ceb",
                             u8"6",
                             u8"\u0cec",
                             u8"7",
                             u8"\u0ced",
                             u8"8",
                             u8"\u0cee",
                             u8"9",
                             u8"\u0cef",
                             u8"([\u0c95-\u0cb9\u0cde])\u001d?u",
                             u8"\\1\u0ccc",
                             u8"\u0cca\u001d?o",
                             u8"\u0ccb",
                             u8"\u0ccd\u001d?O",
                             u8"\u0ccb",
                             u8"\u0ccd\u001d?o",
                             u8"\u0cca",
                             u8"([\u0c95-\u0cb9\u0cde])\u001d?i",
                             u8"\\1\u0cc8",
                             u8"\u0cc6\u001d?e",
                             u8"\u0cc7",
                             u8"\u0ccd\u001d?E",
                             u8"\u0cc7",
                             u8"\u0ccd\u001d?e",
                             u8"\u0cc6",
                             u8"\u0ccd\u0cb3\u0ccd\u0cb3\u0ccd\u001d?I",
                             u8"\u0ce3",
                             u8"\u0ccd\u0cb3\u0ccd\u001d?\\^I",
                             u8"\u0ce3",
                             u8"\u0ccd\u0cb3\u0ccd\u0cb3\u0ccd\u001d?i",
                             u8"\u0ce2",
                             u8"\u0ccd\u0cb3\u0ccd\u001d?\\^i",
                             u8"\u0ce2",
                             u8"\u0cc3\u001d?I",
                             u8"\u0cc4",
                             u8"\u0cc3\u001d?\\^I",
                             u8"\u0cc4",
                             u8"\u0cc3\u001d?R",
                             u8"\u0cc4",
                             u8"\u0cc3\u001d?\\^i",
                             u8"\u0cc3",
                             u8"\u0cc4\u001d?i",
                             u8"\u0cc3",
                             u8"\u0ccd\u001d?R",
                             u8"\u0cc3",
                             u8"\u0cc1\u001d?u",
                             u8"\u0cc2",
                             u8"\u0ccd\u001d?U",
                             u8"\u0cc2",
                             u8"\u0ccd\u001d?u",
                             u8"\u0cc1",
                             u8"\u0cbf\u001d?i",
                             u8"\u0cc0",
                             u8"\u0ccd\u001d?I",
                             u8"\u0cc0",
                             u8"\u0ccd\u001d?i",
                             u8"\u0cbf",
                             u8"([\u0c95-\u0cb9\u0cde])\u001d?a",
                             u8"\\1\u0cbe",
                             u8"\u0ccd\u001d?A",
                             u8"\u0cbe",
                             u8"\u0ccd\u001d?a",
                             u8"",
                             u8"\\.a",
                             u8"\u0cbd",
                             u8"\\.m",
                             u8"\u0c82",
                             u8"\\.z",
                             u8"\u0cbc",
                             u8"\\.N",
                             u8"\u0901",
                             u8"\u0ccd\u001d?\\.h",
                             u8"\u0ccd\u200c",
                             u8"\\.h",
                             u8"\u0ccd\u200c",
                             u8"M",
                             u8"\u0c82",
                             u8"H",
                             u8"\u0c83",
                             u8"\u0c95\u0ccd\u001d?H",
                             u8"\u0cf1",
                             u8"\u0caa\u0ccd\u001d?H",
                             u8"\u0cf2",
                             u8"\u0c85\u001d?u",
                             u8"\u0c94",
                             u8"\u0c92\u001d?o",
                             u8"\u0c93",
                             u8"O",
                             u8"\u0c93",
                             u8"o",
                             u8"\u0c92",
                             u8"\u0c85\u001d?i",
                             u8"\u0c90",
                             u8"\u0c8e\u001d?e",
                             u8"\u0c8f",
                             u8"E",
                             u8"\u0c8f",
                             u8"e",
                             u8"\u0c8e",
                             u8"\u0cb3\u0ccd\u0cb3\u0ccd\u001d?I",
                             u8"\u0ce1",
                             u8"\u0cb3\u0ccd\u001d?\\^I",
                             u8"\u0ce1",
                             u8"\u0cb3\u0ccd\u0cb3\u0ccd\u001d?i",
                             u8"\u0c8c",
                             u8"\u0cb3\u0ccd\u001d?^i",
                             u8"\u0c8c",
                             u8"\u0ce0\u001d?I",
                             u8"\u0ce0",
                             u8"\u0ce0\u001d?^I",
                             u8"\u0ce0",
                             u8"\u0c8b\u001d?R",
                             u8"\u0ce0",
                             u8"R",
                             u8"\u0c8b",
                             u8"\u0c89\u001d?u",
                             u8"\u0c8a",
                             u8"U",
                             u8"\u0c8a",
                             u8"u",
                             u8"\u0c89",
                             u8"\u0c87\u001d?i",
                             u8"\u0c88",
                             u8"I",
                             u8"\u0c88",
                             u8"i",
                             u8"\u0c87",
                             u8"A",
                             u8"\u0c86",
                             u8"\u0c85\u001d?a",
                             u8"\u0c86",
                             u8"a",
                             u8"\u0c85",
                             u8"\u0c95\u0ccd\u0cb7\u0ccd\u001d?h",
                             u8"\u0c95\u0ccd\u0cb7\u0ccd",
                             u8"\u0c97\u0ccd\u001d?Y",
                             u8"\u0c9c\u0ccd\u0c9e\u0ccd",
                             u8"\u0c9c\u0ccd\u001d?n",
                             u8"\u0c9c\u0ccd\u0c9e\u0ccd",
                             u8"\u0c95\u0ccd\u001d?S",
                             u8"\u0c95\u0ccd\u0cb7\u0ccd",
                             u8"\u0c95\u0ccd\u0cb8\u0ccd\u001d?h",
                             u8"\u0c95\u0ccd\u0cb7\u0ccd",
                             u8"x",
                             u8"\u0c95\u0ccd\u0cb7\u0ccd",
                             u8"h",
                             u8"\u0cb9\u0ccd",
                             u8"\u0cb7\u0ccd\u001d?h",
                             u8"\u0cb7\u0ccd",
                             u8"S",
                             u8"\u0cb7\u0ccd",
                             u8"z",
                             u8"\u0cb6\u0ccd",
                             u8"\u0cb8\u0ccd\u001d?h",
                             u8"\u0cb6\u0ccd",
                             u8"s",
                             u8"\u0cb8\u0ccd",
                             u8"v",
                             u8"\u0cb5\u0ccd",
                             u8"w",
                             u8"\u0cb5\u0ccd",
                             u8"L",
                             u8"\u0cb3\u0ccd",
                             u8"\\.L",
                             u8"\u0cde\u0ccd",
                             u8"l",
                             u8"\u0cb2\u0ccd",
                             u8"r",
                             u8"\u0cb0\u0ccd",
                             u8"\\.r",
                             u8"\u0cb1\u0ccd",
                             u8"y",
                             u8"\u0caf\u0ccd",
                             u8"~N",
                             u8"\u0c99\u0ccd",
                             u8"\u0c97\u0ccd\u001d?h",
                             u8"\u0c98\u0ccd",
                             u8"G",
                             u8"\u0c98\u0ccd",
                             u8"\\.g",
                             u8"\u0c97\u0cbc\u0ccd",
                             u8"g",
                             u8"\u0c97\u0ccd",
                             u8"\\.K",
                             u8"\u0c96\u0cbc\u0ccd",
                             u8"K",
                             u8"\u0c96\u0ccd",
                             u8"\u0c95\u0ccd\u001d?h",
                             u8"\u0c96\u0ccd",
                             u8"q",
                             u8"\u0c95\u0cbc\u0ccd",
                             u8"k",
                             u8"\u0c95\u0ccd",
                             u8"~n",
                             u8"\u0c9e\u0ccd",
                             u8"\u0c9c\u0ccd\u001d?h",
                             u8"\u0c9d\u0ccd",
                             u8"J",
                             u8"\u0c9d\u0ccd",
                             u8"\\.j",
                             u8"\u0c9c\u0cbc\u0ccd",
                             u8"j",
                             u8"\u0c9c\u0ccd",
                             u8"\u0c9a\u0ccd\u001d?h",
                             u8"\u0c9b\u0ccd",
                             u8"Ch",
                             u8"\u0c9b\u0ccd",
                             u8"ch",
                             u8"\u0c9a\u0ccd",
                             u8"C",
                             u8"\u0c9b\u0ccd",
                             u8"c",
                             u8"\u0c9a\u0ccd",
                             u8"N",
                             u8"\u0ca3\u0ccd",
                             u8"\u0ca1\u0cbc\u0ccd\u001d?h",
                             u8"\u0ca2\u0cbc\u0ccd",
                             u8"\\.D",
                             u8"\u0ca1\u0cbc\u0ccd",
                             u8"\u0ca1\u0ccd\u001d?h",
                             u8"\u0ca2\u0ccd",
                             u8"D",
                             u8"\u0ca1\u0ccd",
                             u8"\u0c9f\u0ccd\u001d?h",
                             u8"\u0ca0\u0ccd",
                             u8"T",
                             u8"\u0c9f\u0ccd",
                             u8"n",
                             u8"\u0ca8\u0ccd",
                             u8"\u0ca6\u0ccd\u001d?h",
                             u8"\u0ca7\u0ccd",
                             u8"d",
                             u8"\u0ca6\u0ccd",
                             u8"\u0ca4\u0ccd\u001d?h",
                             u8"\u0ca5\u0ccd",
                             u8"t",
                             u8"\u0ca4\u0ccd",
                             u8"m",
                             u8"\u0cae\u0ccd",
                             u8"\u0cac\u0ccd\u001d?h",
                             u8"\u0cad\u0ccd",
                             u8"B",
                             u8"\u0cad\u0ccd",
                             u8"b",
                             u8"\u0cac\u0ccd",
                             u8"f",
                             u8"\u0cab\u0cbc\u0ccd",
                             u8"\u0caa\u0ccd\u001d?h",
                             u8"\u0cab\u0ccd",
                             u8"P",
                             u8"\u0cab\u0ccd",
                             u8"p",
                             u8"\u0caa\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?G",
                             u8"\u0c82\u0c98\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?g",
                             u8"\u0c82\u0c97\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?K",
                             u8"\u0c82\u0c96\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?k",
                             u8"\u0c82\u0c95\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?J",
                             u8"\u0c82\u0c9d\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?j",
                             u8"\u0c82\u0c9c\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?Ch",
                             u8"\u0c82\u0c9b\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?ch",
                             u8"\u0c82\u0c9a\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?C",
                             u8"\u0c82\u0c9b\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?c",
                             u8"\u0c82\u0c9a\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?D",
                             u8"\u0c82\u0ca1\u0ccd",
                             u8"\u0ca8\u0ccd\u001d?T",
                             u8"\u0c82\u0c9f\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?g",
                             u8"\u0c99\u0ccd\u0c97\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?k",
                             u8"\u0c99\u0ccd\u0c95\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?j",
                             u8"\u0c9e\u0ccd\u0c9c\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?Ch",
                             u8"\u0c9e\u0ccd\u0c9b\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?ch",
                             u8"\u0c9e\u0ccd\u0c9a\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?C",
                             u8"\u0c9e\u0ccd\u0c9b\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?c",
                             u8"\u0c9e\u0ccd\u0c9a\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?D",
                             u8"\u0ca3\u0ccd\u0ca1\u0ccd",
                             u8"\u0ca8\u0ccd\u0ca8\u0ccd\u001d?T",
                             u8"\u0ca3\u0ccd\u0c9f\u0ccd",
                             u8"\\|",
                             u8"\u0964",
                             u8"\u0964\u001d?\\|",
                             u8"\u0965"};
const unsigned int kTransformsLen = base::size(kTransforms);
const char* kHistoryPrune = "C|c";

}  // namespace kn_phone
