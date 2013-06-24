#ifndef _PASSPREVH_
#define _PASSPREVH_

extern void pass(float*,float*,int*); // calling this function between
                                      // loops prevents loop fusion
                                      // and unfair speed gains with
                                      // automatic arrays where
                                      // the intel compiler
                                      // optimizes much of the
                                      // computations away!

#endif
