//
// ErrorReport.c - MrsWatson
// Created by Nik Reiman on 9/22/12.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "app/BuildInfo.h"
#include "base/FileUtilities.h"
#include "base/PlatformUtilities.h"
#include "base/StringUtilities.h"
#include "logging/ErrorReporter.h"
#include "logging/EventLogger.h"
#include "MrsWatson.h"

#if UNIX
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#elif WINDOWS
#include <Shlobj.h>
#endif

// If support for libarchive is built, then the error report will be compressed
// on completion. However, this library doesn't build so easily on Windows, so
// this feature is disabled for the time being.
#define HAVE_LIBARCHIVE 0

#if HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#endif

ErrorReporter newErrorReporter(void) {

  ErrorReporter errorReporter = (ErrorReporter)malloc(sizeof(ErrorReporterMembers));

  errorReporter->started = false;
  errorReporter->completed = false;
  errorReporter->reportName = newCharString();

  errorReporter->desktopPath = newCharString();
  errorReporter->reportDirPath = newCharString();

  return errorReporter;
}

void initializeErrorReporter(ErrorReporter errorReporter) {
  time_t now;
  int length;
  int i;

  time(&now);
  errorReporter->started = true;

  snprintf(errorReporter->reportName->data, errorReporter->reportName->length,
    "MrsWatson Report %s", ctime(&now));
  // Trim the final newline character from this string if it exists
  length = strlen(errorReporter->reportName->data);
  if(errorReporter->reportName->data[length - 1] == '\n') {
    errorReporter->reportName->data[length - 1] = '\0';
    length--;
  }
  for(i = 0; i < length; i++) {
    if(!(isLetter(errorReporter->reportName->data[i]) ||
         isNumber(errorReporter->reportName->data[i]))) {
      errorReporter->reportName->data[i] = '-';
    }
  }

 #if UNIX
  snprintf(errorReporter->desktopPath->data, errorReporter->desktopPath->length,
    "%s/Desktop", getenv("HOME"));
#elif WINDOWS
  SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, errorReporter->desktopPath->data);
#endif
  snprintf(errorReporter->reportDirPath->data, errorReporter->reportDirPath->length,
    "%s%c%s", errorReporter->desktopPath->data, PATH_DELIMITER, errorReporter->reportName->data);
  makeDirectory(errorReporter->reportDirPath);
}

void createCommandLineLauncher(ErrorReporter errorReporter, int argc, char* argv[]) {
  CharString outScriptName = newCharString();
  FILE *scriptFilePointer;
  int i;

  copyToCharString(outScriptName, "run.sh");
  remapPathToErrorReportDir(errorReporter, outScriptName);
  scriptFilePointer = fopen(outScriptName->data, "w");
  fprintf(scriptFilePointer, "#!/bin/sh\n");
  fprintf(scriptFilePointer, "mrswatson");
  for(i = 1; i < argc; i++) {
    // Don't run with the error report option again
    if(strcmp(argv[i], "--error-report")) {
      fprintf(scriptFilePointer, " %s", argv[i]);
    }
  }
  fprintf(scriptFilePointer, "\n");
  fclose(scriptFilePointer);
}

void remapPathToErrorReportDir(ErrorReporter errorReporter, CharString path) {
  CharString basename = newCharString();
  CharString outString = newCharStringWithCapacity(path->length);

  copyToCharString(basename, getFileBasename(path->data));
  buildAbsolutePath(errorReporter->reportDirPath, basename, NULL, outString);
  copyCharStrings(path, outString);

  freeCharString(basename);
  freeCharString(outString);
}

boolByte copyFileToErrorReportDir(ErrorReporter errorReporter, CharString path) {
  boolByte result = false;
  CharString destination = newCharString();

  copyCharStrings(destination, path);
  remapPathToErrorReportDir(errorReporter, destination);
  copyFileToDirectory(path, errorReporter->reportDirPath);

  freeCharString(destination);
  return result;
}

// TODO: Refactor this into FileUtilities
static boolByte _copyDirectoryToErrorReportDir(ErrorReporter errorReporter, CharString path) {
  // TODO: This is the lazy way of doing this...
#if UNIX
  CharString copyCommand = newCharString();
  snprintf(copyCommand->data, copyCommand->length, "/bin/cp -r \"%s\" \"%s\"",
    path->data, errorReporter->reportDirPath->data);
  // TODO: Check error codes
  system(copyCommand->data);
#else
  logUnsupportedFeature("Copy directory recursively");
#endif
  return false;
}

