#!/usr/bin/env fish

# Script to add GPL license preambles to C/C++ source files

set current_year (date +%Y)
set license_header "// Copyright (c)  $current_year Lukas Neverauskis <lukas.neverauskis@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only
"

set pragma_once_header "#pragma once
 "

for file in (find ln/ port/ cmake/ -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" \))
    set temp_file (mktemp)
    if not grep -qE "[cC]opyright\s+\([cC]\)" "$file"
    echo "+preable $file"
        echo -e "$license_header" > "$temp_file"
    end
    if string match -qr '\.(h|hpp)$' -- "$file"
        if not grep -q "#pragma once" "$file"
            echo -e "$pragma_once_header" >> "$temp_file"
        end
    end
    cat "$file" >> "$temp_file"
    mv "$temp_file" "$file"
end
