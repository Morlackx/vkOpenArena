/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_shade.c

#include "tr_local.h"
#include "vk_shade_geometry.h"
#include "Vk_Instance.h"



/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/



/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t	tess;


static void R_BindAnimatedImage( textureBundle_t *bundle )
{
	int		index;

	if ( bundle->isVideoMap ) {
		ri.CIN_RunCinematic(bundle->videoMapHandle);
		ri.CIN_UploadCinematic(bundle->videoMapHandle);
		return;
	}

	if ( bundle->numImageAnimations <= 1 ) {
		GL_Bind( bundle->image[0] );
		return;
	}

	// it is necessary to do this messy calc to make sure animations line up
	// exactly with waveforms of the same frequency
	index = (int)( tess.shaderTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE );
	index >>= FUNCTABLE_SIZE2;

	if ( index < 0 ) {
		index = 0;	// may happen with shader time offsets
	}
	index %= bundle->numImageAnimations;

	GL_Bind( bundle->image[ index ] );
}

/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris (shaderCommands_t *input) {
	GL_Bind( tr.whiteImage );



	// VULKAN

    memset(tess.svars.colors, tr.identityLightByte, tess.numVertexes * 4 );
    VkPipeline pipeline = backEnd.viewParms.isMirror ? vk.tris_mirror_debug_pipeline : vk.tris_debug_pipeline;
    vk_shade_geometry(pipeline, qfalse, force_zero, qtrue);


}


/*
================
DrawNormals

Draws vertex normals for debugging
================
*/
static void DrawNormals (shaderCommands_t *input) {
	int		i;


	// VULKAN

    vec4_t xyz[SHADER_MAX_VERTEXES];
    memcpy(xyz, tess.xyz, tess.numVertexes * sizeof(vec4_t));
    memset(tess.svars.colors, tr.identityLightByte, SHADER_MAX_VERTEXES * sizeof(color4ub_t));

    int numVertexes = tess.numVertexes;
    i = 0;
    while (i < numVertexes) {
        int count = numVertexes - i;
        if (count >= SHADER_MAX_VERTEXES/2 - 1)
            count = SHADER_MAX_VERTEXES/2 - 1;

        for (int k = 0; k < count; k++) {
            VectorCopy(xyz[i + k], tess.xyz[2*k]);
            VectorMA(xyz[i + k], 2, input->normal[i + k], tess.xyz[2*k + 1]);
        }
        tess.numVertexes = 2 * count;
        tess.numIndexes = 0;


        vk_bind_geometry();
        vk_shade_geometry(vk.normals_debug_pipeline, qfalse, force_zero, qfalse);


        i += count;
    }

}

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
void RB_BeginSurface( shader_t *shader, int fogNum ) {

	shader_t *state = (shader->remappedShader) ? shader->remappedShader : shader;

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.shader = state;
	tess.fogNum = fogNum;
	tess.dlightBits = 0;		// will be OR'd in by surface functions
	tess.xstages = state->stages;
	tess.numPasses = state->numUnfoggedPasses;

	tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
	if (tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime) {
		tess.shaderTime = tess.shader->clampTime;
	}


}



