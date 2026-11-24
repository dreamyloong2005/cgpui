#include "dx12_dred_scalar_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  DredSetupScript disabled;
  disabled.query = {0, true};
  const auto off = dx12_request_dred_setup(false, disabled);
  if (!std::holds_alternative<Dx12DredNotRequested>(off) || disabled.queries || disabled.setters)
    return dred_fail(1, "not requested must not acquire or invoke settings");
  DredSetupScript failed;
  failed.query = {dred_no_interface, false};
  const auto unavailable = dx12_request_dred_setup(true, failed);
  const auto* failure = std::get_if<Dx12DredSetupUnavailable>(&unavailable);
  if (!failure || failure->query.hresult != dred_no_interface || failure->query.received ||
      failed.queries != 1 || failed.setters)
    return dred_fail(2, "actual settings failure lost bits or invoked void setter");
  DredSetupScript null_success;
  null_success.query = {0, false};
  const auto absent = dx12_request_dred_setup(true, null_success);
  const auto* missing = std::get_if<Dx12DredSetupUnavailable>(&absent);
  if (!missing || missing->query.hresult != 0 || missing->query.received ||
      null_success.queries != 1 || null_success.setters)
    return dred_fail(3, "success/null must retain S_OK without invoking absent interface");
  DredSetupScript available;
  available.query = {0, true};
  const auto requested = dx12_request_dred_setup(true, available);
  const auto* returned = std::get_if<Dx12DredSetupRequestReturned>(&requested);
  if (!returned || returned->query.hresult != 0 || !returned->query.received ||
      available.queries != 1 || available.setters != 1)
    return dred_fail(4, "void request return must be separate from actual acquisition result");
  DredSetupScript anomalous;
  anomalous.query = {dred_no_interface, true};
  const auto contradictory = dx12_request_dred_setup(true, anomalous);
  const auto* observed = std::get_if<Dx12DredSetupUnavailable>(&contradictory);
  if (!observed || !observed->query.received || observed->query.hresult != dred_no_interface || anomalous.setters)
    return dred_fail(5, "failed acquisition cannot authorize setter even with an anomalous pointer");
  return 0;
}
