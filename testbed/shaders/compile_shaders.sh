#!/bin/bash

# Directory containing the shaders
SHADER_DIR="${1:-.}"

# Function to compile shaders recursively
compile_shaders() {
    local dir="$1"
    for file in "$dir"/*; do
        if [[ -d "$file" ]]; then
            # If it's a directory, recurse into it
            compile_shaders "$file"
        elif [[ -f "$file" ]]; then
            # If it's a file, check for shader extensions
            if [[ "$file" =~ \.vert$|\.frag$|\.comp$|\.geom$|\.tesc$|\.tese$ ]]; then
                output_file="${file}.spv"
                echo "Compiling $file -> $output_file"
                glslc "$file" -o "$output_file" -std=450 -fpreserve-bindings
                if [[ $? -ne 0 ]]; then
                    echo "Failed to compile $file"
                fi
            fi
        fi
    done
}

# Start compilation from the shader directory
compile_shaders "$SHADER_DIR"
