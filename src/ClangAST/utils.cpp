/* Copyright 2023 R. Thomas
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string>
#include "log.hpp"
#include "iCDump/ObjC/Protocol.hpp"
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Property.hpp"

#include "ASTGen.hpp"

#include <llvm/Support/raw_ostream.h>

#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/IdentifierTable.h>


using namespace clang;

namespace iCDump {
namespace ClangAST {

PrintingPolicy get_print_policy() {
  PrintingPolicy policy = LangOptions();
  policy.Bool = true;
  return policy;
}

void init_TU(ASTGen& gen, DeclContext* DC) {
  //ASTContext& ctx = gen.ast_ctx();
  //IdentifierInfo& id = ctx.Idents.get("uint32_t");
  //auto x = TypedefDecl::Create(ctx, DC, SourceLocation(), SourceLocation(), &id, nullptr);
  //TypeDecl* decl = ctx.buildImplicitTypedef(ctx.UnsignedIntTy, "uint32_t");
  //DC->addDecl(x);
    //case OBJC_TYPES::UNSIGNED_INT:
    //  {
    //    IdentifierInfo& id = ctx.Idents.get("uint32_t");
    //    TypeSourceInfo* ti = ctx.getTrivialTypeSourceInfo(ctx.UnsignedIntTy);
    //    TypedefDecl* u32 = TypedefDecl::Create(
    //        ctx, DC, SourceLocation(), SourceLocation(),
    //        &id, ti);
    //    return ctx.getTypeDeclType(u32);
    //  }
}

std::string generate(const ObjC::Protocol& protocol) {
  ASTGen& generator = ASTGen::get();
  ASTContext& ctx = generator.ast_ctx();

  auto TU = TranslationUnitDecl::Create(ctx);
  init_TU(generator, TU);
  generator.decl_protocol(protocol, TU);

  PrintingPolicy policy = get_print_policy();
  std::string out;
  llvm::raw_string_ostream rso(out);

  TU->print(rso, policy);
  return out;
}


std::string generate(const ObjC::Class& cls) {
  ASTGen& generator = ASTGen::get();
  ASTContext& ctx = generator.ast_ctx();

  auto TU = TranslationUnitDecl::Create(ctx);
  init_TU(generator, TU);
  generator.decl_class(cls, TU);

  PrintingPolicy policy = get_print_policy();
  std::string out;
  llvm::raw_string_ostream rso(out);

  TU->print(rso, policy);
  return out;
}

std::string generate(const ObjC::Property& property) {
  ASTGen& generator = ASTGen::get();
  ASTContext& ctx = generator.ast_ctx();

  auto TU = TranslationUnitDecl::Create(ctx);
  //generator.decl_property(property, TU);

  PrintingPolicy policy = get_print_policy();
  std::string out;
  llvm::raw_string_ostream rso(out);

  TU->print(rso, policy);
  return out;
}

}
}
