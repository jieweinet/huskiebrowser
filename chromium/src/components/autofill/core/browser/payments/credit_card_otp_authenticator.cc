// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/payments/credit_card_otp_authenticator.h"

#include "components/autofill/core/browser/autofill_metrics.h"
#include "components/autofill/core/common/autofill_tick_clock.h"

namespace autofill {

CreditCardOtpAuthenticator::OtpAuthenticationResponse::
    OtpAuthenticationResponse() = default;
CreditCardOtpAuthenticator::OtpAuthenticationResponse::
    ~OtpAuthenticationResponse() = default;

CreditCardOtpAuthenticator::CreditCardOtpAuthenticator(AutofillClient* client)
    : autofill_client_(client), payments_client_(client->GetPaymentsClient()) {}

CreditCardOtpAuthenticator::~CreditCardOtpAuthenticator() = default;

void CreditCardOtpAuthenticator::OnChallengeOptionSelected(
    const CreditCard* card,
    const CardUnmaskChallengeOption& selected_challenge_option,
    base::WeakPtr<Requester> requester,
    const std::string& context_token,
    int64_t billing_customer_number) {
  if (!card) {
    requester->OnOtpAuthenticationComplete(
        OtpAuthenticationResponse().with_did_succeed(false));
    Reset();
    return;
  }
  // Currently only virtual cards are supported for OTP authentication.
  // If non-virtual cards are allowed for OTP unmasking in the future,
  // |OnDidSelectChallengeOption()| and |OnDidGetRealPan()| should allow for a
  // generic error dialog.
  DCHECK_EQ(card->record_type(), CreditCard::VIRTUAL_CARD);
  DCHECK_EQ(selected_challenge_option.type,
            CardUnmaskChallengeOptionType::kSmsOtp);
  DCHECK(!context_token.empty());
  // Store info for this session. These info will be shared for multiple
  // payments requests. Only |context_token_| will be changed during this
  // session.
  card_ = card;
  selected_challenge_option_ = selected_challenge_option;
  requester_ = requester;
  context_token_ = context_token;
  billing_customer_number_ = billing_customer_number;

  // Asynchronously prepare payments_client. This is only needed once per
  // session.
  DCHECK(payments_client_);
  payments_client_->Prepare();

  AutofillMetrics::LogOtpAuthAttempt();

  // Send user selected challenge option to server.
  SendSelectChallengeOptionRequest();
}

void CreditCardOtpAuthenticator::SendSelectChallengeOptionRequest() {
  // Prepare SelectChallengeOption request.
  select_challenge_option_request_ = std::make_unique<
      payments::PaymentsClient::SelectChallengeOptionRequestDetails>();
  select_challenge_option_request_->selected_challenge_option =
      selected_challenge_option_;
  select_challenge_option_request_->billing_customer_number =
      billing_customer_number_;
  select_challenge_option_request_->context_token = context_token_;

  select_challenge_option_request_timestamp_ = AutofillTickClock::NowTicks();

  // Send SelectChallengeOption request to server, the callback is
  // |OnDidSelectChallengeOption|.
  payments_client_->SelectChallengeOption(
      *select_challenge_option_request_,
      base::BindOnce(&CreditCardOtpAuthenticator::OnDidSelectChallengeOption,
                     weak_ptr_factory_.GetWeakPtr()));
}

void CreditCardOtpAuthenticator::OnDidSelectChallengeOption(
    AutofillClient::PaymentsRpcResult result,
    const std::string& context_token) {
  if (select_challenge_option_request_timestamp_.has_value()) {
    AutofillMetrics::LogOtpAuthSelectChallengeOptionRequestLatency(
        AutofillTickClock::NowTicks() -
        *select_challenge_option_request_timestamp_);
  }

  if (result == AutofillClient::PaymentsRpcResult::kSuccess) {
    DCHECK(!context_token.empty());
    // Update the |context_token_| with the new one.
    context_token_ = context_token;
    // Display the OTP dialog.
    ShowOtpDialog();
    return;
  }

  // End Session.
  requester_->OnOtpAuthenticationComplete(
      OtpAuthenticationResponse().with_did_succeed(false));
  Reset();

  // Show the virtual card permanent error dialog if server explicitly returned
  // vcn permanent error, show temporary error dialog for the rest failure cases
  // since currently only virtual card is supported.
  autofill_client_->ShowVirtualCardErrorDialog(
      /*is_permanent_error=*/result ==
      AutofillClient::PaymentsRpcResult::kVcnRetrievalPermanentFailure);

  if (result ==
          AutofillClient::PaymentsRpcResult::kVcnRetrievalPermanentFailure ||
      result ==
          AutofillClient::PaymentsRpcResult::kVcnRetrievalTryAgainFailure) {
    AutofillMetrics::LogOtpAuthResult(
        AutofillMetrics::OtpAuthEvent::
            kSelectedChallengeOptionVirtualCardRetrievalError);
  } else {
    AutofillMetrics::LogOtpAuthResult(
        AutofillMetrics::OtpAuthEvent::kSelectedChallengeOptionGenericError);
  }
}

void CreditCardOtpAuthenticator::ShowOtpDialog() {
  // Before showing OTP dialog, let's load required risk data if it's not
  // prepared. Risk data is only required for unmask request. Not required for
  // select challenge option request.
  // TODO(crbug.com/1243475): Explore the possibility of sending one
  // LoadRiskData request per session.
  if (risk_data_.empty()) {
    autofill_client_->LoadRiskData(
        base::BindOnce(&CreditCardOtpAuthenticator::OnDidGetUnmaskRiskData,
                       weak_ptr_factory_.GetWeakPtr()));
  }

  // TODO(crbug.com/1243475): Invoke autofill_client to show otp authentication
  // dialog. Then hold request before otp value is populated.
  // Once user confirms the OTP, we wil invoke |OnUnmaskPromptAccepted(otp)|.
  // If user asks for a new OTP code, we will invoke
  // |SendSelectChallengeOptionRequest()| again.
}

void CreditCardOtpAuthenticator::OnUnmaskPromptAccepted(
    const std::u16string& otp) {
  otp_ = otp;

  unmask_request_ =
      std::make_unique<payments::PaymentsClient::UnmaskRequestDetails>();
  unmask_request_->card = *card_;
  unmask_request_->billing_customer_number = billing_customer_number_;
  unmask_request_->context_token = context_token_;
  unmask_request_->otp = otp_;

  if (card_->record_type() == CreditCard::VIRTUAL_CARD) {
    absl::optional<GURL> last_committed_url_origin;
    if (autofill_client_->GetLastCommittedURL().is_valid()) {
      last_committed_url_origin =
          autofill_client_->GetLastCommittedURL().DeprecatedGetOriginAsURL();
    }
    unmask_request_->last_committed_url_origin = last_committed_url_origin;
  }

  // Populating risk data and showing OTP dialog may occur asynchronously.
  // If |risk_data_| has already been loaded, send the unmask card request.
  // Otherwise, continue to wait and let OnDidGetUnmaskRiskData handle it.
  if (!risk_data_.empty()) {
    SendUnmaskCardRequest();
  }
}

void CreditCardOtpAuthenticator::OnUnmaskPromptClosed(
    bool dismiss_by_cancellation) {
  // This function will be invoked when the prompt closes, no matter if it is
  // due to success or cancellation by users. If |dismiss_by_cancellation| is
  // false we should return early in this case.
  if (!dismiss_by_cancellation)
    return;

  requester_->OnOtpAuthenticationComplete(
      OtpAuthenticationResponse().with_did_succeed(false));
  Reset();
  AutofillMetrics::LogOtpAuthResult(
      AutofillMetrics::OtpAuthEvent::kFlowCancelled);
}

void CreditCardOtpAuthenticator::OnDidGetUnmaskRiskData(
    const std::string& risk_data) {
  risk_data_ = risk_data;
  // Populating risk data and showing OTP dialog may occur asynchronously.
  // If the dialog has already been accepted (otp is provided), send the unmask
  // card request. Otherwise, continue to wait for the user to accept the OTP
  // dialog.
  if (!otp_.empty()) {
    SendUnmaskCardRequest();
  }
}

void CreditCardOtpAuthenticator::SendUnmaskCardRequest() {
  unmask_request_->risk_data = risk_data_;
  unmask_card_request_timestamp_ = AutofillTickClock::NowTicks();
  payments_client_->UnmaskCard(
      *unmask_request_,
      base::BindOnce(&CreditCardOtpAuthenticator::OnDidGetRealPan,
                     weak_ptr_factory_.GetWeakPtr()));
}

void CreditCardOtpAuthenticator::OnDidGetRealPan(
    AutofillClient::PaymentsRpcResult result,
    payments::PaymentsClient::UnmaskResponseDetails& response_details) {
  if (unmask_card_request_timestamp_.has_value()) {
    AutofillMetrics::LogOtpAuthUnmaskCardRequestLatency(
        AutofillTickClock::NowTicks() - *unmask_card_request_timestamp_);
  }

  if (result == AutofillClient::PaymentsRpcResult::kSuccess) {
    if (response_details.card_type !=
        AutofillClient::PaymentsRpcCardType::kVirtualCard) {
      // Currently we offer OTP authentication only for virtual cards.
      NOTREACHED();
      requester_->OnOtpAuthenticationComplete(
          OtpAuthenticationResponse().with_did_succeed(false));
      Reset();
      return;
    }
    // If |flow_status| is present, this intermediate status allows the user to
    // stay in the current session to finish the unmasking with certain user
    // actions rather than ending the flow.
    if (!response_details.flow_status.empty()) {
      DCHECK(!response_details.context_token.empty());
      // Update the |context_token_| with the new one.
      context_token_ = response_details.context_token;

      // TODO(crbug.com/1243475): Invoke autofill_client to update the OTP
      // dialog with the flow status, e.g. OTP mismatch or expired.
      if (response_details.flow_status.find("INCORRECT_OTP") !=
          std::string::npos) {
        AutofillMetrics::LogOtpAuthRetriableError(
            AutofillMetrics::OtpAuthEvent::kOtpMismatch);
      } else {
        DCHECK(response_details.flow_status.find("EXPIRED_OTP") !=
               std::string::npos);
        AutofillMetrics::LogOtpAuthRetriableError(
            AutofillMetrics::OtpAuthEvent::kOtpExpired);
      }
      return;
    }

    // The following prerequisites should be ensured in the PaymentsClient.
    DCHECK(!response_details.real_pan.empty());
    DCHECK(!response_details.dcvv.empty());
    DCHECK(!response_details.expiration_month.empty());
    DCHECK(!response_details.expiration_year.empty());

    unmask_request_->card.SetNumber(
        base::UTF8ToUTF16(response_details.real_pan));
    unmask_request_->card.set_record_type(CreditCard::VIRTUAL_CARD);
    unmask_request_->card.SetExpirationMonthFromString(
        base::UTF8ToUTF16(response_details.expiration_month),
        /*app_locale=*/std::string());
    unmask_request_->card.SetExpirationYearFromString(
        base::UTF8ToUTF16(response_details.expiration_year));

    auto response = OtpAuthenticationResponse().with_did_succeed(true);
    response.card = &(unmask_request_->card);
    response.cvc = base::UTF8ToUTF16(response_details.dcvv);
    requester_->OnOtpAuthenticationComplete(response);
    Reset();

    AutofillMetrics::LogOtpAuthResult(AutofillMetrics::OtpAuthEvent::kSuccess);
    return;
  }

  // Show the virtual card permanent error dialog if server explicitly returned
  // vcn permanent error, show temporary error dialog for the rest failure cases
  // since currently only virtual card is supported.
  autofill_client_->ShowVirtualCardErrorDialog(
      /*is_permanent_error=*/result ==
      AutofillClient::PaymentsRpcResult::kVcnRetrievalPermanentFailure);
  requester_->OnOtpAuthenticationComplete(
      OtpAuthenticationResponse().with_did_succeed(false));
  Reset();

  if (result ==
          AutofillClient::PaymentsRpcResult::kVcnRetrievalPermanentFailure ||
      result ==
          AutofillClient::PaymentsRpcResult::kVcnRetrievalTryAgainFailure) {
    AutofillMetrics::LogOtpAuthResult(
        AutofillMetrics::OtpAuthEvent::kUnmaskCardVirtualCardRetrievalError);
  } else {
    AutofillMetrics::LogOtpAuthResult(
        AutofillMetrics::OtpAuthEvent::kUnmaskCardAuthError);
  }
}

void CreditCardOtpAuthenticator::Reset() {
  weak_ptr_factory_.InvalidateWeakPtrs();
  payments_client_->CancelRequest();
  card_ = nullptr;
  selected_challenge_option_ = CardUnmaskChallengeOption();
  otp_ = std::u16string();
  context_token_ = std::string();
  risk_data_ = std::string();
  billing_customer_number_ = 0;
  select_challenge_option_request_.reset();
  unmask_request_.reset();
  select_challenge_option_request_timestamp_ = absl::nullopt;
  unmask_card_request_timestamp_ = absl::nullopt;
}

}  // namespace autofill
