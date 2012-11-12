//
// ProgramOption.h - MrsWatson
// Created by Nik Reiman on 1/2/12.
// Copyright (c) 2012 Teragon Audio. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#ifndef MrsWatson_ProgramOption_h
#define MrsWatson_ProgramOption_h

#include "base/CharString.h"
#include "base/Types.h"

#define NO_DEFAULT_VALUE -1

typedef enum {
  ARGUMENT_TYPE_NONE,
  ARGUMENT_TYPE_OPTIONAL,
  ARGUMENT_TYPE_REQUIRED
} ProgramOptionArgumentType;

typedef struct {
  int index;
  CharString name;
  CharString help;
  int helpDefaultValue;
  boolByte hasShortForm;

  ProgramOptionArgumentType argumentType;
  CharString argument;
  boolByte enabled;
} ProgramOptionMembers;

typedef ProgramOptionMembers* ProgramOption;

typedef struct {
  ProgramOption* options;
  int numOptions;
} ProgramOptionsMembers;
typedef ProgramOptionsMembers* ProgramOptions;

void addNewProgramOption(const ProgramOptions programOptions, const int optionIndex,
  const char* name, const char* help, boolByte hasShortForm, ProgramOptionArgumentType argumentType,
  int defaultValue);
boolByte parseCommandLine(ProgramOptions programOptions, int argc, char** argv);
ProgramOption findProgramOptionFromString(const ProgramOptions programOptions, const CharString string);

void printProgramOptions(const ProgramOptions programOptions, boolByte withFullHelp, int indentSize);
void printProgramOption(const ProgramOption programOption, boolByte withFullHelp, int indentSize, int initialIndent);

void freeProgramOptions(ProgramOptions programOptions);

#endif