/*
===================
ProjectDlightTexture

Perform dynamic lighting with another rendering pass
===================
*/
static void ProjectDlightTexture( void ) {
	int		i, l;
	vec3_t	origin;
	float	*texCoords;
	byte	*colors;
	byte	clipBits[SHADER_MAX_VERTEXES];
	int		numIndexes;
	float	scale;
	float	radius;
	vec3_t	floatColor;
	float	modulate;

	if ( !backEnd.refdef.num_dlights ) {
		return;
	}

	for ( l = 0 ; l < backEnd.refdef.num_dlights ; l++ ) {
		dlight_t	*dl;

		if ( !( tess.dlightBits & ( 1 << l ) ) ) {
			continue;	// this surface definately doesn't have any of this light
		}
		texCoords = tess.svars.texcoords[0][0];
		colors = tess.svars.colors[0];

		dl = &backEnd.refdef.dlights[l];
		VectorCopy( dl->transformed, origin );
        radius = dl->radius;
		scale = 1.0f / radius;

		floatColor[0] = dl->color[0] * 255.0f;
		floatColor[1] = dl->color[1] * 255.0f;
		floatColor[2] = dl->color[2] * 255.0f;

		for ( i = 0 ; i < tess.numVertexes ; i++, texCoords += 2, colors += 4 ) {
			vec3_t	dist;
			int		clip;

			backEnd.pc.c_dlightVertexes++;

			VectorSubtract( origin, tess.xyz[i], dist );
			texCoords[0] = 0.5f + dist[0] * scale;
			texCoords[1] = 0.5f + dist[1] * scale;

			clip = 0;
			if ( texCoords[0] < 0.0f ) {
				clip |= 1;
			} else if ( texCoords[0] > 1.0f ) {
				clip |= 2;
			}
			if ( texCoords[1] < 0.0f ) {
				clip |= 4;
			} else if ( texCoords[1] > 1.0f ) {
				clip |= 8;
			}
			// modulate the strength based on the height and color
			if ( dist[2] > radius ) {
				clip |= 16;
				modulate = 0.0f;
			} else if ( dist[2] < -radius ) {
				clip |= 32;
				modulate = 0.0f;
			} else {
				dist[2] = fabs(dist[2]);
				if ( dist[2] < radius * 0.5f ) {
					modulate = 1.0f;
				} else {
					modulate = 2.0f * (radius - dist[2]) * scale;
				}
			}
			clipBits[i] = clip;

			colors[0] = (floatColor[0] * modulate);
			colors[1] = (floatColor[1] * modulate);
			colors[2] = (floatColor[2] * modulate);
			colors[3] = 255;
		}

		// build a list of triangles that need light
		numIndexes = 0;
		for ( i = 0 ; i < tess.numIndexes ; i += 3 ) {
			int		a, b, c;

			a = tess.indexes[i];
			b = tess.indexes[i+1];
			c = tess.indexes[i+2];
			if ( clipBits[a] & clipBits[b] & clipBits[c] ) {
				continue;	// not lighted
			}
			numIndexes += 3;
		}

		if ( !numIndexes ) {
			continue;
		}



		GL_Bind( tr.dlightImage );
		// include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
		// where they aren't rendered
		backEnd.pc.c_totalIndexes += numIndexes;
		backEnd.pc.c_dlightIndexes += numIndexes;

		// VULKAN

		VkPipeline pipeline = vk.dlight_pipelines[dl->additive > 0 ? 1 : 0][tess.shader->cullType][tess.shader->polygonOffset];
		vk_shade_geometry(pipeline, qfalse, normal, qtrue);

	}
}


/*
===================
RB_FogPass

Blends a fog texture on top of everything else
===================
*/
static void RB_FogPass( void ) {
	fog_t		*fog;
	int			i;


	fog = tr.world->fogs + tess.fogNum;

	for ( i = 0; i < tess.numVertexes; i++ ) {
		* ( int * )&tess.svars.colors[i] = fog->colorInt;
	}

	RB_CalcFogTexCoords( ( float * ) tess.svars.texcoords[0] );

	GL_Bind( tr.fogImage );

	// VULKAN

    assert(tess.shader->fogPass > 0);
    VkPipeline pipeline = vk.fog_pipelines[tess.shader->fogPass - 1][tess.shader->cullType][tess.shader->polygonOffset];
    vk_shade_geometry(pipeline, qfalse, normal, qtrue);


}

