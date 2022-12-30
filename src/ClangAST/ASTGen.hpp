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
#ifndef ICDUMP_ASTGEN_H_
#define ICDUMP_ASTGEN_H_
#include <memory>
namespace clang {
class ASTContext;
class CompilerInstance;
class DeclContext;
class ObjCContainerDecl;
class ObjCInterfaceDecl;
class ObjCIvarDecl;
class ObjCMethodDecl;
class ObjCPropertyDecl;
class ObjCProtocolDecl;
class ParmVarDecl;
class QualType;
}

namespace iCDump {
namespace ObjC {
class Class;
class IVar;
class Method;
class Property;
class Protocol;
}

namespace ClangAST {

// Wrapper over clang::ASTContext
class ASTGen {
  public:
  static ASTGen& get();

  clang::ObjCProtocolDecl* decl_protocol(const ObjC::Protocol& protocol, clang::DeclContext* DC);
  clang::ObjCMethodDecl* decl_method(const ObjC::Method& method, clang::DeclContext* DC);
  clang::ObjCInterfaceDecl* decl_class(const ObjC::Class& cls, clang::DeclContext* DC);
  clang::ObjCPropertyDecl* decl_property(const ObjC::Property& prop, clang::DeclContext* DC);
  clang::ObjCIvarDecl* decl_ivar(const ObjC::IVar& ivar, clang::ObjCContainerDecl* DC);
  //clang::ParmVarDecl* decl_parameter(const ObjCMethod& protocol, clang::DeclContext* DC);
  clang::ASTContext& ast_ctx();

  static void destroy();
  private:
  ASTGen();
  inline static ASTGen* instance_ = nullptr;
  std::unique_ptr<clang::CompilerInstance> ci_;
};

}
}
#endif
