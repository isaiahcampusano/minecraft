#include "core/BreakMode.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  BreakModeState mode;
  if(mode.fullAutoEnabled()||mode.getCurrentBreakModeText()!="SINGLE")return fail("Initial break mode was not displayed as SINGLE");
  mode.toggle();
  if(!mode.fullAutoEnabled()||mode.getCurrentBreakModeText()!="FULL AUTO")return fail("Enabling full auto did not update its display text");
  mode.toggle();
  if(mode.fullAutoEnabled()||mode.getCurrentBreakModeText()!="SINGLE")return fail("Disabling full auto did not restore the SINGLE display text");
  return 0;
}
