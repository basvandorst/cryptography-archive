/*
 * definitions for a stupid encryption pipeline: rot13
 *
 * $Id: pgpRot13Mod.h,v 1.5 1997/06/25 19:40:25 lloyd Exp $
 */

#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

PGPPipeline **rot13Create (PGPPipeline **head);

PGP_END_C_DECLARATIONS