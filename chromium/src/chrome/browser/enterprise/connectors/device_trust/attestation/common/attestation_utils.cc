// Copyright (c) 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/enterprise/connectors/device_trust/attestation/common/attestation_utils.h"

#include "base/base64.h"
#include "base/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"

namespace enterprise_connectors {

std::string JsonChallengeToProtobufChallenge(const std::string& challenge) {
  absl::optional<base::Value> data = base::JSONReader::Read(
      challenge, base::JSONParserOptions::JSON_ALLOW_TRAILING_COMMAS);

  // If json is malformed or it doesn't include the needed field return
  // an empty string.
  if (!data || !data.value().FindPath("challenge"))
    return std::string();

  std::string serialized_signed_challenge;
  if (!base::Base64Decode(data.value().FindPath("challenge")->GetString(),
                          &serialized_signed_challenge)) {
    LOG(ERROR) << "Error during decoding base64 challenge.";
    return std::string();
  }

  return serialized_signed_challenge;
}

std::string ProtobufChallengeToJsonChallenge(
    const std::string& challenge_response) {
  base::Value signed_data(base::Value::Type::DICTIONARY);

  std::string encoded;
  base::Base64Encode(challenge_response, &encoded);

  base::Value dict(base::Value::Type::DICTIONARY);
  dict.SetKey("challengeResponse", base::Value(encoded));

  std::string json;
  base::JSONWriter::Write(dict, &json);
  return json;
}

}  // namespace enterprise_connectors
