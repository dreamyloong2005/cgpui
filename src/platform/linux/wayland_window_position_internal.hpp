#pragma once

[[nodiscard]] PlatformWindowPositionState position_state() const override;
bool request_position(Point position) override;
