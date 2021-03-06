// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/speech/tts_controller_delegate_impl.h"

#include <stddef.h>

#include <string>

#include "base/json/json_reader.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/tts_controller.h"
#include "third_party/blink/public/mojom/speech/speech_synthesis.mojom.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

absl::optional<content::TtsControllerDelegate::PreferredVoiceId>
PreferredVoiceIdFromString(const base::DictionaryValue* pref,
                           const std::string& pref_key) {
  std::string voice_id;
  pref->GetString(l10n_util::GetLanguage(pref_key), &voice_id);
  if (voice_id.empty())
    return absl::nullopt;

  std::unique_ptr<base::DictionaryValue> json =
      base::DictionaryValue::From(base::JSONReader::ReadDeprecated(voice_id));
  std::string name;
  std::string id;
  json->GetString("name", &name);
  json->GetString("extension", &id);
  return absl::optional<content::TtsControllerDelegate::PreferredVoiceId>(
      {name, id});
}

}  // namespace

//
// TtsControllerDelegateImpl
//

// static
TtsControllerDelegateImpl* TtsControllerDelegateImpl::GetInstance() {
  return base::Singleton<TtsControllerDelegateImpl>::get();
}

TtsControllerDelegateImpl::TtsControllerDelegateImpl() = default;

TtsControllerDelegateImpl::~TtsControllerDelegateImpl() = default;

std::unique_ptr<content::TtsControllerDelegate::PreferredVoiceIds>
TtsControllerDelegateImpl::GetPreferredVoiceIdsForUtterance(
    content::TtsUtterance* utterance) {
  const base::DictionaryValue* lang_to_voice_pref =
      GetLangToVoicePref(utterance);
  if (!lang_to_voice_pref)
    return nullptr;

  std::unique_ptr<PreferredVoiceIds> preferred_ids =
      std::make_unique<PreferredVoiceIds>();

  if (!utterance->GetLang().empty()) {
    preferred_ids->lang_voice_id = PreferredVoiceIdFromString(
        lang_to_voice_pref, l10n_util::GetLanguage(utterance->GetLang()));
  }

  const std::string app_lang = g_browser_process->GetApplicationLocale();
  preferred_ids->locale_voice_id = PreferredVoiceIdFromString(
      lang_to_voice_pref, l10n_util::GetLanguage(app_lang));

  preferred_ids->any_locale_voice_id =
      PreferredVoiceIdFromString(lang_to_voice_pref, "noLanguageCode");
  return preferred_ids;
}

void TtsControllerDelegateImpl::UpdateUtteranceDefaultsFromPrefs(
    content::TtsUtterance* utterance,
    double* rate,
    double* pitch,
    double* volume) {
  // Update pitch, rate and volume from user prefs if not set explicitly
  // on this utterance.
  const PrefService* prefs = GetPrefService(utterance);
  if (*rate == blink::mojom::kSpeechSynthesisDoublePrefNotSet) {
    *rate = prefs ? prefs->GetDouble(prefs::kTextToSpeechRate)
                  : blink::mojom::kSpeechSynthesisDefaultRate;
  }
  if (*pitch == blink::mojom::kSpeechSynthesisDoublePrefNotSet) {
    *pitch = prefs ? prefs->GetDouble(prefs::kTextToSpeechPitch)
                   : blink::mojom::kSpeechSynthesisDefaultPitch;
  }
  if (*volume == blink::mojom::kSpeechSynthesisDoublePrefNotSet) {
    *volume = prefs ? prefs->GetDouble(prefs::kTextToSpeechVolume)
                    : blink::mojom::kSpeechSynthesisDefaultVolume;
  }
}

const PrefService* TtsControllerDelegateImpl::GetPrefService(
    content::TtsUtterance* utterance) {
  // The utterance->GetBrowserContext() is null in tests.
  if (!utterance->GetBrowserContext())
    return nullptr;

  const Profile* profile =
      Profile::FromBrowserContext(utterance->GetBrowserContext());
  return profile ? profile->GetPrefs() : nullptr;
}

const base::DictionaryValue* TtsControllerDelegateImpl::GetLangToVoicePref(
    content::TtsUtterance* utterance) {
  const PrefService* prefs = GetPrefService(utterance);
  return prefs == nullptr
             ? nullptr
             : prefs->GetDictionary(prefs::kTextToSpeechLangToVoiceName);
}
