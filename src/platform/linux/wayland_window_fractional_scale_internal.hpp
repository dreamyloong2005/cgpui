#pragma once

void initialize_fractional_scale(
    wp_fractional_scale_manager_v1* manager,
    wp_viewporter* viewporter);
void update_fractional_viewport_destination();
static void handle_preferred_fractional_scale(
    void* data,
    wp_fractional_scale_v1* fractional_scale,
    std::uint32_t scale);

wp_fractional_scale_v1* fractional_scale_ = nullptr;
wp_viewport* viewport_ = nullptr;
std::optional<float> preferred_fractional_scale_;
