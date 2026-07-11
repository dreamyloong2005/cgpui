#include "win32_ole_file_data_object_internal.hpp"

#include <string>
#include <vector>

namespace {
std::vector<std::wstring> expected_paths;
int runner_calls = 0;

HRESULT WINAPI inspect_files(
    IDataObject* data, IDropSource*, DWORD allowed, DWORD* effect) {
  ++runner_calls;
  if (data == nullptr || effect == nullptr || allowed != DROPEFFECT_MOVE) return E_FAIL;
  FORMATETC format{CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  if (data->QueryGetData(&format) != S_OK) return E_FAIL;
  IEnumFORMATETC* enumerator = nullptr;
  if (data->EnumFormatEtc(DATADIR_GET, &enumerator) != S_OK || enumerator == nullptr)
    return E_FAIL;
  FORMATETC enumerated{};
  const HRESULT next = enumerator->Next(1, &enumerated, nullptr);
  enumerator->Release();
  if (next != S_OK || enumerated.cfFormat != CF_HDROP) return E_FAIL;
  STGMEDIUM storage{};
  if (data->GetData(&format, &storage) != S_OK || storage.tymed != TYMED_HGLOBAL)
    return E_FAIL;
  const auto drop = reinterpret_cast<HDROP>(storage.hGlobal);
  const UINT count = DragQueryFileW(drop, 0xFFFFFFFFU, nullptr, 0);
  std::vector<std::wstring> paths;
  for (UINT index = 0; index < count; ++index) {
    const UINT length = DragQueryFileW(drop, index, nullptr, 0);
    std::wstring path(static_cast<std::size_t>(length) + 1U, L'\0');
    DragQueryFileW(drop, index, path.data(), length + 1U);
    path.resize(length);
    paths.push_back(std::move(path));
  }
  ReleaseStgMedium(&storage);
  if (paths != expected_paths) return E_FAIL;
  *effect = DROPEFFECT_MOVE;
  return DRAGDROP_S_DROP;
}
} // namespace

int main() {
  const std::vector<std::string> files{
      "C:\\alpha\\report.txt",
      "D:\\unicode\\\xE4\xB8\xAD\xE6\x96\x87-\xF0\x9F\x98\x80.bin",
  };
  cgpui::Win32OleFileDataObject object{files};
  void* queried = nullptr;
  if (!object.valid() || object.QueryInterface(IID_IDataObject, &queried) != S_OK ||
      queried != static_cast<IDataObject*>(&object)) return 1;
  if (object.Release() != 1) return 2;
  FORMATETC wrong{CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  if (object.QueryGetData(&wrong) != DV_E_FORMATETC) return 3;
  expected_paths = {L"C:\\alpha\\report.txt",
                    L"D:\\unicode\\\u4E2D\u6587-\U0001F600.bin"};
  const auto result = cgpui::run_win32_ole_file_drag(
      files, DROPEFFECT_MOVE, &inspect_files);
  if (result.result != DRAGDROP_S_DROP || result.effect != DROPEFFECT_MOVE) return 4;
  const int valid_calls = runner_calls;
  const std::vector<std::string> empty_path{""};
  const std::vector<std::string> invalid_utf8{{char(0xC3), char(0x28)}};
  const std::vector<std::string> embedded_nul{std::string{'a', '\0', 'b'}};
  if (cgpui::run_win32_ole_file_drag({}, DROPEFFECT_MOVE, &inspect_files).result != E_FAIL ||
      cgpui::run_win32_ole_file_drag(empty_path, DROPEFFECT_MOVE, &inspect_files).result != E_FAIL ||
      cgpui::run_win32_ole_file_drag(invalid_utf8, DROPEFFECT_MOVE, &inspect_files).result != E_FAIL ||
      cgpui::run_win32_ole_file_drag(embedded_nul, DROPEFFECT_MOVE, &inspect_files).result != E_FAIL ||
      runner_calls != valid_calls) return 5;
  return 0;
}
