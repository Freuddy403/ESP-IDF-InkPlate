
#define __INKPLATE_PLATFORM__ 1
#include "drivers/inkplate_platform.hpp"
#include "drivers/eink_6.hpp"
#include "drivers/mcp23017.hpp"
#include "drivers/inkplate_platform.hpp"  // ou le bon chemin exact selon ton include

InkPlatePlatform InkPlatePlatform::singleton;

MCP23017 mcp(0x20);
EInk6 eink(mcp);
//EInk& e_ink = eink;
