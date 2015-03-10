#ifndef EMITTER_HPP
#define EMITTER_HPP

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

namespace monicelli {

class Id;
class Number;
class Integer;
class Float;
class Return;
class Loop;
class VarDeclaration;
class Assignment;
class Print;
class Input;
class Abort;
class Assert;
class FunctionCall;
class Branch;
class Main;
class FunctionPrototype;
class Function;
class Module;
class Program;
class BinaryExpression;


class Emitter {
public:
    virtual bool emit(Return const&) = 0;
    virtual bool emit(Loop const&) = 0;
    virtual bool emit(VarDeclaration const&) = 0;
    virtual bool emit(Assignment const&) = 0;
    virtual bool emit(Print const&) = 0;
    virtual bool emit(Input const&) = 0;
    virtual bool emit(Abort const&) = 0;
    virtual bool emit(Assert const&) = 0;
    virtual bool emit(FunctionPrototype const&) = 0;
    virtual bool emit(Branch const&) = 0;
    virtual bool emit(Function const&) = 0;
    virtual bool emit(Module const&) = 0;
    virtual bool emit(Program const&) = 0;

    virtual bool emit(Id const&) = 0;
    virtual bool emit(Integer const&) = 0;
    virtual bool emit(Float const&) = 0;
    virtual bool emit(FunctionCall const&) = 0;
    virtual bool emit(BinaryExpression const&) = 0;
};

}

#endif

