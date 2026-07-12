#pragma once

#include "win32_uia_navigation_internal.hpp"

#include <atomic>

namespace cgpui {

class Win32UiaProvider final
    : public IRawElementProviderSimple,
      public IRawElementProviderFragment,
      public IRawElementProviderFragmentRoot {
 public:
  Win32UiaProvider(
      Win32UiaProviderTreeHandle tree,
      Win32UiaProviderNode node,
      bool is_root);
  ~Win32UiaProvider();

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;

  HRESULT STDMETHODCALLTYPE get_ProviderOptions(
      ProviderOptions* options) override;
  HRESULT STDMETHODCALLTYPE GetPatternProvider(
      PATTERNID pattern,
      IUnknown** provider) override;
  HRESULT STDMETHODCALLTYPE GetPropertyValue(
      PROPERTYID property,
      VARIANT* value) override;
  HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(
      IRawElementProviderSimple** provider) override;

  HRESULT STDMETHODCALLTYPE Navigate(
      NavigateDirection direction,
      IRawElementProviderFragment** result) override;
  HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** result) override;
  HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* result) override;
  HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(
      SAFEARRAY** result) override;
  HRESULT STDMETHODCALLTYPE SetFocus() override;
  HRESULT STDMETHODCALLTYPE get_FragmentRoot(
      IRawElementProviderFragmentRoot** result) override;

  HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(
      double x,
      double y,
      IRawElementProviderFragment** result) override;
  HRESULT STDMETHODCALLTYPE GetFocus(
      IRawElementProviderFragment** result) override;

 private:
  std::atomic<ULONG> reference_count_{1};
  Win32UiaProviderTreeHandle tree_;
  Win32UiaProviderNode node_;
  bool is_root_ = false;
};

} // namespace cgpui
