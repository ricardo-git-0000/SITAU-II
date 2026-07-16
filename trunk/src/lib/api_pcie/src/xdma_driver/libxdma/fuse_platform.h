//  ------------------------------------------------------------
//  -- Copyright: 2017 Raul Mateos-Gil. University of Alcala
//  --            https://soc-repo.depeca.uah.es/gitlab
//  ------------------------------------------------------------
//  
//  ----------------------------------------------------------------------------------
//  -- Company: University of Alcala
//  -- Authors: Raul Mateos
//  -- 
//  -- Create Date: 06/11/2017 10:23:13
//  -- Design Name: xxx
//  -- Module Name: xxx (systemc module)
//  -- Project Name: xxx SoC.
//  -- Target Devices: Xilinx 7 series (Zynq)
//  -- Tool Versions: Questa Sim 10.4c / Vivado 2015.4
//  -- Description: 
//  -- 
//  -- Dependencies: RMG support package
//  -- 
//  -- Revision:
//  -- Revision 0.01 - File Created
//  -- Additional Comments:
//  -- 
//  ----------------------------------------------------------------------------------
//  -- Modification history:
//  --  06/11/2017: source code established.
//  ----------------------------------------------------------------------------------

#ifndef FUSE_PLATFORM_H
#define FUSE_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <basetsd.h>

#include "fuse_olink.h"

int HTG_K800_init_gt_clk_gen(fuse_olink *olink_drv);



#ifdef __cplusplus
}
#endif

#endif  // FUSE_PLATFORM_H
