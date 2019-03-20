#ifndef VK_CLEAER_ATTACHMENTS_H_
#define VK_CLEAER_ATTACHMENTS_H_


#include "qvk.h"

void vk_clear_attachments(VkBool32 clear_depth_stencil, VkBool32 clear_color, float* color);


void set_depth_attachment(VkBool32 s);

VkBool32 get_depth_attachment(void);
#endif
