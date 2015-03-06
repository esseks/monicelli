/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BitcodeEmitter.hpp"
#include "Scope.hpp"
#include "Nodes.hpp"

#include <llvm/Analysis/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>


using namespace monicelli;
using llvm::getGlobalContext;

struct BitcodeEmitter::Private {
    llvm::Value *expval = nullptr;
    llvm::Module *module = nullptr;

    llvm::IRBuilder<> builder = llvm::IRBuilder<>(getGlobalContext());
    Scope<Id const*, llvm::Value*> scope;
};

BitcodeEmitter::BitcodeEmitter() {
    module = std::unique_ptr<llvm::Module>(new llvm::Module("monicelli", getGlobalContext()));
    d = new Private;
}

BitcodeEmitter::~BitcodeEmitter() {
    delete d;
}

void BitcodeEmitter::emit(Return const& node) {
    if (node.getExpression()) {
        node.getExpression()->emit(this);
        d->builder.CreateRet(d->expval);
    } else {
        d->builder.CreateRetVoid();
    }
}

void BitcodeEmitter::emit(Loop const& node) {
}

void BitcodeEmitter::emit(VarDeclaration const& node) {
}

void BitcodeEmitter::emit(Assignment const& node) {
}

void BitcodeEmitter::emit(Print const& node) {
}

void BitcodeEmitter::emit(Input const& node) {
}

void BitcodeEmitter::emit(Abort const& node) {
}

void BitcodeEmitter::emit(Assert const& node) {
}

void BitcodeEmitter::emit(FunctionCall const& node) {
}

void BitcodeEmitter::emit(Branch const& node) {
}

void BitcodeEmitter::emit(Main const& node) {
    d->scope.enter();
    for (Statement const* statement: node.getBody()) {
        statement->emit(this);
    }
    d->scope.leave();
}

void BitcodeEmitter::emit(Function const& node) {
}

void BitcodeEmitter::emit(Module const& node) {}

void BitcodeEmitter::emit(Program const& program) {
    if (program.getMain()) {
        program.getMain()->emit(this);
    }

    for (Function const* function: program.getFunctions()) {
        function->emit(this);
    }

    // TODO modules
}

void BitcodeEmitter::emit(Id const& node) {
    auto value = d->scope.lookup(&node);

    if (!value) {
        // TODO errore
    }

    d->expval = *value;
}

void BitcodeEmitter::emit(Integer const& node) {
    d->expval = llvm::ConstantInt::get(
        getGlobalContext(), llvm::APInt(64, node.getValue(), true)
    );
}

void BitcodeEmitter::emit(Float const& node) {
    d->expval = llvm::ConstantFP::get(
        getGlobalContext(), llvm::APFloat(node.getValue())
    );
}

static inline
bool isFP(llvm::Value const* var) {
    llvm::Type *type = var->getType();
    return type->isFloatTy() || type->isDoubleTy();
}

#define HANDLE(intop, fpop) \
    if (fp) { \
        d->expval = d->builder.Create##fpop(left, right); \
    } else { \
        d->expval = d->builder.Create##intop(left, right); \
    }

#define HANDLE_INT_ONLY(op) \
    if (fp) { \
        d->expval = nullptr; \
    } else { \
        d->expval = d->builder.Create##op(left, right); \
    }


// TODO Handle automatic casts
void BitcodeEmitter::emit(BinaryExpression const& expression) {
    expression.getLeft().emit(this);
    llvm::Value *left = d->expval;

    expression.getRight().emit(this);
    llvm::Value *right = d->expval;

    bool fp = isFP(left) || isFP(right);

    // TODO left->getType() != right->getType()

    switch (expression.getOperator()) {
        case Operator::PLUS:
            HANDLE(Add, FAdd)
            break;
        case Operator::MINUS:
            HANDLE(Sub, FSub)
            break;
        case Operator::TIMES:
            HANDLE(Mul, FMul)
            break;
        case Operator::DIV:
            HANDLE(SDiv, FDiv)
            break;
        case Operator::SHL:
            HANDLE_INT_ONLY(Shl);
            break;
        case Operator::SHR:
            HANDLE_INT_ONLY(LShr);
            break;
        case Operator::LT:
            HANDLE(ICmpULT, FCmpULT)
            break;
        case Operator::GT:
            HANDLE(ICmpUGT, FCmpUGT)
            break;
        case Operator::GTE:
            HANDLE(ICmpUGE, FCmpUGE)
            break;
        case Operator::LTE:
            HANDLE(ICmpULE, FCmpULE)
            break;
        case Operator::EQ:
            HANDLE(ICmpEQ, FCmpOEQ)
            break;
    }
}

#undef HANDLE
#undef HANDLE_INT_ONLY

