#include "tr_backend.h"
#include "vk_shade_geometry.h"
#include "tr_globals.h"
#include "vk_pipelines.h"


/*
================
Draws vertex normals for debugging
================
*/
void RB_DrawNormals (shaderCommands_t* const pTess)
{
    uint32_t numVertexes = pTess->numVertexes;
    
    vec4_t xyz[SHADER_MAX_VERTEXES];
    memcpy(xyz, pTess->xyz, numVertexes * sizeof(vec4_t));
   
    memset(pTess->svars.colors, tr.identityLightByte, SHADER_MAX_VERTEXES * 4);

    // updateCurDescriptor( tr.whiteImage->descriptor_set, 0);
    
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
        
        updateMVP(backEnd.viewParms.isPortal, backEnd.projection2D, getptr_modelview_matrix());
        vk_shade_geometry(g_debugPipelines.normals, VK_FALSE, DEPTH_RANGE_ZERO, VK_FALSE);

        i += count;
    }
}
