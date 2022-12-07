#pragma once
#include "Config.h"
namespace vkUtil
{
	struct SwapChainFrame {
		vk::Image image;
		vk::ImageView imageView;
	};
}
