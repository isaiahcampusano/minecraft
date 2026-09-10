#include "player/CraftingRecipe.h"
#include "player/ToolRegistry.h"
#include <array>
#include <iostream>

namespace {int fail(const char* message){std::cerr<<message<<'\n';return 1;}}

int main(){
  int toolRecipes=0;std::array<std::array<bool,static_cast<std::size_t>(ToolTier::COUNT)>,static_cast<std::size_t>(ToolKind::COUNT)> toolSeen{};for(const auto& recipe:CraftingRegistry::recipes())if(recipe.output.kind==ItemKind::TOOL){++toolRecipes;toolSeen[static_cast<std::size_t>(recipe.output.toolKind)][static_cast<std::size_t>(recipe.output.toolTier)]=true;if(!recipe.requiresTable())return fail("tool recipe did not require the crafting table");}if(CraftingRegistry::recipes().size()!=13||toolRecipes!=10)return fail("initial recipe registry did not contain 3 material and 10 tool recipes");for(const auto& tiers:toolSeen)for(bool found:tiers)if(!found)return fail("a tool kind/tier recipe was missing");
  std::array<ItemStack,9> grid{};
  grid[4]=ItemStack::block(BlockType::OAK_LOG,3);const auto* planks=CraftingRegistry::match(grid,3,3);if(!planks||planks->output.blockType!=BlockType::PLANKS||planks->output.count!=4)return fail("shapeless log recipe did not ignore position");
  ItemStack cursor;if(!CraftingRegistry::craft(grid,3,3,cursor)||grid[4].count!=2||cursor.blockType!=BlockType::PLANKS||cursor.count!=4)return fail("crafting did not consume exactly one log");

  grid={};grid[0]=ItemStack::block(BlockType::PLANKS,2);if(CraftingRegistry::match(grid,2,2))return fail("one occupied plank slot matched the two-plank stick recipe");
  grid[3]=ItemStack::block(BlockType::PLANKS,2);const auto* sticks=CraftingRegistry::match(grid,2,2);if(!sticks||sticks->output.kind!=ItemKind::MATERIAL||sticks->output.materialType!=MaterialType::STICK)return fail("shapeless stick recipe did not match scattered planks");
  cursor={};if(!CraftingRegistry::craft(grid,2,2,cursor)||grid[0].count!=1||grid[3].count!=1||cursor.count!=4)return fail("shapeless craft consumed whole input stacks");

  const auto wood=ItemStack::block(BlockType::PLANKS),stick=ItemStack::material(MaterialType::STICK);
  grid={};grid[0]=wood;grid[1]=wood;grid[3]=wood;grid[4]=stick;grid[7]=stick;const auto* axe=CraftingRegistry::match(grid,3,3);if(!axe||axe->output.kind!=ItemKind::TOOL||axe->output.toolKind!=ToolKind::AXE||axe->output.toolTier!=ToolTier::WOOD)return fail("aligned wooden axe recipe did not match");
  grid={};grid[1]=wood;grid[4]=stick;grid[7]=stick;const auto* shovel=CraftingRegistry::match(grid,3,3);if(!shovel||shovel->output.kind!=ItemKind::TOOL||shovel->output.toolKind!=ToolKind::SHOVEL||shovel->output.toolTier!=ToolTier::WOOD)return fail("translated wooden shovel recipe did not match");
  cursor={};if(!CraftingRegistry::craft(grid,3,3,cursor)||!grid[1].empty()||!grid[4].empty()||!grid[7].empty()||cursor.toolKind!=ToolKind::SHOVEL)return fail("translated shaped craft consumed the wrong slots");
  grid={};grid[0]=wood;grid[1]=wood;grid[4]=wood;grid[3]=stick;grid[6]=stick;if(CraftingRegistry::match(grid,3,3))return fail("mirrored axe recipe matched without an explicit mirrored recipe");
  grid={};grid[0]=wood;grid[1]=wood;grid[3]=stick;grid[4]=wood;grid[5]=stick;if(CraftingRegistry::match(grid,3,3))return fail("rotated axe recipe matched without an explicit rotated recipe");
  grid={};grid[0]=wood;grid[1]=wood;grid[3]=wood;grid[4]=stick;if(CraftingRegistry::match(grid,3,3))return fail("insufficient tool ingredients matched");
  return 0;
}
