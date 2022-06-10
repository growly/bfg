#include "sky130_buf.h"

#include <iostream>
#include <memory>
#include <string>

#include "atom.h"
#include "../cell.h"

namespace bfg {
namespace atoms {

bfg::Cell *Sky130Buf::Generate() {
  std::unique_ptr<bfg::Cell> cell(new bfg::Cell("sky130_buf"));

  return cell.release();
}

}  // namespace atoms
}  // namespace bfg
