#!/bin/bash

# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved



__VERSION="1.0.0"

# Initialize reused variables
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

declare -A testlist

# CLI variables
declare -a exclude_list
declare -a include_list
printH=false
printTestList=false
printV=false

# log level variables
declare -i __VERBOSITY
declare -A LOG_LEVELS
LOG_LEVELS=( [0]="error" [1]="info" [2]="debug" )
LOG_LEVEL_COLORS=( [0]="${RED}" [1]="${BLUE}" [2]="${YELLOW}" )

# Statistics
npass=0
nfail=0

# --------------------------------------------------------- #
# Process env vars
if [ -z "$TESTS_PATH" ]; then
  TESTS_PATH=/usr/lib/embers-tests/tests
fi

# Verify VERBOSITY is valid. Defaults to info.
if [ -n "$VERBOSITY" ]; then
  __VERBOSITY=$(( VERBOSITY ))
  if [ ${__VERBOSITY} -gt 2 ]; then
    .log 0 "VERBOSITY must be one of the following : [0,1,2]"
  fi
else
  __VERBOSITY=1
fi

# Setup Logging
function .log () {
  declare -i LEVEL=${1}
  shift
  if [ $__VERBOSITY -ge "$LEVEL" ]; then
    printf '%b[%s]%b %s\n' "${LOG_LEVEL_COLORS[$LEVEL]}" "${LOG_LEVELS[$LEVEL]}" "${NC}" "$@"
  fi
}

declare -a test_paths=(
  "${TESTS_PATH}"
)

FilterInclude() {
  .log 2 "Filtering for only included tests"
  local -a sorted unsorted include_list=("$@")

  unsorted+=("${!testlist[@]}")
  unsorted+=("${include_list[@]}")

  IFS=$'\n'
  mapfile -t sorted <<< "$(sed '/^$/d' <<<"${unsorted[*]}" | sort | uniq -u)"
  unset IFS

  for k in "${sorted[@]}"
  do
    .log 2 "skipping $k"
    unset 'testlist["$k"]'
  done
  .log 2 "skipping ${#sorted[@]} tests"
}

FilterExclude() {
  .log 2 "Filtering out excluded tests"
  local -a unsorted exclude_list=("$@")
  unsorted+=("${!testlist[@]}")
  unsorted+=("${exclude_list[@]}")

  IFS=$'\n'
  mapfile -t sorted <<< "$(sed '/^$/d' <<<"${unsorted[*]}" | sort | uniq -d)"
  unset IFS

  for k in "${sorted[@]}"
  do
    .log 2 "skipping $k"
    unset 'testlist["$k"]'
  done
  .log 2 "skipping ${#sorted[@]} tests"
}

PopulateTestlist() {
  .log 2 "Populating testlist"
  for p in "${test_paths[@]}"
  do
    for f in "${p}"/*
    do
      name="$(basename "$f")"
      testlist["$name"]=$f
    done
  done
  .log 2 "Found ${#testlist[@]} tests"
}

Run()
{
  .log 2 "Running ${#testlist[@]} tests"
  for name in "${!testlist[@]}"
  do
    f="${testlist[$name]}"
    if [ -x "$f" ]
    then
      .log 1 "$name"
      printf '%-.48s' "running ................................................"
      if ! $f >/dev/null 2>&1
      then
        nfail=$((nfail+1))
        printf "%bFAIL%b\n" "${RED}" "${NC}"
      else
        npass=$((npass+1))
        printf "%bPASS%b\n" "${GREEN}" "${NC}"
      fi
    fi
  done
}

PrintResults()
{
  # Print pass/fail count
  testcount=$((npass+nfail))
  .log 1 "PASS: ${npass}/${testcount}"
  .log 1 "FAIL: ${nfail}/${testcount}"

  # Pretty print overall PASS/FAIL
  if [ $nfail -ne 0 ]
  then
    text="
+================+
|░█▀▀░█▀█░▀█▀░█░░|
|░█▀▀░█▀█░░█░░█░░|
|░▀░░░▀░▀░▀▀▀░▀▀▀|
+================+
  "
    printf "%b%s%b" "${RED}" "${text}" "${NC}"
  else
    text="
+================+
|░█▀█░█▀█░█▀▀░█▀▀|
|░█▀▀░█▀█░▀▀█░▀▀█|
|░▀░░░▀░▀░▀▀▀░▀▀▀|
+================+
"
    printf "%b%s%b" "${GREEN}" "${text}" "${NC}"
  fi
}

Help()
{
  # Display Help
   echo "run_emberstests.sh is a simple wrapper for embers unit tests."
   echo
   echo "Syntax: run_emberstests.sh [-h|E|I|V]"
   echo "options:"
   echo "h                   Print this Help."
   echo "l                   Print list of tests that will run."
   echo "E <test_name>       Add unit test to exlude list. This option is repeatable"
   echo "I <test_name>       Add unit test to include list. This options is repeatable."
   echo "                    Only run unit tests found in the include list if it exists"
   echo "V                   Print software version and exit."
   echo
   echo "Environment Variables:"
   echo "VERBOSITY    Set the log level. Must be one of [0,1,2] where 2 is the most verbose. [Default: 1]"
}

Version() {
  case $1 in
    ''|*[!0-9]*) .log 0 "non-digit character passed to Version()" ;;
  esac
  .log "$1" "Version: $__VERSION"
}

ListTests() {
  .log 2 "Listing all tests found after filtering"
  IFS=$'\n'
  echo "${!testlist[*]}"
  unset IFS
}

# MAIN
Main()
{
  while getopts ":hlVE:I:" opt; do
    case $opt in
        h) printH=true;;
        l) printTestList=true;;
        E) exclude_list+=("$OPTARG");;
        I) include_list+=("$OPTARG");;
        V) printV=true;;
        *) .log 0 "Invalid option found, see -h for help." && exit 1;;
    esac
  done
  shift $((OPTIND -1))

  # Enforce mutual exclusion
  if [ ${#exclude_list[@]} -ne 0 ] && [ ${#include_list[@]} -ne 0 ]
  then
    echo "Exclude and Include lists are mutually exclusive. Please use either -E or -I but not both."
    exit 1
  fi

  if [ "${printH}" == true ]
  then
    Help
    exit 0
  fi

  if [ "${printV}" == true ]
  then
    Version 1
    exit 0
  fi
  Version 2
  PopulateTestlist

  if [ ${#exclude_list[@]} -ne 0 ]
  then
    FilterExclude "${exclude_list[@]}"
  elif [ ${#include_list[@]} -ne 0 ]
  then
    FilterInclude "${include_list[@]}"
  fi

  if [ "${printTestList}" == true ]
  then
    ListTests
    exit 0
  fi

  Run
  PrintResults
  if [ $nfail -ne 0 ]
  then
    exit 1
  else
    exit 0
  fi
}

Main "$@"
