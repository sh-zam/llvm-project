//===--- NoKeyFunctionCheck.cpp - clang-tidy ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "NoKeyFunctionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

namespace {
AST_MATCHER(CXXMethodDecl, isDeclaration) {
  // A function definition, with optional inline but not the declaration.
  return !Node.isThisDeclarationADefinition();
}
}

void NoKeyFunctionCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
    cxxRecordDecl(hasAttr(clang::attr::Visibility),
                  has(cxxMethodDecl(isVirtual(), isDefinition()))).bind("x"), this);
}

void NoKeyFunctionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("x");

  auto HasVirtualMethod = false;
  auto HasKeyFunction = false;
  for (const auto method: MatchedDecl->methods()) {
    if (method->isVirtual()) {
      HasVirtualMethod = true;
      if (!method->hasInlineBody() && !method->isPure()) {
        HasKeyFunction = true;
      }
    }
  }
  if (!HasKeyFunction && HasVirtualMethod) {
      diag(MatchedDecl->getLocation(), "class %0 does not have a Key Function")
          << MatchedDecl;
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
