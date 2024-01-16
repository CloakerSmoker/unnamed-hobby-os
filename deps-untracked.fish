#!/usr/bin/env fish

for file in (string split ' ' (cat build/*.d)); git ls-files --error-unmatch $file 2&> /dev/null; if test $status = 1; echo (string replace "$(pwd)" '.' $file)' is not tracked!'; end; end | uniq | grep -v compiler
