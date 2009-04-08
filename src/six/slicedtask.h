// Yo Emacs, this -*- C++ -*-
#ifndef SLICEDTASK_H
#define SLICEDTASK_H

/**
 * A task that does its job in neat little slices.
 */
class SlicedTask
{
public:
  virtual ~SlicedTask() {};
  virtual void doSlice() = 0;
};

#endif
