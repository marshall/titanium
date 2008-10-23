#include "npSupport.h"

std::string NPStringToString (NPString string) {
  const NPUTF8 *chars = string.UTF8Characters;
  std::string newString = chars;

  newString.erase(string.UTF8Length);
  return newString;
}
