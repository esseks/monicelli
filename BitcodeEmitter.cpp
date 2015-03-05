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
#include "Nodes.hpp"

using namespace monicelli;

struct BitcodeEmitter::Private {
};

BitcodeEmitter::BitcodeEmitter() {
    d = new Private;
}

BitcodeEmitter::~BitcodeEmitter() {
    delete d;
}

void BitcodeEmitter::emit(Return const& node) {
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

void BitcodeEmitter::emit(BranchCase const& node) {
}

void BitcodeEmitter::emit(Branch const& node) {
}

void BitcodeEmitter::emit(Main const& node) {
}

void BitcodeEmitter::emit(Function const& node) {
}

void BitcodeEmitter::emit(Module const& node) {
}

void BitcodeEmitter::emit(Program const& node) {
}

void BitcodeEmitter::emit(Id const& node) {
}

void BitcodeEmitter::emit(Integer const& node) {
}

void BitcodeEmitter::emit(Float const& node) {
}

void BitcodeEmitter::emit(BinaryExpression const& node) {
}

void BitcodeEmitter::emit(BinarySemiExpression const& node) {
}

