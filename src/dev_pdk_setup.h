#ifndef DEV_PDK_SETUP_H_
#define DEV_PDK_SETUP_H_

// TODO(aryap): There is no way that the permanent model for setting up PDKs is
// a pile of C++ exactly like the following. It should come from a human
// readable format on disk. There are some vague steps toward this goal in the
// proto/ directory but they are inchoate at best.

#include "physical_properties_database.h"

namespace bfg {

void SetUpSky130(bfg::PhysicalPropertiesDatabase *db);
void SetUpGf180Mcu(bfg::PhysicalPropertiesDatabase *db);

}  // namespace bfg

#endif  // DEV_PDK_SETUP_H_
