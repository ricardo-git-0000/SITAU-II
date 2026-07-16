#ifndef FUSE_EMUL_INTS_H
#define FUSE_EMUL_INTS_H

//----------------------
//-- INTERRUPT MASKS: --
//----------------------

#define RUN_DONE_INT_MASK     0x0001      // End of sequence (of HW images) transmission    (event interrupt)
#define IMG_DONE_INT_MASK     0x0002      // End of HW image transmission                   (event interrupt)


#endif // FUSE_EMUL_INTS_H

