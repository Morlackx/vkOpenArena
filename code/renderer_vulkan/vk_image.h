#ifndef VK_IMAGE_H_
#define VK_IMAGE_H_

#include "tr_image.h"

// work around, will be removed in the future

#ifndef GL_REPEAT
#define GL_REPEAT				0x2901
#endif

#ifndef GL_CLAMP
#define GL_CLAMP				0x2900
#endif

uint32_t find_memory_type(uint32_t memory_type_bits, VkMemoryPropertyFlags properties);

void vk_createViewForImageHandle(VkImage Handle, VkFormat Fmt, VkImageView* const pView);

void vk_destroyImageRes(void);

image_t* R_FindImageFile(const char *name, VkBool32 mipmap,	VkBool32 allowPicmip, int glWrapClampMode);

image_t* R_CreateImage( const char *name, unsigned char* pic, uint32_t width, uint32_t height,
						VkBool32 mipmap, VkBool32 allowPicmip, int glWrapClampMode);

void vk_createBufferResource(uint32_t Size, VkBufferUsageFlags Usage,
        VkBuffer * const pBuf, VkDeviceMemory * const pDevMem);
void vk_destroyBufferResource(VkBuffer hBuf, VkDeviceMemory hDevMem);

void R_LoadImage(const char *name, unsigned char **pic, uint32_t* width, uint32_t* height );


void R_InitImages( void );

void gpuMemUsageInfo_f(void);

image_t * R_GetScratchImageHandle(int idx);


// TODO ; impl this
// void R_ImageList_f( void );
void printImageHashTable_f(void);

#endif
