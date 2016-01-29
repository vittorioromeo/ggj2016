#pragma once

#include <vrm/core/type_aliases.hpp>
#include <vrm/core/tuple_utils.hpp>
#include <SSVStart/Global/Typedefs.hpp>
#include <SSVStart/Input/Input.hpp>

GGJ16_NAMESPACE
{
    namespace vrmc = vrm::core;
    namespace BTR = ssvs::BTR;

    using ft = ssvu::FT;
    using sfc = sf::Color;

    using vrm::core::uint;
    using vrm::core::sz_t;

    using vec2f = ssvs::Vec2f;
    using vec2i = ssvs::Vec2i;
    using vec2u = ssvs::Vec2u;

    using k_key = ssvs::KKey;
    using m_btn = ssvs::MBtn;
    using input_type = ssvs::Input::Type;
    using input_mode = ssvs::Input::Mode;
}
GGJ16_NAMESPACE_END
