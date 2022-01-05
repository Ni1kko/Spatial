#pragma once

#include <Menu/stb_image.h>
#include <Menu/Texture.h>

#include "Resources/avatar_ct.h"
#include "Resources/avatar_tt.h"
#include "Resources/menu_bg.h"

struct PNGTexture {
    template <std::size_t N>
    PNGTexture(const std::array<char, N>& png) noexcept : pngData{ png.data() }, pngDataSize{ png.size() } {}

    ImTextureID getTexture() const noexcept
    {
        if (!texture.get()) {
            int width, height;
            stbi_set_flip_vertically_on_load_thread(false);

            if (const auto data = stbi_load_from_memory((const stbi_uc*)pngData, pngDataSize, &width, &height, nullptr, STBI_rgb_alpha)) {
                texture.init(width, height, data);
                stbi_image_free(data);
            }
            else {
                assert(false);
            }
        }

        return texture.get();
    }

    void clearTexture() const noexcept { texture.clear(); }

private:
    const char* pngData;
    std::size_t pngDataSize;

    mutable Texture texture;
};

static const PNGTexture avatarTT{ Resource::avatar_tt };
static const PNGTexture avatarCT{ Resource::avatar_ct };
static const PNGTexture menuBG{ Resource::menu_bg };