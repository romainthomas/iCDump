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
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Class.hpp"
#include "iCDump/ObjC/IVar.hpp"
#include "iCDump/ObjC/Property.hpp"
#include "iCDump/ObjC/Protocol.hpp"
#include "iCDump/ObjC/TypesEncoding.hpp"
#include "ASTGen.hpp"
#include "log.hpp"

#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Comment.h>
#include <clang/AST/Attr.h>

#include <clang/AST/ASTContext.h>

using namespace clang;


namespace iCDump::ClangAST {

static const ObjC::ObjectTy NSObject = ObjC::ObjectTy("NSObject");

DeclContext::lookup_result search(ASTContext& ctx, const std::string& name, DeclContext* DC) {
  IdentifierInfo& II = ctx.Idents.get(name);
  DeclContext::lookup_result res;
  for (; res.empty() && DC != nullptr; DC = DC->getParent()) {
    res = DC->lookup(&II);
  }
  return res;
}

QualType NSObjectTy(ASTGen& gen, DeclContext* DC) {
  static constexpr const char NSObject_name[] = "NSObject";
  ASTContext& ctx = gen.ast_ctx();
  DeclContext::lookup_result NSObject_lk = search(gen.ast_ctx(), NSObject_name, DC);
  if (NSObject_lk.empty()) {
    IdentifierInfo& II = ctx.Idents.get(NSObject_name);
    auto* decl = ObjCInterfaceDecl::Create(
        ctx, DC, SourceLocation(), &II, nullptr, nullptr);
    return ctx.getObjCInterfaceType(decl);
  } else {
    NamedDecl* ND = NSObject_lk.front();
    if (llvm::isa<ObjCInterfaceDecl>(ND)) {
      return ctx.getObjCInterfaceType(llvm::cast<ObjCInterfaceDecl>(ND));
    }
    ICDUMP_ERR("Inconsistency: NSObject is not ObjCInterfaceDecl");
    return ctx.UnknownAnyTy;
  }
}

std::string pretty_struct(const std::string& sname) {
  // TODO: It should be better to use typedef: using std::string = std::basic_string<...
  if (sname == "basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >") {
    return "std::string";
  }
  return sname;
}


ASTGen& ASTGen::get() {
  if (instance_ == nullptr) {
    instance_ = new ASTGen{};
    std::atexit(destroy);
  }
  return *instance_;
}

ASTGen::ASTGen() {
  ci_ = std::make_unique<CompilerInstance>();
  ci_->createDiagnostics();
  ci_->getTargetOpts().Triple = "x86_64-pc-linux-gnu"; // TODO: Use Mach-O binary information?
  ci_->setTarget(TargetInfo::CreateTargetInfo(
      ci_->getDiagnostics(), ci_->getInvocation().TargetOpts));
  ci_->createFileManager();
  ci_->createSourceManager(ci_->getFileManager());
  ci_->createPreprocessor(TU_Complete);
  ci_->createASTContext();
}

void ASTGen::destroy() {
  delete instance_;
}

ASTContext& ASTGen::ast_ctx() {
  return ci_->getASTContext();
}

ObjCProtocolDecl* ASTGen::decl_protocol(const ObjC::Protocol& protocol, DeclContext* DC) {
  auto& ctx = ast_ctx();
  IdentifierInfo& protocol_id = ctx.Idents.get(protocol.mangled_name());
  auto protocol_decl = ObjCProtocolDecl::Create(
      ctx, DC, &protocol_id,
      SourceLocation(), SourceLocation(), nullptr);
  protocol_decl->startDefinition();

  for (const ObjC::Method& meth : protocol.optional_methods()) {
    ObjCMethodDecl* cmeth = decl_method(meth, protocol_decl);
    cmeth->setDeclImplementation(ObjCMethodDecl::ImplementationControl::Optional);
  }

  for (const ObjC::Method& meth : protocol.required_methods()) {
    ObjCMethodDecl* cmeth = decl_method(meth, protocol_decl);
    cmeth->setDeclImplementation(ObjCMethodDecl::ImplementationControl::Required);
  }

  for (const ObjC::Property& prop : protocol.properties()) {
    ObjCPropertyDecl* cprop = decl_property(prop, protocol_decl);
  }

  DC->addDecl(protocol_decl);
  return protocol_decl;
}


QualType get_qtype(ASTGen& gen, const ObjC::Type& t, DeclContext* DC) {
  auto& ctx = gen.ast_ctx();

  switch (t.type) {
    case ObjC::OBJC_TYPES::CHAR:               return ctx.CharTy;
    case ObjC::OBJC_TYPES::INT:                return ctx.IntTy;
    case ObjC::OBJC_TYPES::SHORT:              return ctx.ShortTy;
    case ObjC::OBJC_TYPES::LONG:               return ctx.LongTy;
    case ObjC::OBJC_TYPES::LONG_LONG:          return ctx.LongLongTy;
    case ObjC::OBJC_TYPES::UNSIGNED_CHAR:      return ctx.UnsignedCharTy;
    case ObjC::OBJC_TYPES::UNSIGNED_INT:       return ctx.UnsignedIntTy;
    case ObjC::OBJC_TYPES::UNSIGNED_SHORT:     return ctx.UnsignedShortTy;
    case ObjC::OBJC_TYPES::UNSIGNED_LONG:      return ctx.UnsignedLongTy;
    case ObjC::OBJC_TYPES::UNSIGNED_LONG_LONG: return ctx.UnsignedLongLongTy;
    case ObjC::OBJC_TYPES::FLOAT:              return ctx.FloatTy;
    case ObjC::OBJC_TYPES::DOUBLE:             return ctx.DoubleTy;
    case ObjC::OBJC_TYPES::BOOL:               return ctx.BoolTy;
    case ObjC::OBJC_TYPES::VOID:               return ctx.VoidTy;
    case ObjC::OBJC_TYPES::SELECTOR:           return ctx.getObjCSelType();
    case ObjC::OBJC_TYPES::BLOCK:              return ctx.getBlockDescriptorType();
    case ObjC::OBJC_TYPES::CLASS:              return ctx.getObjCClassType();

    //case OBJC_TYPES::UNSIGNED_INT:
    //  {
    //    IdentifierInfo& id = ctx.Idents.get("uint32_t");
    //    TypeSourceInfo* ti = ctx.getTrivialTypeSourceInfo(ctx.UnsignedIntTy);
    //    TypedefDecl* u32 = TypedefDecl::Create(
    //        ctx, DC, SourceLocation(), SourceLocation(),
    //        &id, ti);
    //    return ctx.getTypeDeclType(u32);
    //  }

    case ObjC::OBJC_TYPES::CSTRING:
      {
        QualType t = ctx.getPointerType(ctx.CharTy);
        t.addConst();
        return t;
      }

    case ObjC::OBJC_TYPES::POINTER:
      {
        const auto& pointer_ty = static_cast<const ObjC::PointerTy&>(t);
        QualType utype = get_qtype(gen, *pointer_ty.subtype, DC);
        return ctx.getPointerType(utype);
      }


    case ObjC::OBJC_TYPES::STRUCT:
      {
        // Check if the structure is already declared in the DC
        const auto& struct_ty = static_cast<const ObjC::StructTy&>(t);
        //const std::string sname = struct_ty.name;
        const std::string sname = pretty_struct(struct_ty.name);
        IdentifierInfo& II = ctx.Idents.get(sname);
        QualType qt;
        DeclContext::lookup_result lookup = search(ctx, sname, DC);
        if (lookup.empty()) {
          auto* record = CXXRecordDecl::Create(
              ctx, RecordDecl::TagKind::TTK_Struct, DC,
              SourceLocation(), SourceLocation(),
              &II);
          qt = ctx.getRecordType(record);
        }
        else if (lookup.isSingleResult()) {
          NamedDecl* ND = lookup.front();
          if (llvm::isa<RecordDecl>(ND)) {
            qt = ctx.getRecordType(llvm::cast<RecordDecl>(ND));
          } else {
            ICDUMP_ERR("{} is not a structure", struct_ty.name);
            qt = ctx.VoidPtrTy;
          }
        }
        else {
          ICDUMP_ERR("Multiple definitions of {}", struct_ty.name);
        }
        return qt;
      }

    case ObjC::OBJC_TYPES::OBJECT:
      {
        const auto& obj_ty = static_cast<const ObjC::ObjectTy&>(t);
        QualType qt;
        if (!obj_ty.name.empty()) {
          DeclContext::lookup_result lookup = search(ctx, obj_ty.name, DC);
          if (lookup.empty()) {
            IdentifierInfo& II = ctx.Idents.get(obj_ty.name);
            auto* cls_decl = ObjCInterfaceDecl::Create(
                ctx, DC, SourceLocation(), &II, nullptr, nullptr);
            qt = ctx.getObjCInterfaceType(cls_decl);
          }
          else if (lookup.isSingleResult()) {
            NamedDecl* ND = lookup.front();
            if (llvm::isa<ObjCInterfaceDecl>(ND)) {
              qt = ctx.getObjCInterfaceType(llvm::cast<ObjCInterfaceDecl>(ND));
            } else {
              ICDUMP_ERR("{} is not a getObjCInterfaceType", obj_ty.name);
              qt = ctx.VoidTy;
            }
          }
          else {
            ICDUMP_ERR("Multiple definitions of {}", obj_ty.name);
            qt = ctx.VoidTy;
          }
        } else {
          return get_qtype(gen, NSObject, DC);
        }
        return ctx.getPointerType(qt);
      }

    case ObjC::OBJC_TYPES::ARRAY:
      {
        const auto& array_ty = static_cast<const ObjC::ArrayTy&>(t);
        const llvm::APInt dim(sizeof(ObjC::ArrayTy::dim) * 8, array_ty.dim);
        return ctx.getConstantArrayType(
            get_qtype(gen, *array_ty.subtype, DC), dim,
            nullptr, ArrayType::ArraySizeModifier::Normal, 0);
      }

    case ObjC::OBJC_TYPES::UNION:
      {
        // Check if the union is already declared in the DC
        const auto& union_ty = static_cast<const ObjC::UnionTy&>(t);
        //const std::string sname = struct_ty.name;
        const std::string sname = pretty_struct(union_ty.name);
        IdentifierInfo& II = ctx.Idents.get(sname);
        QualType qt;
        DeclContext::lookup_result lookup = search(ctx, sname, DC);
        if (lookup.empty()) {
          auto* record = CXXRecordDecl::Create(
              ctx, RecordDecl::TagKind::TTK_Union, DC,
              SourceLocation(), SourceLocation(),
              &II);
          qt = ctx.getRecordType(record);
        }
        else if (lookup.isSingleResult()) {
          NamedDecl* ND = lookup.front();
          if (llvm::isa<RecordDecl>(ND)) {
            qt = ctx.getRecordType(llvm::cast<RecordDecl>(ND));
          } else {
            ICDUMP_ERR("{} is not a union", union_ty.name);
            qt = ctx.VoidPtrTy;
          }
        }
        else {
          ICDUMP_ERR("Multiple definitions of {}", union_ty.name);
        }
        return qt;

      }

    default:
      {
        ICDUMP_ERR("Type: {} not supported", to_string(t.type));
        return ctx.VoidTy;
      }
        //t.addCons;
  }
}

ObjCMethodDecl* ASTGen::decl_method(const ObjC::Method& meth, DeclContext* DC) {
  auto& ctx = ast_ctx();
  auto selector = GetUnarySelector(meth.name(), ctx);

  ObjC::Method::prototype_t prototype = meth.prototype();
  QualType rtype = get_qtype(*this, *prototype.rtype, DC);

  auto clang_method = ObjCMethodDecl::Create(
      ctx, SourceLocation(), SourceLocation(),
      selector, rtype, nullptr,
      DC, meth.is_instance());

  llvm::SmallVector<ParmVarDecl*, 8> params;
  //ICDUMP_ERR("{}: {}", meth.name(), meth.mangled_type(), prototype.);
  for (size_t i = 0; i < prototype.params.size(); ++i) {
    auto& p = prototype.params[i];
    QualType qt;
    std::string arg_name;
    if (i == 0 && p->type == ObjC::OBJC_TYPES::OBJECT) {
      arg_name = "self";
      if (llvm::isa<ObjCInterfaceDecl>(DC)) {
        qt = ctx.getObjCInterfaceType(llvm::cast<ObjCInterfaceDecl>(DC));
        qt = ctx.getPointerType(qt);
      } else {
        qt = get_qtype(*this, *p, clang_method);
      }
    }
    else if (p->type == ObjC::OBJC_TYPES::SELECTOR) {
      arg_name = "id";
      qt = get_qtype(*this, *p, clang_method);
    }
    else {
      arg_name = "arg" + std::to_string(i);
      qt = get_qtype(*this, *p, clang_method);
    }

    auto* pdecl = ParmVarDecl::Create(
        ctx, clang_method,
        SourceLocation(), SourceLocation(),
        &ctx.Idents.get(arg_name), qt, nullptr,
        StorageClass::SC_None, nullptr);
    params.push_back(pdecl);
  }
  clang_method->setMethodParams(ctx, params);

  DC->addDecl(clang_method);
  return clang_method;
}


ObjCInterfaceDecl* ASTGen::decl_class(const ObjC::Class& cls, DeclContext* DC) {
  auto& ctx = ast_ctx();

  //auto* type_param_list = clang::ObjCTypeParamList::create(
  //    ctx, clang::SourceLocation(), {}, clang::SourceLocation());

  IdentifierInfo& id = ctx.Idents.get(cls.demangled_name());

  auto* cls_decl = ObjCInterfaceDecl::Create(
      ctx, DC, SourceLocation(), &id, nullptr, nullptr);
  cls_decl->startDefinition();
  llvm::SmallVector<ObjCProtocolDecl*, 8> protocols;
  llvm::SmallVector<SourceLocation, 8> source_locations;
  for (const ObjC::Protocol& proto : cls.protocols()) {
    IdentifierInfo& protocol_id = ctx.Idents.get(proto.mangled_name());
    auto protocol_decl = ObjCProtocolDecl::Create(
        ctx, cls_decl, &protocol_id,
        SourceLocation(), SourceLocation(), nullptr);
    protocols.push_back(protocol_decl);
    source_locations.push_back(SourceLocation());
  }
  if (!protocols.empty()) {
    //QualType ns = NSObjectTy(*this, DC);
    //TypeSourceInfo* TSI = ctx.CreateTypeSourceInfo(ns);
    //cls_decl->setSuperClass(TSI);
    cls_decl->setProtocolList(protocols.data(), protocols.size(), source_locations.data(), ctx);
  }

  for (const ObjC::Method& meth : cls.methods()) {
    ObjCMethodDecl* cmeth = decl_method(meth, cls_decl);
  }

  for (const ObjC::IVar& ivar : cls.ivars()) {
    ObjCIvarDecl* cprop = decl_ivar(ivar, cls_decl);
  }

  for (const ObjC::Property& prop : cls.properties()) {
    ObjCPropertyDecl* cprop = decl_property(prop, cls_decl);
  }
  DC->addDecl(cls_decl);
  return cls_decl;
}

ObjCPropertyDecl* ASTGen::decl_property(const ObjC::Property& prop, DeclContext* DC) {
  auto& ctx = ast_ctx();

  IdentifierInfo& id = ctx.Idents.get(prop.name());
  auto* prop_decl = ObjCPropertyDecl::Create(
      ctx, DC, SourceLocation(), &id, SourceLocation(), SourceLocation(),
      /* TODO */ctx.VoidTy, nullptr);
  DC->addDecl(prop_decl);
  return prop_decl;
}


ObjCIvarDecl* ASTGen::decl_ivar(const ObjC::IVar& ivar, ObjCContainerDecl* DC) {
  auto& ctx = ast_ctx();

  IdentifierInfo& id = ctx.Idents.get(ivar.name());

  QualType type;
  if (!ivar.mangled_type().empty()) {
    std::unique_ptr<ObjC::Type> ivar_type = ivar.type();
    if (ivar_type != nullptr) {
      type = get_qtype(*this, *ivar_type, DC);
    } else {
      ICDUMP_ERR("Can't resolve type for ivar: {} ({})", ivar.name(), ivar.mangled_type());
      type = ctx.UnknownAnyTy;
    }
  } else {
    // Empty type. Assume it is NSObject
    type = ctx.getPointerType(NSObjectTy(*this, DC));
  }

  auto* ivar_decl = ObjCIvarDecl::Create(
      ctx, DC, SourceLocation(), SourceLocation(), &id,
      type, nullptr, /* TODO */ ObjCIvarDecl::AccessControl::Public
      );
  DC->addDecl(ivar_decl);
  return ivar_decl;
}


}
