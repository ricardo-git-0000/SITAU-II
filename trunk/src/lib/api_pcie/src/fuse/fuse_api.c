#include <fuse_api.h>

int api_main()
{
  fuse_status_t status;

  status = FUSE_STATUS_HW_ERR | FUSE_STATUS_SW_ERR;

  return 0;
}
