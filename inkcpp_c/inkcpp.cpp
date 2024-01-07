#include "inkcpp.h"

#include <story.h>
#include <snapshot.h>
#include <globals.h>
#include <runner.h>
#include <choice.h>

using namespace ink::runtime;

extern "C"
{
  HInkSnapshot* ink_snapshot_from_file(const char* filename) {
    return reinterpret_cast<HInkSnapshot*>(snapshot::from_file(filename));
  }
  int ink_snapshot_num_runners(const HInkSnapshot* self) {
    return reinterpret_cast<const snapshot*>(self)->num_runners();
  }
  void ink_snapshot_write_to_file(const HInkSnapshot* self, const char* filename) {
    reinterpret_cast<const snapshot*>(self)->write_to_file(filename);
  }
}