boolByte copyPluginsToErrorReportDir(ErrorReporter errorReporter, PluginChain pluginChain) {
  CharString promptText = newCharStringWithCString("Would you like to copy the \
plugin to the report? All data sent to the official support address is kept \
strictly confidential. If the plugin in question has copy protection (or is \
cracked), or depends on external resources, this probably won't work. But if \
the plugin can be copied, it greatly helps in fixing bugs.\n\
Copy the plugin? (y/n) ");
  CharString wrappedPromptText = newCharStringWithCapacity(promptText->length);
  CharString pluginAbsolutePath = newCharString();
  Plugin currentPlugin;
  boolByte result = true;
  int i;
  char response;

  wrapString(promptText->data, wrappedPromptText->data, 0);
  printf("%s", wrappedPromptText->data);
  response = getchar();
  if(response == 'y' || response == 'Y') {
    for(i = 0; i < pluginChain->numPlugins; i++) {
      currentPlugin = pluginChain->plugins[i];
      currentPlugin->getAbsolutePath(currentPlugin, pluginAbsolutePath);
      if(getPlatformType() == PLATFORM_MACOSX) {
        result |= _copyDirectoryToErrorReportDir(errorReporter, pluginAbsolutePath);
      }
      else {
        result |= copyFileToErrorReportDir(errorReporter, pluginAbsolutePath);      
      }
    }
    return result;
  }
  return false;
}

#if HAVE_LIBARCHIVE
static void _remapFileToErrorReportRelativePath(void* item, void* userData) {
  char* itemName = (char*)item;
  CharString tempPath = newCharString();
  ErrorReporter errorReporter = (ErrorReporter)userData;
  copyToCharString(tempPath, itemName);
  snprintf(tempPath->data, tempPath->length, "%s/%s", errorReporter->reportName->data, itemName);
  strncpy(itemName, tempPath->data, tempPath->length);
}
#endif

#if HAVE_LIBARCHIVE
static void _addFileToArchive(void* item, void* userData) {
  char* itemPath = (char*)item;
  struct archive* outArchive = (struct archive*)userData;
  struct archive_entry* entry = archive_entry_new();
  struct stat fileStat;
  FILE* filePointer;
  size_t bytesRead;
  byte* fileBuffer = (byte*)malloc(8192);

  stat(itemPath, &fileStat);
  archive_entry_set_pathname(entry, itemPath);
  archive_entry_set_size(entry, fileStat.st_size);
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  archive_write_header(outArchive, entry);
  filePointer = fopen(itemPath, "rb");
  do {
    bytesRead = fread(fileBuffer, 1, 8192, filePointer);
    if(bytesRead > 0) {
      archive_write_data(outArchive, fileBuffer, bytesRead);
    }
  } while(bytesRead > 0);
  fclose(filePointer);
  archive_entry_free(entry);
}
#endif

void printErrorReportInfo(void) {
  CharString infoText = newCharStringWithCString("MrsWatson is now running \
in error report mode, which will generate a zipfile on your desktop with \
any input/output sources and error logs. This also enables some extra \
arguments and will disable console logging.\n");
  CharString wrappedInfoText = newCharStringWithCapacity(infoText->length);
  printf("=== Starting error report ===\n");
  wrapString(infoText->data, wrappedInfoText->data, 0);
  // The second newline here is intentional
  printf("%s\n", wrappedInfoText->data);
}

void completeErrorReport(ErrorReporter errorReporter) {
#if HAVE_LIBARCHIVE
  struct archive* outArchive;
  CharString outputFilename = newCharString();
  LinkedList reportContents = newLinkedList();
#endif

  // Always do this, just in case
  flushErrorLog();

#if HAVE_LIBARCHIVE
  // In case any part of the error report causes a segfault, this function will
  // be called recursively. A mutex would really be a better solution here, but
  // this will also work just fine.
  if(!errorReporter->completed) {
    errorReporter->completed = true;
    buildAbsolutePath(errorReporter->desktopPath, errorReporter->reportName, "tar.gz", outputFilename);
    listDirectory(errorReporter->reportDirPath->data, reportContents);
    if(errorReporter != NULL) {
      outArchive = archive_write_new();
      archive_write_set_compression_gzip(outArchive);
      archive_write_set_format_pax_restricted(outArchive);
      archive_write_open_filename(outArchive, outputFilename->data);

      foreachItemInList(reportContents, _remapFileToErrorReportRelativePath, errorReporter);
      chdir(errorReporter->desktopPath->data);
      foreachItemInList(reportContents, _addFileToArchive, outArchive);

      archive_write_close(outArchive);
      archive_write_free(outArchive);
    }
    // Remove original error report
    removeDirectory(errorReporter->reportDirPath);
  }
#endif
}

void printErrorReportComplete(void) {
  printf("\n=== Error report complete ===\n");
#if HAVE_LIBARCHIVE
  printf("Please compress and email the error report on your desktop to: %s\n", SUPPORT_EMAIL);
#else
  printf("Please email the error report on your desktop to: %s\n", SUPPORT_EMAIL);
#endif
  printf("Thanks!\n");
}

void freeErrorReporter(ErrorReporter errorReporter) {
  freeCharString(errorReporter->reportName);
  freeCharString(errorReporter->reportDirPath);
  freeCharString(errorReporter->desktopPath);
  free(errorReporter);
}