#ifndef BITCODE_HPP
#define BITCODE_HPP

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

#include "Emitter.hpp"

namespace llvm {
    class Module;
}

namespace monicelli {

class BitcodeEmitter: public Emitter {
public:
    BitcodeEmitter(llvm::Module *module);
    BitcodeEmitter(BitcodeEmitter &) = delete;
    virtual ~BitcodeEmitter();

    virtual void emit(Return const&) override;
    virtual void emit(Loop const&) override;
    virtual void emit(VarDeclaration const&) override;
    virtual void emit(Assignment const&) override;
    virtual void emit(Print const&) override;
    virtual void emit(Input const&) override;
    virtual void emit(Abort const&) override;
    virtual void emit(Assert const&) override;
    virtual void emit(FunctionCall const&) override;
    virtual void emit(Branch const&) override;
    virtual void emit(Function const&) override;
    virtual void emit(Module const&) override;
    virtual void emit(Program const&) override;

    virtual void emit(Id const&) override;
    virtual void emit(Integer const&) override;
    virtual void emit(Float const&) override;
    virtual void emit(BinaryExpression const&) override;

private:
    struct Private;

    llvm::Module *module;
    Private *d;
};

}

#endif

