// #include "tr_backend.h"
#include "vk_shade_geometry.h"
#include "tr_globals.h"
#include "vk_pipelines.h"


/*
================
Draws vertex normals for debugging
================
*/
void RB_DrawNormals (struct shaderCommands_s * const pTess, VkBool32 isPortal, VkBool32 is2D)
{
    uint32_t numVertexes = pTess->numVertexes;
    
    vec4_t xyz[SHADER_MAX_VERTEXES];
    memcpy(xyz, pTess->xyz, numVertexes * sizeof(vec4_t));
   
    memset(pTess->svars.colors, tr.identityLightByte, SHADER_MAX_VERTEXES * 4);

    // updateCurDescriptor( tr.whiteImage->descriptor_set, 0);
    updateMVP(isPortal, is2D, getptr_modelview_matrix());
    // vk_rcdUpdateViewport(is2D, DEPTH_RANGE_ZERO);

    int i = 0;
    while (i < numVertexes)
    {
        int count = numVertexes - i;
        if (count >= SHADER_MAX_VERTEXES/2 - 1)
            count = SHADER_MAX_VERTEXES/2 - 1;

        int k;
        for (k = 0; k < count; ++k)
        {
            VectorCopy(xyz[i + k], pTess->xyz[2*k]);
            VectorMA(xyz[i + k], 2, pTess->normal[i + k], pTess->xyz[2*k + 1]);
        }
        pTess->numVertexes = 2 * count;
        pTess->numIndexes = 0;

        vk_UploadXYZI(pTess->xyz, pTess->numVertexes, NULL, 0);
        
        vk_shade_geometry(g_debugPipelines.normals, pTess, VK_FALSE, VK_FALSE);

        i += count;
    }
}
