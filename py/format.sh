#!/usr/bin/env bash
find . -name '*.py' -and -not -path './.git/*' -and -not -path './archive/*' -exec echo {} \; -exec yapf -p -i {} \;

