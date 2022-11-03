#!/usr/bin/env bash

set -eu

declare -r SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
declare -r FW_DIR=$SCRIPT_DIR/firmware
declare -r CLANG_FORMAT_VER=16
declare -r CLANG=clang-format-"$CLANG_FORMAT_VER"

if ! command -v "$CLANG" &> /dev/null; then
    echo "ERROR: Unable to find $CLANG in PATH" >&2
    echo "You can download any version of clang-format here https://apt.llvm.org/" >&2
    echo "Specifically do:" >&2
    echo "  wget https://apt.llvm.org/llvm.sh" >&2
    echo "  chmod +x llvm.sh" >&2
    echo "  sudo ./llvm.sh $CLANG_FORMAT_VER" >&2
    echo "  sudo apt-get install $CLANG" >&2
    exit 1
fi

readarray -d '' files_to_format < <(find "$FW_DIR" -maxdepth 1 -type f -name '*.[ch]' -print0)
readarray -d '' -O "${#files_to_format[@]}" files_to_format < <(find "$FW_DIR/modes" -type f -name '*.[ch]' -print0)

have_unformatted_files=0
if ! "$CLANG" --dry-run --Werror "${files_to_format[@]}" &> /dev/null; then
    have_unformatted_files=1
fi

"$CLANG" -i "${files_to_format[@]}"
exit "$have_unformatted_files"
