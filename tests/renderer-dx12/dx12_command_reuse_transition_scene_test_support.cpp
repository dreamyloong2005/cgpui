#include "dx12_command_reuse_transition_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"

namespace cgpui_test {
using namespace cgpui;
namespace {
ImageAsset pattern() {
  return {.id={9901},.logical_size={2,2},.bitmap={.width=2,.height=2,.stride=8,
      .pixels={0,255,0,255,0,0,255,255,255,255,255,255,0,0,0,255}}};
}
}
bool transition_present(const std::shared_ptr<Dx12State>& state,TransitionScene scene,TransitionUpload upload,bool capture) {
  state->frame_active=true;Dx12Frame frame(state);
  frame.clear(scene.change==TransitionChange::clear?Color{0.4F,0.1F,0.2F,1}:Color{0.02F,0.04F,0.07F,1});
  if(upload==TransitionUpload::initial) {frame.upload_image(srv_tile_image(0));frame.upload_image(pattern());}
  if(upload==TransitionUpload::replace) {auto blue=srv_tile_image(0);blue.bitmap.pixels={0,0,255,255};frame.upload_image(blue);}
  if(upload==TransitionUpload::invalidate) frame.invalidate_image({9900});
  if(upload==TransitionUpload::resupply) frame.upload_image(srv_tile_image(0));
  if(upload==TransitionUpload::undrawn) {auto extra=srv_tile_image(0);extra.id={9999};frame.upload_image(extra);}
  if(upload==TransitionUpload::undrawn_invalidate) frame.invalidate_image({9999});
  std::optional<Rect> clip;
  if(scene.change==TransitionChange::common_clip) clip=Rect{{8,8},{18,18}};
  frame.draw_rect({.rect=scene.change==TransitionChange::geometry?Rect{{1,1},{11,11}}:Rect{{1,1},{27,27}},
      .color={0.1F,0.1F,0.8F,1},.clip_rect=clip});
  auto first_image=[&]{frame.draw_image({.bounds={{4,4},{18,17}},.asset=describe_image_asset(srv_tile_image(0)),
      .sampling=ImageSamplingMode::nearest,
      .clip_rect=scene.change==TransitionChange::mixed_clip?std::optional<Rect>{Rect{{8,8},{18,18}}}:clip});};
  auto rounded=[&]{frame.draw_rounded_rect({.rect={{10,9},{16,15}},.color={1,1,0,1},.radius=BorderRadii::all(3),.clip_rect=clip});};
  if(scene.change==TransitionChange::order) {rounded();first_image();}else{first_image();rounded();}
  if(scene.second_image) frame.draw_image({.bounds={{15,14},{13,13}},.asset=describe_image_asset(pattern()),
      .sampling=scene.change==TransitionChange::sampling?ImageSamplingMode::nearest:ImageSamplingMode::linear,.clip_rect=clip});
  if(capture && !frame.request_pixel_capture()) return false;
  const auto result=frame.present();
  if(!result) std::cerr<<result.error().message<<'\n';
  return bool(result);
}
} // namespace cgpui_test
