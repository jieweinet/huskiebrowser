import { RequestMode, RequestIdTokenStatus, LogoutStatus, FederatedAuthRequest, FederatedAuthRequestReceiver } from '/gen/third_party/blink/public/mojom/webid/federated_auth_request.mojom.m.js';

function toMojoIdTokenStatus(status) {
  switch(status) {
    case "Success": return RequestIdTokenStatus.kSuccess;
    case "ApprovalDeclined": return RequestIdTokenStatus.kApprovalDeclined;
    case "ErrorTooManyRequests": return RequestIdTokenStatus.kErrorTooManyRequests;
    case "ErrorWebIdNotSupportedByProvider": return RequestIdTokenStatus.kErrorWebIdNotSupportedByProvider;
    case "ErrorFetchingWellKnown": return RequestIdTokenStatus.kErrorFetchingWellKnown;
    case "ErrorInvalidWellKnown": return RequestIdTokenStatus.kErrorInvalidWellKnown;
    case "ErrorFetchingSignin": return RequestIdTokenStatus.kErrorFetchingSignin;
    case "ErrorInvalidSigninResponse": return RequestIdTokenStatus.kErrorInvalidSigninResponse;
    case "ErrorInvalidAccountsResponse": return RequestIdTokenStatus.kErrorInvalidAccountsResponse;
    case "ErrorInvalidTokenResponse": return RequestIdTokenStatus.kErrorInvalidTokenResponse;
    case "Error": return RequestIdTokenStatus.kError;
    default: throw new Error(`Invalid status: ${status}`);
  }
}

// A mock service for responding to federated auth requests.
export class MockFederatedAuthRequest {
  constructor() {
    this.receiver_ = new FederatedAuthRequestReceiver(this);
    this.interceptor_ = new MojoInterfaceInterceptor(FederatedAuthRequest.$interfaceName);
    this.interceptor_.oninterfacerequest = e => {
        this.receiver_.$.bindHandle(e.handle);
    }
    this.interceptor_.start();
    this.idToken_ = null;
    this.status_ = RequestIdTokenStatus.kError;
    this.logoutStatus_ = LogoutStatus.kError;
  }

  // Causes the subsequent `navigator.id.get()` to resolve with the token.
  returnIdToken(token) {
    this.status_ = RequestIdTokenStatus.kSuccess;
    this.idToken_ = token;
  }

  // Causes the subsequent `navigator.id.get()` to reject with the error.
  returnError(error) {
    if (error == "Success")
      throw new Error("Success is not a valid error");
    this.status_ = toMojoIdTokenStatus(error);
    this.idToken_ = null;
  }

  // Implements
  //   RequestIdToken(url.mojom.Url provider, string id_request, RequestMode mode) => (RequestIdTokenStatus status, string? id_token);
  async requestIdToken(provider, idRequest, mode) {
    return Promise.resolve({
      status: this.status_,
      idToken: this.idToken_
    });
  }

  async logout(logout_endpoints) {
    return Promise.resolve({
      status: this.logoutStatus_
    });
  }

  async reset() {
    this.idToken_ = null;
    this.status_ = RequestIdTokenStatus.kError;
    this.logoutStatus_ = LogoutStatus.kError;
    this.receiver_.$.close();
    this.interceptor_.stop();

    // Clean up and reset mock stubs asynchronously, so that the blink side
    // closes its proxies and notifies JS sensor objects before new test is
    // started.
    await new Promise(resolve => { setTimeout(resolve, 0); });
  }
}
