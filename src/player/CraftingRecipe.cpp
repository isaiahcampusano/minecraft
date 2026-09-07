#include "CraftingRecipe.h"
#include "ToolRegistry.h"
#include <algorithm>

namespace {
RecipeIngredient block(BlockType type){RecipeIngredient value;value.blockType=type;return value;}
RecipeIngredient material(MaterialType type){RecipeIngredient value;value.kind=ItemKind::MATERIAL;value.materialType=type;return value;}
bool matches(const ItemStack& stack,const RecipeIngredient& ingredient){
  if(ingredient.empty())return stack.empty();
  if(stack.empty()||stack.kind!=ingredient.kind)return false;
  return stack.kind==ItemKind::BLOCK?stack.blockType==ingredient.blockType:stack.kind==ItemKind::MATERIAL&&stack.materialType==ingredient.materialType;
}
bool stackableWith(const ItemStack& a,const ItemStack& b){
  if(a.empty())return true;
  if(a.kind!=b.kind||a.kind==ItemKind::TOOL)return false;
  return sameItemType(a,b);
}
CraftingRecipe shaped(int width,int height,std::initializer_list<RecipeIngredient> ingredients,const ItemStack& output){
  CraftingRecipe recipe;recipe.width=width;recipe.height=height;recipe.output=output;std::copy(ingredients.begin(),ingredients.end(),recipe.grid.begin());return recipe;
}
CraftingRecipe shapeless(int width,int height,std::initializer_list<RecipeIngredient> ingredients,const ItemStack& output){auto recipe=shaped(width,height,ingredients,output);recipe.shapeless=true;return recipe;}
ItemStack tool(ToolKind kind,ToolTier tier){return ItemStack::tool(kind,tier,maxToolDurability(tier));}
RecipeIngredient tierMaterial(ToolTier tier){return block(tier==ToolTier::WOOD?BlockType::PLANKS:BlockType::COBBLESTONE);}
void addToolRecipes(std::vector<CraftingRecipe>& recipes,ToolTier tier){
  const auto m=tierMaterial(tier),s=material(MaterialType::STICK),e=block(BlockType::AIR);
  recipes.push_back(shaped(3,3,{m,m,m,e,s,e,e,s,e},tool(ToolKind::PICKAXE,tier)));
  recipes.push_back(shaped(2,3,{m,m,m,s,e,s},tool(ToolKind::AXE,tier)));
  recipes.push_back(shaped(1,3,{m,s,s},tool(ToolKind::SHOVEL,tier)));
  recipes.push_back(shaped(1,3,{m,m,s},tool(ToolKind::SWORD,tier)));
  recipes.push_back(shaped(2,3,{m,m,e,s,e,s},tool(ToolKind::HOE,tier)));
}
}

const std::vector<CraftingRecipe>& CraftingRegistry::recipes(){
  static const std::vector<CraftingRecipe> value=[](){
    std::vector<CraftingRecipe> result;
    result.push_back(shapeless(1,1,{block(BlockType::OAK_LOG)},ItemStack::block(BlockType::PLANKS,4)));
    result.push_back(shapeless(2,1,{block(BlockType::PLANKS),block(BlockType::PLANKS)},ItemStack::material(MaterialType::STICK,4)));
    result.push_back(shaped(2,2,{block(BlockType::PLANKS),block(BlockType::PLANKS),block(BlockType::PLANKS),block(BlockType::PLANKS)},ItemStack::block(BlockType::CRAFTING_TABLE)));
    addToolRecipes(result,ToolTier::WOOD);addToolRecipes(result,ToolTier::STONE);return result;
  }();
  return value;
}

const CraftingRecipe* CraftingRegistry::match(const std::array<ItemStack,9>& input,int width,int height){
  if(width<1||width>3||height<1||height>3)return nullptr;
  for(const auto& recipe:recipes()){
    if(recipe.width>width||recipe.height>height)continue;
    if(recipe.shapeless){
      std::array<bool,9> used{};bool ok=true;
      for(int r=0;r<recipe.width*recipe.height&&ok;++r){if(recipe.grid[static_cast<std::size_t>(r)].empty())continue;bool found=false;for(int i=0;i<width*height;++i)if(!used[static_cast<std::size_t>(i)]&&matches(input[static_cast<std::size_t>(i)],recipe.grid[static_cast<std::size_t>(r)])){used[static_cast<std::size_t>(i)]=true;found=true;break;}ok=found;}
      if(!ok)continue;
      for(int i=0;i<width*height;++i)if(!input[static_cast<std::size_t>(i)].empty()&&!used[static_cast<std::size_t>(i)]){ok=false;break;}
      if(ok)return &recipe;
      continue;
    }
    bool ok=true;
    for(int row=0;row<height&&ok;++row)for(int col=0;col<width;++col){RecipeIngredient ingredient; if(row<recipe.height&&col<recipe.width)ingredient=recipe.grid[static_cast<std::size_t>(row*recipe.width+col)];if(!matches(input[static_cast<std::size_t>(row*width+col)],ingredient)){ok=false;break;}}
    if(ok)return &recipe;
  }
  return nullptr;
}

bool CraftingRegistry::craft(std::array<ItemStack,9>& grid,int width,int height,ItemStack& cursor){
  const CraftingRecipe* recipe=match(grid,width,height);if(!recipe||!stackableWith(cursor,recipe->output))return false;
  if(!cursor.empty()&&cursor.count+recipe->output.count>64)return false;
  if(recipe->shapeless){
    std::array<bool,9> used{};for(int r=0;r<recipe->width*recipe->height;++r){const auto& ingredient=recipe->grid[static_cast<std::size_t>(r)];if(ingredient.empty())continue;for(int i=0;i<width*height;++i)if(!used[static_cast<std::size_t>(i)]&&matches(grid[static_cast<std::size_t>(i)],ingredient)){used[static_cast<std::size_t>(i)]=true;if(--grid[static_cast<std::size_t>(i)].count==0)grid[static_cast<std::size_t>(i)]={};break;}}
  }else{
    for(int row=0;row<recipe->height;++row)for(int col=0;col<recipe->width;++col){const auto& ingredient=recipe->grid[static_cast<std::size_t>(row*recipe->width+col)];if(ingredient.empty())continue;auto& stack=grid[static_cast<std::size_t>(row*width+col)];if(--stack.count==0)stack={};}
  }
  if(cursor.empty())cursor=recipe->output;else cursor.count+=recipe->output.count;return true;
}