/*
===============
ComputeColors
===============
*/
static void ComputeColors( shaderStage_t *pStage )
{
	int		i;

	//
	// rgbGen
	//
	switch ( pStage->rgbGen )
	{
		case CGEN_IDENTITY:
			memset( tess.svars.colors, 0xff, tess.numVertexes * 4 );
			break;
		default:
		case CGEN_IDENTITY_LIGHTING:
			memset( tess.svars.colors, tr.identityLightByte, tess.numVertexes * 4 );
			break;
		case CGEN_LIGHTING_DIFFUSE:
			RB_CalcDiffuseColor( tess.svars.colors );
			break;
		case CGEN_EXACT_VERTEX:
			memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0] ) );
			break;
		case CGEN_CONST:
			for ( i = 0; i < tess.numVertexes; i++ ) {
				*(int *)tess.svars.colors[i] = *(int *)pStage->constantColor;
			}
			break;
		case CGEN_VERTEX:
			if ( tr.identityLight == 1 )
			{
				memcpy( tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof( tess.vertexColors[0] ) );
			}
			else
			{
				for ( i = 0; i < tess.numVertexes; i++ )
				{
					tess.svars.colors[i][0] = tess.vertexColors[i][0] * tr.identityLight;
					tess.svars.colors[i][1] = tess.vertexColors[i][1] * tr.identityLight;
					tess.svars.colors[i][2] = tess.vertexColors[i][2] * tr.identityLight;
					tess.svars.colors[i][3] = tess.vertexColors[i][3];
				}
			}
			break;
		case CGEN_ONE_MINUS_VERTEX:
			if ( tr.identityLight == 1 )
			{
				for ( i = 0; i < tess.numVertexes; i++ )
				{
					tess.svars.colors[i][0] = 255 - tess.vertexColors[i][0];
					tess.svars.colors[i][1] = 255 - tess.vertexColors[i][1];
					tess.svars.colors[i][2] = 255 - tess.vertexColors[i][2];
				}
			}
			else
			{
				for ( i = 0; i < tess.numVertexes; i++ )
				{
					tess.svars.colors[i][0] = ( 255 - tess.vertexColors[i][0] ) * tr.identityLight;
					tess.svars.colors[i][1] = ( 255 - tess.vertexColors[i][1] ) * tr.identityLight;
					tess.svars.colors[i][2] = ( 255 - tess.vertexColors[i][2] ) * tr.identityLight;
				}
			}
			break;
		case CGEN_FOG:
			{
				fog_t		*fog;

				fog = tr.world->fogs + tess.fogNum;

				for ( i = 0; i < tess.numVertexes; i++ ) {
					* ( int * )&tess.svars.colors[i] = fog->colorInt;
				}
			}
			break;
		case CGEN_WAVEFORM:
			RB_CalcWaveColor( &pStage->rgbWave, ( unsigned char * ) tess.svars.colors );
			break;
		case CGEN_ENTITY:
			RB_CalcColorFromEntity( ( unsigned char * ) tess.svars.colors );
			break;
		case CGEN_ONE_MINUS_ENTITY:
			RB_CalcColorFromOneMinusEntity( ( unsigned char * ) tess.svars.colors );
			break;
	}

	//
	// alphaGen
	//
	switch ( pStage->alphaGen )
	{
	case AGEN_SKIP:
		break;
	case AGEN_IDENTITY:
		if ( pStage->rgbGen != CGEN_IDENTITY ) {
			if ( ( pStage->rgbGen == CGEN_VERTEX && tr.identityLight != 1 ) ||
				 pStage->rgbGen != CGEN_VERTEX ) {
				for ( i = 0; i < tess.numVertexes; i++ ) {
					tess.svars.colors[i][3] = 0xff;
				}
			}
		}
		break;
	case AGEN_CONST:
		if ( pStage->rgbGen != CGEN_CONST ) {
			for ( i = 0; i < tess.numVertexes; i++ ) {
				tess.svars.colors[i][3] = pStage->constantColor[3];
			}
		}
		break;
	case AGEN_WAVEFORM:
		RB_CalcWaveAlpha( &pStage->alphaWave, ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_LIGHTING_SPECULAR:
		RB_CalcSpecularAlpha( ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_ENTITY:
		RB_CalcAlphaFromEntity( ( unsigned char * ) tess.svars.colors );
		break;
	case AGEN_ONE_MINUS_ENTITY:
		RB_CalcAlphaFromOneMinusEntity( ( unsigned char * ) tess.svars.colors );
		break;
    case AGEN_VERTEX:
		if ( pStage->rgbGen != CGEN_VERTEX ) {
			for ( i = 0; i < tess.numVertexes; i++ ) {
				tess.svars.colors[i][3] = tess.vertexColors[i][3];
			}
		}
        break;
    case AGEN_ONE_MINUS_VERTEX:
        for ( i = 0; i < tess.numVertexes; i++ )
        {
			tess.svars.colors[i][3] = 255 - tess.vertexColors[i][3];
        }
        break;
	case AGEN_PORTAL:
		{
			unsigned char alpha;

			for ( i = 0; i < tess.numVertexes; i++ )
			{
				float len;
				vec3_t v;

				VectorSubtract( tess.xyz[i], backEnd.viewParms.or.origin, v );
				len = VectorLength( v );

				len /= tess.shader->portalRange;

				if ( len < 0 )
				{
					alpha = 0;
				}
				else if ( len > 1 )
				{
					alpha = 0xff;
				}
				else
				{
					alpha = len * 0xff;
				}

				tess.svars.colors[i][3] = alpha;
			}
		}
		break;
	}

	//
	// fog adjustment for colors to fade out as fog increases
	//
	if ( tess.fogNum )
	{
		switch ( pStage->adjustColorsForFog )
		{
		case ACFF_MODULATE_RGB:
			RB_CalcModulateColorsByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_MODULATE_ALPHA:
			RB_CalcModulateAlphasByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_MODULATE_RGBA:
			RB_CalcModulateRGBAsByFog( ( unsigned char * ) tess.svars.colors );
			break;
		case ACFF_NONE:
			break;
		}
	}
}

/*
===============
ComputeTexCoords
===============
*/
static void ComputeTexCoords( shaderStage_t *pStage ) {
	int		i;
	int		b;

	for ( b = 0; b < NUM_TEXTURE_BUNDLES; b++ ) {
		int tm;

		//
		// generate the texture coordinates
		//
		switch ( pStage->bundle[b].tcGen )
		{
		case TCGEN_IDENTITY:
			memset( tess.svars.texcoords[b], 0, sizeof( float ) * 2 * tess.numVertexes );
			break;
		case TCGEN_TEXTURE:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {
				tess.svars.texcoords[b][i][0] = tess.texCoords[i][0][0];
				tess.svars.texcoords[b][i][1] = tess.texCoords[i][0][1];
			}
			break;
		case TCGEN_LIGHTMAP:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {
				tess.svars.texcoords[b][i][0] = tess.texCoords[i][1][0];
				tess.svars.texcoords[b][i][1] = tess.texCoords[i][1][1];
			}
			break;
		case TCGEN_VECTOR:
			for ( i = 0 ; i < tess.numVertexes ; i++ ) {
				tess.svars.texcoords[b][i][0] = DotProduct( tess.xyz[i], pStage->bundle[b].tcGenVectors[0] );
				tess.svars.texcoords[b][i][1] = DotProduct( tess.xyz[i], pStage->bundle[b].tcGenVectors[1] );
			}
			break;
		case TCGEN_FOG:
			RB_CalcFogTexCoords( ( float * ) tess.svars.texcoords[b] );
			break;
		case TCGEN_ENVIRONMENT_MAPPED:
			RB_CalcEnvironmentTexCoords( ( float * ) tess.svars.texcoords[b] );
			break;
		case TCGEN_BAD:
			return;
		}

		//
		// alter texture coordinates
		//
		for ( tm = 0; tm < pStage->bundle[b].numTexMods ; tm++ ) {
			switch ( pStage->bundle[b].texMods[tm].type )
			{
			case TMOD_NONE:
				tm = TR_MAX_TEXMODS;		// break out of for loop
				break;

			case TMOD_TURBULENT:
				RB_CalcTurbulentTexCoords( &pStage->bundle[b].texMods[tm].wave, 
						                 ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_ENTITY_TRANSLATE:
				RB_CalcScrollTexCoords( backEnd.currentEntity->e.shaderTexCoord,
									 ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_SCROLL:
				RB_CalcScrollTexCoords( pStage->bundle[b].texMods[tm].scroll,
										 ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_SCALE:
				RB_CalcScaleTexCoords( pStage->bundle[b].texMods[tm].scale,
									 ( float * ) tess.svars.texcoords[b] );
				break;
			
			case TMOD_STRETCH:
				RB_CalcStretchTexCoords( &pStage->bundle[b].texMods[tm].wave, 
						               ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_TRANSFORM:
				RB_CalcTransformTexCoords( &pStage->bundle[b].texMods[tm],
						                 ( float * ) tess.svars.texcoords[b] );
				break;

			case TMOD_ROTATE:
				RB_CalcRotateTexCoords( pStage->bundle[b].texMods[tm].rotateSpeed,
										( float * ) tess.svars.texcoords[b] );
				break;

			default:
				ri.Error( ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'\n", pStage->bundle[b].texMods[tm].type, tess.shader->name );
				break;
			}
		}
	}
}



void RB_StageIteratorGeneric( void )
{
//	shaderCommands_t *input;

//	input = &tess;

	RB_DeformTessGeometry();

    extern int s_CurTmu;
	//
	// call shader function
	//
	// VULKAN

	vk_bind_geometry();
    
    int stage = 0;

	for ( stage = 0; stage < MAX_SHADER_STAGES; stage++ )
	{
		if ( NULL == tess.xstages[stage])
		{
			break;
		}

		ComputeColors( tess.xstages[stage] );
		ComputeTexCoords( tess.xstages[stage] );

        // base
        s_CurTmu = 0 ;

        // set state
		R_BindAnimatedImage( &tess.xstages[stage]->bundle[0] );
		//
		// do multitexture
		//
        qboolean multitexture = (tess.xstages[stage]->bundle[1].image[0] != NULL);

		if ( multitexture )
		{
            // DrawMultitextured( input, stage );
            // output = t0 * t1 or t0 + t1

            // t0 = most upstream according to spec
            // t1 = most downstream according to spec
            // this is an ugly hack to work around a GeForce driver
            // bug with multitexture and clip planes


            // lightmap/secondary pass
            s_CurTmu = 1 ;

            R_BindAnimatedImage( &tess.xstages[stage]->bundle[1] );
            // disable texturing on TEXTURE1, then select TEXTURE0
            s_CurTmu = 0;
		}

		// VULKAN

        VkPipeline vk_pipeline;
        if (backEnd.viewParms.isMirror) {
            vk_pipeline = tess.xstages[stage]->vk_mirror_pipeline;
        }
        else if (backEnd.viewParms.isPortal) {
            vk_pipeline = tess.xstages[stage]->vk_portal_pipeline;
        }
        else
        {
            vk_pipeline = tess.xstages[stage]->vk_pipeline;
        }

        enum Vk_Depth_Range depth_range = normal;
        if (tess.shader->isSky) {
            depth_range = force_one;
            if (r_showsky->integer)
                depth_range = force_zero;
        } else if (backEnd.currentEntity->e.renderfx & RF_DEPTHHACK) {
            depth_range = weapon;
        }

        if (r_lightmap->integer && multitexture)
            GL_Bind(tr.whiteImage); // replace diffuse texture with a white one thus effectively render only lightmap


        vk_shade_geometry(vk_pipeline, multitexture, depth_range, qtrue);
        

		// allow skipping out to show just lightmaps during development
		if ( r_lightmap->integer && ( tess.xstages[stage]->bundle[0].isLightmap || tess.xstages[stage]->bundle[1].isLightmap ) )
		{
			break;
		}
	}

	// 
	// now do any dynamic lighting needed
	//
	if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE
		&& !(tess.shader->surfaceFlags & (SURF_NODLIGHT | SURF_SKY) ) ) {
		ProjectDlightTexture();
	}

	//
	// now do fog
	//
	if ( tess.fogNum && tess.shader->fogPass ) {
		RB_FogPass();
	}
}





/*
** RB_EndSurface
*/
void RB_EndSurface( void ) {
	shaderCommands_t *input;

	input = &tess;

	if (input->numIndexes == 0) {
		return;
	}
	if (tess.shader->isSky && r_fastsky->integer) {
		return;
	}

	if (input->indexes[SHADER_MAX_INDEXES-1] != 0) {
		ri.Error (ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit");
	}	
	if (input->xyz[SHADER_MAX_VERTEXES-1][0] != 0) {
		ri.Error (ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit");
	}

	if ( tess.shader == tr.shadowShader ) {
		RB_ShadowTessEnd();
		return;
	}

	// for debugging of sort order issues, stop rendering after a given sort value
	if ( r_debugSort->integer && r_debugSort->integer < tess.shader->sort ) {
		return;
	}

	//
	// update performance counters
	//
	backEnd.pc.c_shaders++;
	backEnd.pc.c_vertexes += tess.numVertexes;
	backEnd.pc.c_indexes += tess.numIndexes;
	backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	//
	// call off to shader specific tess end function
	//
	if (tess.shader->isSky)
		RB_StageIteratorSky();
	else
		RB_StageIteratorGeneric();

	//
	// draw debugging stuff
	//
	if ( r_showtris->integer ) {
		DrawTris (input);
	}
	if ( r_shownormals->integer ) {
		DrawNormals (input);
	}
	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;
    tess.numVertexes = 0;

}

